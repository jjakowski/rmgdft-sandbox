# FROZEN RECORD — pre-refactor explore branch, pre May 2026 sync
# Do not edit.

# Session Progress — Concrete File State and Next Actions
**Date:** 2026-04-01
**Branch:** `explore`

---

## Files Modified (Existing Files with Annotations Added)

### `TDDFT/RMG_TDDFT/RmgTddft.cpp`
**What was added:**
- File-level documentation block (after line 21, after copyright block):
  - Full formalism statement (von Neumann equation)
  - 3-step algorithm description
  - Both perturbation modes (EFIELD, VECTOR_POT)
  - All 8 GAPs listed with severity and brief description
- Inline step annotations throughout the main time loop (STEP 1, 2a-2e, 3)
- Velocity gauge initialization block annotations
- Current observable computation annotations (J_0 ground state + J(t) time loop)

**Git status:** Modified (M) — not committed

---

### `TDDFT/RMG_TDDFT/VecPmatrix.cpp`
**What was added:**
- File-level documentation block (~60 lines) explaining:
  - Why velocity gauge is required for PBC
  - Full momentum matrix element formula with Bloch k-correction derivation
  - Storage convention (factor i included in Pxmatrix_cpu)
- Inline comment at k-correction lines (215-217)
- GEMM annotation block

**Git status:** Modified (implicitly via annotation — check `git diff`)

---

### `TDDFT/RMG_TDDFT/CurrentNlpp.cpp`
**What was added:**
- File-level documentation block (~45 lines) explaining:
  - Heisenberg equation v = i[H,r] = p + i[V_NL,r]
  - Separable NCPP commutator derivation
  - What AppNls_0xyz computes in each direction
  - Accumulation into Pxmatrix (+=, not =)
- Inline comment at AppNls_0xyz call sites

**Git status:** Modified (implicitly via annotation)

---

## New Files Created

### `docs/rmg_tddft_theory.tex`
- 18-page LaTeX document on RT-TDDFT theory/algorithm/implementation for RMG
- All 8 theory-code gaps documented with colored severity boxes
- PP section with Heisenberg derivation of i[V_NL,r_α] commutator
- Compile: `pdflatex rmg_tddft_theory && bibtex rmg_tddft_theory && pdflatex rmg_tddft_theory && pdflatex rmg_tddft_theory`
- **Status:** Compiles cleanly, 18 pages PDF generated

### `docs/rmg_tddft_theory.bib`
- 14 bibliography entries
- Covers: Magnus, Castro/Octopus, Yabana-Bertsch, Ismail-Beigi, Pickard-Mauri, Hamann-Schlüter-Chiang, Troullier-Martins, Kleinman-Bylander, Vanderbilt USPP, Blöchl PAW, King-Smith-Vanderbilt, Briggs/Bernholc RMG papers
- **Status:** Complete

### `docs/tddft_memory_tutorial.tex`
- ~~22-page~~ **40-page** LaTeX tutorial on memory effects, dissipation, double excitations, gauge invariance, current-DFT
- Graduate-level, derivation-driven (not handwavy)
- **Status: ALL 39 GAPS PATCHED. Compiles cleanly. 40 pages. 2026-04-01.**
- Compile: `pdflatex tddft_memory_tutorial && pdflatex tddft_memory_tutorial` (two passes for cross-refs; no bibtex needed)
- **Gap patch log (this session):**
  - B4 ✓ Dirac delta defined (keyresult box, sifting property, Kronecker contrast)
  - D2 ✓ Notation disambiguation box (δF/δf vs δ(x) vs δf)
  - D3 ✓ Four-step algorithm: Perturb → Subtract/divide → Single integral → Read off coefficient
  - A3 ✓ Composite chain rule proved: δ/δρ(r')[g(ρ(r))] = g'(ρ(r))δ(r−r')
  - B1 ✓ Second derivative worked: δ²(∫f²dx)/δf(x)δf(x') = 2δ(x−x'), grid check included
  - B2 ✓ Bilinear functional: first deriv = 2∫Kf dy, second deriv = 2K(x,x'); Casida/hybrid connection
  - C1 ✓ E_H[ρ] ↔ ½ΣP_{μν}P_{λσ}(μν|λσ); G_{μν} = ∂E_H/∂P_{μν}; RMG grid analogy
  - A1 ✓ Finite-grid proof: δF/δf(x_i) = (∂F/∂a_i)/h; verified on F=∫f²dx
  - A2 ✓ Sifting property invocation made explicit (intermediate equation added)
  - A4 ✓ Hartree derivbox: r↔r' dummy-variable swap shown; Fubini named and justified
  - A5 ✓ GGA IBP: perturbation generates ∇φ term; IBP identity; result Eq.(vxc_gga)
  - A6 ✓ E_tot = T_s + E_ext + E_H + E_xc decomposition defined; E_xc defined as remainder
  - A7 ✓ Functional Taylor expansion stated as keyresult (full series + linear-response truncation)
  - A8 ✓ Elastic/viscous limits corrected: G(τ)=G₀ (constant) → Hooke; G(τ)=ηδ(τ) → Newton
  - A9 ✓ Dyson equation motivated: KS responds to V_eff not V_ext; self-consistent loop explained
  - A10 ✓ Von Neumann equation derived from TDKS: i∂_t|ψ_n⟩=H|ψ_n⟩ → i Ṗ=[H,P]
  - A11 ✓ Polynomial/pole: polynomial is entire (no singularities anywhere); topological obstruction named
  - A12 ✓ Im f_xc role: linearised TDKS → Im f_xc adds imaginary damping → Γ_{nm}∝Im f_xc(ω_{nm})
  - B3 ✓ Chain rule: when f is a functional of g explained; concrete example f=K*g, F=∫f²
  - B5 ✓ Poles explained: resolvent (H−ω)⁻¹ diverges at eigenvalues; Lehmann representation stated
  - B6 ✓ One-sided FT: causality zeroes τ<0 half; = Laplace on real axis; e^{+iωτ} convention explained
  - B7 ✓ J=J_p−ρA motivated: canonical vs kinetic momentum (classical then quantum); diamagnetic term named
  - C2 ✓ K_{ia,jb} = (ia|jb) + (ia|f_xc|jb); comparison table CIS/RPA/TDDFT-LDA/exact
  - C3 ✓ V_xc^LDA multiplicative; KS matrix element (V_xc)_{μν} = ∫χ_μ V_xc χ_ν; RMG grid = quadrature
  - C4 ✓ f_n occupation numbers: Aufbau sets at t=0; frozen for all t>0; why T₁ impossible
  - C5 ✓ P_{ij}(t) MO vs P_{μν}(t) AO: transformation C†P_MO C; AO von Neumann has overlap S
  - C6 ✓ ε_xc(ρ): r_s = (3/4πρ)^{1/3} defined; Ceperley-Alder QMC; VWN parameterization named
  - C7 ✓ HK theorem stated before Runge-Gross; RG identified as time-dependent generalization of HK
  - C8 ✓ Test function φ explained; discrete analogue = δP_{μν} in Gaussian codes
  - D1 ✓ Gâteaux derivative: named (1913), analogy to directional derivative, vs Fréchet, no topology needed
  - D4 ✓ f_xc IS δV_xc/δρ = δ²E_xc/δρδρ' explicitly stated; static limit = Eq.(fxc_kernel_primer)
  - D5 ✓ Resolved by A4 (Fubini named inside Hartree derivbox)
  - D6 ✓ ½ factor in E_H: double-counting argument (each unordered pair counted twice)
  - D7 ✓ Adiabatic kernel derived: V_xc^ATDDFT(t) depends only on ρ(t) → δV_xc(t)/δρ(t')=0 for t'≠t
  - D8 ✓ f_xc^L (longitudinal, compressional, from ρ) and f_xc^T (transverse, shear, needs J) defined
  - D9 ✓ KK relations derived: causality → analyticity in upper half-plane → Cauchy theorem → dispersion
  - D10 ✓ Notation box: ε_{αβ} (full strain), e_{αβ} (deviatoric), ė_{αβ} (deviatoric rate, used in VK)
  - D11 ✓ Magnus: EOM vs solution distinguished; Ω≈½(H(t)+H(t+Δt))Δt shown; e^{−iΩ}Pe^{+iΩ} explained
  - D12 ✓ γ (interacting, C≠0) vs P (KS, C=0) contrast explicit; C is what P lacks for dephasing

### `docs/rmg_tddft_theory.pdf` — generated, 18 pages
### `docs/tddft_memory_tutorial.pdf` — generated, **40 pages** (updated 2026-04-01)
### `docs/rmg_tddft_theory.aux`, `.bbl`, `.blg`, `.log`, `.out`, `.toc` — LaTeX build artifacts

### `session_context.md` — this session's context record (just written)
### `session_progress.md` — this file

---

## Immediate Next Action

**Thread 1 (tutorial patching) is COMPLETE.** All 39 gaps patched 2026-04-01.
Document is at 40 pages, compiles clean with two passes of pdflatex (no bibtex).

---

## Open Threads — What Remains to Do

### Thread 1: Tutorial patching — **COMPLETE** (2026-04-01)
- All 39 gaps patched. `tddft_memory_tutorial.tex` is 40 pages, fully patched, compiles clean.
- `gap_list.md` is now a historical record only.

### Thread 2: Toy DFT/LDA Python Code — MOVED TO DEDICATED REPO (2026-04-02)
- **Repo:** `~/Development/RMG/grid-dft/` — `github.com/jjakowski/grid-dft`
- **Last state:** DESIGN.md complete (768 lines), pre-coding audit done (12 items), no code yet
- **Next:** STEP 3 — create YAML input files (`inputs/he_atom.yaml`, `inputs/h2_molecule.yaml`)
- **Continue in:** new Claude Code session opened inside `~/Development/RMG/grid-dft/`

### Thread 3: Ehrenfest Dynamics (Active Feature Branch: `develop-Ehrenfest`)
- **Open items from earlier sessions:**
  - GAP-4 in `rmg_tddft_theory.tex`: non-collinear spin incomplete in VECTOR_POT path — two "noncoll need change" markers in HmatrixUpdate.cpp
  - PP force term (Eq. for F^I_{NL}) derived in `rmg_tddft_theory.tex` Sec. 3.x.7 — needed for Ehrenfest
  - Real work lives in `rmgdft-sandbox-Ehrenfest/` branch `develop-Ehrenfest`
- **No code changes made in this session** to Ehrenfest branch

### Thread 4: Add Functional Derivative Content to rmg_tddft_theory.tex
- The PP section already uses functional notation but doesn't derive it
- Consider adding a brief mathematical appendix matching what will be added to the tutorial
- Lower priority than Thread 1

### Thread 5: GAP-1 Fix (VECTOR_POT sustained field)
- Lines 694-701 in RmgTddft.cpp contain the commented-out A(t)cos(ωt) update block
- Fixing requires: (1) uncomment and correct the daxpy; (2) add diamagnetic current (GAP-2); (3) add A² term (GAP-3)
- Must be done on `develop-Ehrenfest` or a new feature branch, NOT on `explore`
- No action taken in this session

---

## Build Instructions (Reminder)

### LaTeX documents
```bash
cd /Users/j2c/Development/RMG/rmgdft-sandbox-explore/docs/

# rmg_tddft_theory.pdf
pdflatex rmg_tddft_theory && bibtex rmg_tddft_theory && pdflatex rmg_tddft_theory && pdflatex rmg_tddft_theory

# tddft_memory_tutorial.pdf
pdflatex tddft_memory_tutorial && pdflatex tddft_memory_tutorial
```

### RMG code (explore branch)
```bash
source ~/source.miniconda
conda activate rmg-dev
cd ~/Development/RMG/rmgdft-sandbox-explore/build_explore/
make VERBOSE=1 -j 4 >& make.log
# Rebuild single file:
touch ../TDDFT/RMG_TDDFT/RmgTddft.cpp && make -j 4
```

---

## Key Reference: Equation Numbers in tddft_memory_tutorial.tex

For context on the interrupted question:
- **Eq. 51** (`\label{eq:fxc_kernel}`): `f_xc(r,r',t-t') = δV_xc(r,t)/δρ(r',t')` — the XC kernel as functional derivative
- **Eq. 52** (`\label{eq:fxc_static_def}`): `f_xc^gs(r,r') = δ²E_xc[ρ]/δρ(r)δρ(r')` — second functional derivative
- **Eq. 53** (`\label{eq:fxc_lda}`): `f_xc^LDA(r,r') = d²[ρε_xc(ρ)]/dρ² · δ(r-r')` — LDA local limit

These appear in Subsection 5.7.4 "The XC Kernel in DFT" (inside the new "What Is a Kernel?" subsection).
