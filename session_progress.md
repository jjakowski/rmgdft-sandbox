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
- 22-page LaTeX tutorial on memory effects, dissipation, double excitations, gauge invariance, current-DFT
- Graduate-level, derivation-driven (not handwavy)
- **Status:** Compiles cleanly, 22 pages PDF generated
- Compile: `pdflatex tddft_memory_tutorial` (no bibtex needed — no \cite commands)

### `docs/rmg_tddft_theory.pdf` — generated, 18 pages
### `docs/tddft_memory_tutorial.pdf` — generated, 22 pages
### `docs/rmg_tddft_theory.aux`, `.bbl`, `.blg`, `.log`, `.out`, `.toc` — LaTeX build artifacts

### `session_context.md` — this session's context record (just written)
### `session_progress.md` — this file

---

## Immediate Next Action (What Was Interrupted)

**Jacek asked:** Explain functional derivatives — specifically δV_xc/δρ (Eq. 51) and δ²E_xc/δρδρ' (Eq. 52) in `tddft_memory_tutorial.tex`.

**What to do next:**
1. Add a new subsection to `tddft_memory_tutorial.tex` titled **"Functional Derivatives: A Primer"** — place it at the START of Sec. 2 (before "Memory as a Convolution Integral") or as Sec. 1.5, so it appears before the first use of functional derivative notation.

2. Content to cover:
   - What a functional is (F: function → number) vs a function (f: number → number)
   - Functional derivative: δF/δf(x₀) = lim_{ε→0} [F[f + εδ(x-x₀)] - F[f]] / ε
   - Mental picture: "rate of change of F when f is nudged by a spike at x₀"
   - **Example 1** (simple): F[f] = ∫ f(x)² dx → δF/δf(x) = 2f(x)
     - Derivation: F[f+εδ(x-x₀)] = ∫(f+εδ)² dx = ∫f²dx + 2ε∫f(x)δ(x-x₀)dx + O(ε²) = F[f] + 2εf(x₀) → δF/δf(x₀) = 2f(x₀)
   - **Example 2** (nonlocal): F[f] = ∫∫ K(x,y)f(x)f(y)dxdy → δF/δf(x₀) = 2∫K(x₀,y)f(y)dy
   - **DFT application**: E_xc[ρ] = ∫ ε_xc(ρ(r))ρ(r)d³r → V_xc(r) = δE_xc/δρ(r)
     - LDA explicit: V_xc(r) = d[ρ·ε_xc(ρ)]/dρ |_{ρ=ρ(r)} (reduces to ordinary derivative because LDA is local)
   - **XC kernel**: f_xc(r,r') = δV_xc(r)/δρ(r') = δ²E_xc/δρ(r)δρ(r') — show explicitly this is the second functional derivative
   - LDA f_xc: f_xc^LDA(r,r') = d²[ρ·ε_xc]/dρ² · δ(r-r') — local in space (delta function)

3. After adding this, recompile: `pdflatex tddft_memory_tutorial`

---

## Open Threads — What Remains to Do

### Thread 1: Functional Derivative Primer (IMMEDIATE — session was interrupted here)
- **Action:** Add new subsection to `tddft_memory_tutorial.tex`
- **Location:** Before or at start of Sec. 2
- **Estimated size:** ~1.5 pages, 3-4 derivation boxes

### Thread 2: Toy DFT/LDA Python Code (NEXT MAJOR TASK)
- **Agreement:** Write after LaTeX tutorial document
- **Plan:** See `session_context.md` Sec. 6 for full design decisions
- **Start with:** He atom, Gaussian-smoothed all-electron (Option 1)
- **File to create:** Something like `toys/rmg_dft_toy.py` or `toys/lda_grid.py`
- **Estimated size:** ~300-350 lines for Option 1; +80-100 lines for GTH PP (Option 2)
- **Note:** Emil recently added Gygi all-electron PP to RMG — toy code Option 1 uses same approach

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
