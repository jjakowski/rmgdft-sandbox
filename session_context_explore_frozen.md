# FROZEN RECORD — pre-refactor explore branch, pre May 2026 sync
# Do not edit.

# Session Context — RT-TDDFT Theory, Documentation, and Tutorial
**Date:** 2026-04-01 (updated 2026-04-02)
**Branch:** `explore` (`rmgdft-sandbox-explore/`)
**Session goal:** Deep code reading → inline annotations → LaTeX documentation → TDDFT theory tutorial

---

## 1. RT-TDDFT Code Analysis (RMG)

### What Was Read and Understood

The following files were read in full and their physics reconstructed:

| File | What it does |
|------|-------------|
| `TDDFT/RMG_TDDFT/RmgTddft.cpp` | Main time propagation loop — predictor-corrector Magnus |
| `TDDFT/RMG_TDDFT/VecPmatrix.cpp` | Momentum matrix elements P^α for velocity gauge |
| `TDDFT/RMG_TDDFT/CurrentNlpp.cpp` | Nonlocal PP correction to current operator |
| `TDDFT/ELDYN/Magnus.cpp` | Magnus operator Ω = ½(H(t)+H_pred)·dt |
| `TDDFT/ELDYN/commutp2.cpp` | BCH propagator: e^{-iΩ}Pe^{iΩ} as nested commutators |
| `TDDFT/ELDYN/eldyn_ort.cpp` | Propagator selector: BCH (Ieldyn=1) or diag (Ieldyn=2) |
| `TDDFT/RMG_TDDFT/HmatrixUpdate.cpp` | H matrix rebuild: A_ij = vel·GEMM(φ†, V·φ) |
| `TDDFT/RMG_TDDFT/GetNewRho_rmgtddft.cpp` | Density from P(t): ΔP→xpsi→ρ_coarse→Prolong |
| `TDDFT/RMG_TDDFT/RmgTddft_jj.cpp` | Jacek's scratch copy — used as annotation style reference |

### Key Physics Reconstructed from Code

**Formalism:** von Neumann equation `i dP/dt = [H(t), P(t)]` in fixed ground-state KS orbital basis {φ_j}. No overlap matrix S because basis is orthonormal. This is the MO-basis analogue of the AO Liouville equation in Gaussian-basis codes.

**Algorithm:** Predictor-corrector Magnus:
1. Predictor: `H_pred(t+dt) = 2H(t) - H(t-dt)` (linear extrapolation)
2. SCF loop until `‖H_new - H_old‖_∞ < 1e-7`:
   - Ω = ½(H(t) + H_pred)·dt  [magnus()]
   - P(t+dt) = e^{-iΩ} P(t) e^{iΩ}  [commutp/diagev via eldyn_ort]
   - ρ(t+dt) from ΔP  [GetNewRho_rmgtddft]
   - V_H, V_xc update  [VhDriver, compute_vxc]
   - H(t+dt) += ⟨ΔV⟩  [HmatrixUpdate]
3. Advance: P(t)←P(t+dt), H(t-dt)←H(t), H(t)←H(t+dt)

**Perturbation modes:**
- `EFIELD` (finite systems, length gauge): `V_ext = -E·r`, observable = dipole d(t)
- `VECTOR_POT` (periodic systems, velocity gauge): `H = H_KS + A(t)·p`, observable = current J(t)

**Momentum matrix elements (VECTOR_POT):**
- `VecPHmatrix`: P^α_ij = i·vel·⟨φ_i|(∂/∂x_α + ik_α)|φ_j⟩
  - Gradient via `ApplyGradient` (finite-difference stencil)
  - k-correction: `psi_xC[i] += I_t * kptr->kp.kvec[0] * psi_C[i]`
- `CurrentNlpp`: adds i[V_NL, r_α] via `AppNls_0xyz(dir=1,2,3)`
- Total: P^α(total) = P^α(KE) + P^α(NL-PP)

**Current observable:**
- `J_α(t) = Re[Tr(P(t)·P^α)] = Re[zdotc(Pn0, Pxmatrix)] * kweight`
- BZ average: Σ_k w_k J^(k)_α, reduced over kpsub_comm and eldyn_comm

**Buffer aliasing (critical):**
- `Hmatrix_m1_cpu` has THREE sequential roles in one step: H(t-dt) → Ω → ⟨ΔV⟩
- `Hmatrix_cpu` never reset at step start; relies on vtot being incremental

### Eight Theory/Code Gaps Identified

| # | Severity | Description |
|---|----------|-------------|
| GAP-1 | CRITICAL | VECTOR_POT delta-kick only — A(t)cos(ωt) block is commented out |
| GAP-2 | MODERATE | Diamagnetic current J_dia = -n·A(t) absent |
| GAP-3 | MODERATE | A² term dropped, no weak-field regime check |
| GAP-4 | CRITICAL | Non-collinear spin: vtot missing off-diagonal V_xc spin components |
| GAP-5 | MODERATE | Hmatrix_m1_cpu triple aliasing — hazard for future refactoring |
| GAP-6 | MINOR | Hmatrix_cpu implicit accumulation — fragile convention |
| GAP-7 | MODERATE | No mode/boundary-condition guard (e.g. EFIELD + k-points silently wrong) |
| GAP-8 | MINOR | efield_tddft_crds means E₀ in EFIELD, A₀ in VECTOR_POT (undocumented) |

Evidence for GAP-1: lines 694-701 in `RmgTddft.cpp` contain commented-out code:
```cpp
// double coswt = cos(ct.tddft_frequency * tot_steps * time_step);
// daxpy(&n2_C, &coswtx, (double*)Kptr[kpt]->Pxmatrix_cpu, ...);
```

---

## 2. Code Annotations Added

### `TDDFT/RMG_TDDFT/RmgTddft.cpp`
- **File-level documentation block** added after line 21 (after copyright):
  - States the formalism: i dP/dt = [H(t),P(t)]
  - Lists the 3-step algorithm
  - Documents both perturbation modes
  - Lists all 8 GAPs with severity
- **Inline step annotations** throughout the time loop:
  - `// OUTER TIME LOOP header` with state variable legend
  - `// STEP 1 — PREDICTOR`
  - `// STEP 2 — CORRECTOR SCF loop`
  - `// STEP 2a — MAGNUS OPERATOR`
  - `// STEP 2b — PROPAGATE DENSITY MATRIX`
  - `// STEP 2c — DENSITY UPDATE`
  - `// STEP 2d — UPDATE POTENTIALS`
  - `// STEP 2e — UPDATE HAMILTONIAN`
  - `// CONVERGENCE CHECK`
  - `// STEP 3 — ADVANCE`
  - `// VELOCITY GAUGE INITIALIZATION (run once before time loop)`
  - `// GROUND-STATE CURRENT J_0`
  - `// TIME-DEPENDENT CURRENT J(t)`

### `TDDFT/RMG_TDDFT/VecPmatrix.cpp`
- **60-line file-level documentation block** explaining:
  - Why velocity gauge is required for PBC
  - Full formula: P^α_ij = i·vel·⟨φ_i|(∂/∂x_α + ik_α)|φ_j⟩
  - Bloch k-correction derivation from chain rule on e^{ik·r}u_{nk}(r)
  - Storage convention (factor i included in Pxmatrix)
- **Inline k-correction comment** at the three psi_xC/psi_yC/psi_zC lines
- **GEMM annotation** block

### `TDDFT/RMG_TDDFT/CurrentNlpp.cpp`
- **45-line file-level documentation block** explaining:
  - v = dr/dt = i[H,r] = p + i[V_NL, r] via Heisenberg equation
  - Separable NCPP formula for ⟨φ_i|i[V_NL,r_α]|φ_j⟩
  - What AppNls_0xyz computes in each direction
  - After this function, P^α = total current operator (KE + NL-PP)
- **Inline comment** at AppNls_0xyz calls

---

## 3. LaTeX Document: `docs/rmg_tddft_theory.tex`

**Purpose:** Foundation for a manuscript on RT-TDDFT in RMG. Documents theory, algorithm, and implementation together. Focuses on finite → periodic extension and velocity gauge. Critical analysis of gaps.

**Companion:** `docs/rmg_tddft_theory.bib` (14 bibliography entries)

**Status:** Compiled cleanly, 18 pages PDF at `docs/rmg_tddft_theory.pdf`

**Sections:**
1. Introduction — motivation, two paradigms (Gaussian vs real-space)
2. Theoretical Foundations
   - 2.1 TDKS equations
   - 2.2 Density matrix formulation
   - 2.3 External field coupling: length vs velocity gauge
   - 2.4 Velocity gauge: full theory
   - 2.5 Current density (paramagnetic + diamagnetic)
   - 2.6 Momentum matrix elements for Bloch states
3. **Pseudopotentials** (detailed section, added at Jacek's request):
   - Chemistry analogy: PP ≈ ECP in Gaussian codes
   - Norm-conserving PP: 4 conditions including norm conservation
   - Kleinman-Bylander separable form V_NL = Σ D_{lm}|β⟩⟨β|
   - Full Hamiltonian with NCPP
   - **Heisenberg derivation of i[V_NL, r_α]**: step-by-step using [AB,C]=A[B,C]+[A,C]B
   - What AppNls_0xyz computes and how CurrentNlpp assembles matrix elements
   - Hermiticity check (why MyConj symmetrization is needed)
   - NCPP vs USPP vs PAW comparison table
   - PP contribution to forces (Ehrenfest preview)
4. Propagation Algorithm — Magnus, predictor-corrector, BCH, diagev
5. RMG Implementation — basis, EFIELD mode, VECTOR_POT mode, buffer layout
6. Finite vs Periodic: Comparative Summary table
7. Known Gaps (all 8, with colored severity boxes)
8. Future Directions — sustained field, non-collinear spin, Ehrenfest, spin+Ehrenfest

---

## 4. LaTeX Document: `docs/tddft_memory_tutorial.tex`

**Purpose:** Graduate-level tutorial covering the TDDFT memory/dissipation/current-density questions that arose in discussion. Derivation-driven, not handwavy. Written at the level Jacek can follow and educate himself.

**Status:** Compiled cleanly, 22 pages PDF at `docs/tddft_memory_tutorial.pdf`

**Sections:**
1. Background: Runge-Gross theorem, adiabatic approximation defined
2. Memory: XC kernel as convolution integral; frequency domain f_xc(ω); explicit proof that polynomial (time-derivative) expansions cannot reproduce poles
3. Dissipation, Unitarity, Dephasing:
   - Magnus propagator preserves eigenvalues of P(t) — explicit proof
   - T1/T2 Bloch equations written out
   - **Jacek's atom-in-vacuum example stated explicitly as a Remark**
   - Reduced density matrix argument: why N-body unitary evolution → 1-body dephasing possible via collision integral C
4. Double Excitations:
   - **He atom derivation**: ⟨EE|x̂|GG⟩ = 0 step by step (Jacek's intuition confirmed)
   - First-order CI mixing: how |EE⟩ component enters eigenstates when V_ee turned on
   - Casida equations written out fully
   - Maitra-Burke pole formula
   - Perturbation theory energy shift formula
   - **Remark distinguishing one-photon vs two-photon access**
5. Gauge Invariance and Current-Density TDDFT:
   - Gauge transformation rules
   - Proof ρ is gauge-invariant
   - Proof J_p is NOT gauge-invariant (picks up ρ∇χ)
   - Proof J = J_p - ρA IS gauge-invariant
   - Why uniform A(t) is invisible to ρ in crystal
   - Vignale-Kohn theorem
   - **New: "What Is a Kernel?"** — integral operators, convolution, causal kernels, XC kernel as δV_xc/δρ, LDA as local kernel (delta function in space)
   - **New: "Elements of Continuum Mechanics"** — displacement field, strain tensor (volumetric + deviatoric), stress tensor, constitutive relations (Hooke → Newton → viscoelastic), derivation box recovering elastic and viscous limits from general G(τ) kernel
   - VK electron fluid as viscoelastic medium (now accessible after the above)
   - Viscosity kernels η_xc, ζ_xc: explicit formulas and frequency limits
6. Functional Forms and Implementation — scalar memory LDA extension, full VK requirements, Kramers-Kronig relations
7. Summary — one paragraph per question

---

## 5. Key Physics Insights Reached in Discussion

### Adiabatic Approximation
- V_xc^ATDDFT[ρ](r,t) = V_xc^gs[ρ(·,t)](r) — calls same ground-state routine at every step
- Memory = causal convolution with kernel f_xc(r,r',τ); adiabatic = f_xc = c·δ(τ)
- In frequency domain: adiabatic = constant; exact f_xc(ω) has poles at excitation energies
- Taylor expansion in ∂ρ/∂t = polynomial approximation in ω — cannot reproduce poles

### Resonance and Ringing (Jacek's good question)
- Driving with cos(ωt) DOES produce resonant buildup even in adiabatic TDDFT
- What adiabatic gets wrong: (a) slightly wrong resonance position, (b) no linewidth/dissipation, (c) missing double excitations
- My original statement was imprecise — corrected in the tutorial

### Unitarity and Dissipation (Jacek's insight was correct)
- Fundamental reason for no dissipation: Hermitian H_KS → unitary Magnus → eigenvalues of P conserved
- This is correct and independent of whether f_xc has memory
- T1 (population relaxation): requires bath (QED, phonons) — absent in all TDDFT
- T2 (dephasing): requires Im[f_xc(ω)] ≠ 0 — absent in adiabatic TDDFT only
- Resolution of paradox: N-body evolution is unitary; 1-body reduced density matrix P can dephase via collision integral C from electron-electron correlations

### Double Excitations (Jacek's intuition was right but about a different thing)
- Pure doubly excited states ARE dipole-forbidden: ⟨EE|x̂|GG⟩ = 0 (derived explicitly)
- But "double excitations" in TDDFT context = states with MIXED single+double CI character
- Coulomb mixing (CI) gives them nonzero dipole moment AND shifts their energy
- TDDFT failure is ENERGETIC: state appears but at wrong energy
- Pole in f_xc(ω) is needed to shift the Casida eigenvalue to correct position
- Two-photon operator x̂²/x̂ŷ reaches different states — distinct phenomenon (two-photon absorption)

### Gauge Invariance
- ρ is gauge-invariant: |e^{iχ}Ψ|² = |Ψ|²
- J_p is NOT: J_p → J_p + ρ∇χ
- J = J_p - ρA IS gauge-invariant: the ρ∇χ terms cancel
- For periodic solid: uniform A(t) doesn't change ρ(r,t) (density remains periodic) but DOES change J(t) → ρ alone cannot detect A(t) → need (ρ, J) together

### Viscoelastic VK Analogy (Jacek's car analogy formalized)
- Strain rate ė_αβ = symmetrized gradient of velocity = how fast electron fluid is being sheared
- Stress σ_xc(t) = ∫ η_xc(τ) ė_αβ(t-τ) dτ: depends on full deformation history
- Car analogy IS the viscoelastic constitutive relation: response today depends on trajectory, not just current position
- Silly putty example: elastic at high ω (fast deformation), viscous at low ω (slow)

---

## 6. Toy DFT/LDA Code — Plan (Not Yet Written)

**Agreement:** Write this after the tutorial LaTeX document. Language: Python + NumPy/SciPy only (~300-400 lines total).

**Design decisions made:**
- **Option 1 (start here):** Gaussian-smoothed all-electron:
  `V_ext = -Z * erf(r/r_c) / r` — same idea as Gygi approach, 2-line change
  - For He atom first (simplest case, Emil's concern about singularity addressed)
  - Grid: 50³ points, box 10 bohr
- **Option 2 (extension):** GTH norm-conserving PP (analytical form, no file parsing)
  - Parameters tabulated in code for H, He, C, N, O
  - Nonlocal projectors: Gaussian × polynomial, applied via AppNls analog
  - Grid can be coarser: 30³, 15 bohr box

**Why not standard all-electron:** Coulomb singularity -Z/r at nucleus → Kato cusp → needs h~0.05-0.1 bohr → 100³ grid → too expensive for toy code. Gygi smoothing solves this.

**Physics modules planned:**
- 3D grid (numpy arrays)
- Finite-difference Laplacian (7-point stencil, 2nd order)
- FFT Poisson solver (5 lines with numpy.fft)
- LDA XC: Dirac exchange ε_x = -(3/4)(3/π)^{1/3} ρ^{1/3} + VWN correlation
- SCF loop with linear mixing
- Observables: ρ(r) radial profile, orbital energies ε_i, energy components

**Connection to theory:** when document derives KS equations, code shows where each term lives in SCF loop. Option 1 → Option 2 transition demonstrates exactly what PP does (smooths core, same valence).

**Immediate next step for code:** Write it starting with He atom, Option 1.

---

## 7. Pseudopotential Discussion

Emil Briggs recently implemented Gygi's all-electron pseudopotential in RMG. The core issue is the Coulomb singularity on a real-space grid:
- Problem 1: Kato cusp condition — wavefunction gradient discontinuity at nucleus
- Problem 2: -Z/r diverges at grid point → requires very fine grid near nucleus

Gygi approach: replace point nucleus by Gaussian charge distribution:
`V_ext(r) = -Z·erf(r/r_c)/r`
- Smooth at r=0: V(0) = -2Z/(√π r_c)
- Recovers -Z/r for r >> r_c
- For toy code: this is literally 2 lines of code

GTH PP complexity: ~80-100 additional lines (parameter table + projector application). Full UPF file parsing: 200+ lines — not recommended for toy code.

---

## 8. Immediately Interrupted Question — RESOLVED

Jacek asked about **functional derivatives** before the prior session was interrupted.
This was resolved in the 2026-04-02 session: §2 of `tddft_memory_tutorial.tex` was
expanded from ~3 pages to ~12 pages and now covers all of the planned content plus more.
The "Functional Derivatives: A Primer for Quantum Chemists" section (§2) now includes:
- Definition, intuition, gradient analogy, Gâteaux derivative
- Dirac delta defined (B4), notation disambiguation box (D2)
- Four-step algorithm for computing functional derivatives (D3)
- Three worked examples: F=∫f²dx, Hartree energy, bilinear F=∬Kff
- Composite chain rule proved (A3)
- Second derivative, XC kernel, LDA f_xc with chain rule invoked explicitly
- Summary table, functional chain rule with example (B3)
- Gaussian-basis connections throughout (C1, C3)

---

## 9. Lessons Learned — 2026-04-02 Session (Tutorial Gap Patching)

### Strategy
- **One gap at a time, compile after each, never attempt full rewrite.**
  Both prior full-rewrite attempts timed out (~58 min). The one-gap-at-a-time
  strategy completed all 39 gaps in a single session without timeout.
- **grep → targeted read (±15 lines) → Edit.** Never re-read the full 29k-token
  file. Grep for the exact target text, read just enough context, apply Edit.
  This kept context usage comfortably under 50% across the full 39-gap session.
- **Two-pass pdflatex after every edit.** New `\label` commands require a second
  pass to resolve cross-references. Chain error check and output confirmation in
  one bash command: `grep -E "^!|^l\." | head -5 && ... | grep "Output written"`.

### Gap Dependencies
- **Some gaps resolve others.** A4 (Hartree derivbox: dummy-variable swap +
  Fubini named) resolved D5 before D5 was reached. Check for redundancy before
  patching a later gap rather than duplicating work.
- **The priority order in gap_list.md was correct.** B4 (Dirac delta) was a
  prerequisite for A2, A3, B1. D3 (four-step algorithm) set the framework that
  all worked examples referenced. Doing these first was right.

### Content Corrections
- **A8 was a real physics error, not just an omission.** The elastic limit
  `G(τ)=G₀δ(τ)` in the viscoelastic derivation box was wrong — a delta-function
  kernel gives viscous (Newton) behaviour, not elastic (Hooke). The correct
  elastic limit is `G(τ)=G₀` (constant, infinite memory). Gap patching must
  include catching and fixing actual errors, not only filling omissions.

### Handoff Protocol
- **`session_progress.md` is the primary handoff document.** A complete per-gap
  log (one line per gap, ✓ status, brief description of what was inserted) at the
  end of a session is what allows the next instance to pick up without rereading
  large files. The 39-entry log written at session end is the right granularity.
- **Context budget matters.** At the start of a gap-patching session, read only
  session_context.md + session_progress.md + gap_list.md + targeted file greps.
  Do NOT read the full tutorial file. The file grows to 40 pages (29k+ tokens)
  and reading it in full uses ~15% of context budget before any work is done.
