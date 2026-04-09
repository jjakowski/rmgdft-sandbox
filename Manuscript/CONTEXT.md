# Project Context
# RMG Periodic RT-TDDFT — Paper 2

## What this paper is

Extension of Jakowski et al. JCTC 2025 (hereafter "paper 1") to periodic
systems and spin-collinear systems. Paper 1 treated finite molecular systems
using the length gauge and the dipole moment as observable. This paper
introduces the velocity gauge, the current operator as observable, k-point
sampling, and spin-collinear propagation. The unique selling point is RMG's
scalability to very large periodic systems, demonstrated by the NV-center
in diamond showcase calculation.

Target journal: JCTC
Document class: achemso (journal=jctcce), same as paper 1
Template: Manuscript/source/paper1_main.tex

---

## Repository layout

### Git repository and branches
All branches are worktrees of the single repo: jjakowski/rmgdft-sandbox
(personal fork of the upstream RMGDFT/rmgdft)

| Branch | Local folder | Role for this project |
|--------|-------------|----------------------|
| master | rmgdft-sandbox/ | Stable release, strict upstream mirror. READ ONLY. |
| develop | rmgdft-sandbox-develop/ | Mirrors Emil and Wenchang's active development. Primary reference for code inspection. Never modify from here. |
| explore | rmgdft-sandbox-explore/ | Jacek's sandbox: contains develop code PLUS Jacek's annotations, exploratory changes, print statements, and prototypes. Also contains the Manuscript/ folder. May differ from develop in code files -- always check git diff origin/develop before using code from this branch. |
| develop-Ehrenfest | rmgdft-sandbox-Ehrenfest/ | Active Ehrenfest dynamics development. Separate from this manuscript. |

Key rule: explore is never merged into develop or master.
Full worktree details: see CLAUDE.md files in each worktree folder.

### Manuscript files (explore branch)
| Path | Purpose |
|------|---------|
| Manuscript/STATUS.md | Session state -- update at end of every session |
| Manuscript/CONTEXT.md | This file -- project knowledge |
| Manuscript/DECISIONS.md | Design choices log |
| Manuscript/notation.tex | Symbol and convention definitions |
| Manuscript/source/ | Read-only reference files -- see table below |
| Manuscript/paper2/ | Active draft: paper2_main.tex, section files |
| Manuscript/paper2/Figs/ | Benchmark figures (H2, hBN, Si, CrI3) |

### Source folder files (Manuscript/source/)
| File | Purpose |
|------|---------|
| paper1_main.tex | Full paper 1 manuscript -- template for paper 2 |
| references.bib | Primary bibliography -- use this for paper 2 |
| NEAT-notes-RMG_main.tex | Jacek's newer derivation notes (Gaussian units) -- primary source for sections 2b-2d |
| NEAT-notes-RMG_References.bib | Bib file for above |
| NEAT-project-notes_main.tex | Jacek's older, more complete notes (Gaussian units) -- required for section 2h (Berry phase) and Starace/hypervirial content |
| NEAT-project-notes_References.bib | Bib file for above |
| SOURCE_FILES_ANALYSIS.md | **Read before any theory drafting session.** Full inventory of both notes files: what is complete, what is wrong, what is already in the manuscript. |

**Rule: before working on any theory section, fetch and read
Manuscript/source/SOURCE_FILES_ANALYSIS.md first.**

### Code files (develop branch -- use this for theory-code verification)
| Path | Purpose |
|------|---------|
| TDDFT/RMG_TDDFT/ | Core RT-TDDFT routines (RmgTddft.cpp, CurrentNlpp.cpp, etc.) |
| TDDFT/ELDYN/ | Propagation engine (Magnus.cpp, commutp2.cpp, eldyn_ort.cpp) |
| Headers/rmg_tddft.h | Class definition for new refactored tddft class |
| Headers/prototypes_tddft.h | Function prototypes for all TDDFT routines |

### Fetch commands for new sessions
Fetch script location: /home/claude/fetch_github.py
Usage: python3 /home/claude/fetch_github.py "path/to/file" [branch]
Default branch is develop. Specify "explore" for manuscript files.

NOTE: raw.githubusercontent.com is blocked by network proxy.
Fetch via github.com HTML scraping -- works for all text files including .tex.

Examples:
  python3 /home/claude/fetch_github.py "Manuscript/STATUS.md" explore
  python3 /home/claude/fetch_github.py "Manuscript/source/SOURCE_FILES_ANALYSIS.md" explore
  python3 /home/claude/fetch_github.py "TDDFT/RMG_TDDFT/RmgTddft.cpp" develop

---

## Source file quick facts (full details in SOURCE_FILES_ANALYSIS.md)

### Units in the notes files
Both NEAT files use Gaussian (CGS) units. The manuscript uses atomic units.
Key conversion for electrons: set hbar=1, m=1, and q/hbar*c -> q -> -1.
Result: (1/2m)(p - qA/c)^2 in Gaussian becomes (1/2)(p + A)^2 in a.u.

### Known error -- do not copy
NEAT-notes-RMG_main.tex lines 784-796 writes -(1/2m)(kinetic)^2 with a
WRONG leading minus sign on the kinetic energy. Use the correct positive
sign from NEAT-project-notes_main.tex lines 950-962 or from the already-
written manuscript files.

### Content unique to NEAT-project-notes (not in RMG notes)
- Lines 785-851: Starace/hypervirial content. Velocity operator from [H,r].
  Nonlocal PP commutator mention. Hypervirial with correction term.
  -> Used for sec_theory_current.tex (already written).
- Lines 854-862: Berry phase polarization formula. -> Needed for section 2h.
- Lines 1012-1031: Continuity equation start (incomplete in notes).
  -> Completed in sec_theory_current.tex.

### What is already written (do not re-derive)
| Content | File |
|---------|------|
| Density matrix propagation + periodic KS | sec_theory.tex subsection 2.6 |
| Current operator full derivation (8 subsections) | sec_theory_current.tex |
| KB commutator [r,V^nl] full algebra + code connection | app_gauge_nlpp.tex |

---

## Agreed manuscript structure

### Section 1 - Introduction
Follow-up framing, cite competing periodic RT-TDDFT codes (Octopus, CP2K,
Qbox/Qball, GPAW, Exciting, SIESTA), mention memory DFT / TD-CDFT as future
directions, foreshadow NV-center unique capability.

### Section 2 - Methodology/Theory (single file: sec_theory.tex initially)
- 2a: EM interaction, plane wave exp(iwt-iq.r), long-wavelength/dipole approx
- 2b: Gauge transformation length -> velocity (required for periodic systems)
- 2c: Vector potential as foundation for magnetic perturbations (ECD future)
- 2d+2e (MERGED): Current operator, canonical vs mechanical momentum,
      paramagnetic + diamagnetic contributions, continuity equation,
      density matrix formulation, nonlocal PP correction [r,V^nl]
- 2f: Density matrix propagation (von Neumann / BCH) -- adapt from paper 1,
      add note about complex H for k not equal Gamma
- 2g: Spin-collinear extension
- 2h: Postprocessing -- J(t)->FFT->sigma(w)->epsilon(w), Berry phase alternative

### Section 3 - Implementation (sec_implementation.tex)
Algorithm flow diagram (analogous to Algorithm 1 in paper 1 but for
VECTOR_POT mode). Key simplifications justified here:
- Perturbative momentum kick (A at t=0 only, linear response)
- Diamagnetic term zero after t=0
- NCPP only
- First-order Magnus only
- BCH only for complex case

### Section 4 - Benchmarks (sec_benchmarks.tex)
- 4a: H2 chain (1D) -- vs Octopus, data available
- 4b: hBN (2D) -- vs Octopus, data available
- 4c: Si (3D) -- vs Octopus, data available
- 4d: CrI3 (spin-collinear) -- vs Octopus, optical only, data available
- 4e: NV-center diamond (showcase) -- data NOT YET available

### Appendices
Long derivations that are necessary for completeness but would interrupt the
narrative flow of the main text move to appendices. Expected content:
- Full gauge transformation of the nonlocal PP term -- WRITTEN: app_gauge_nlpp.tex
- Proof of equivalence between velocity gauge current and Berry phase polarization
- Possibly: spin-collinear extension of the BCH scheme

---

## Derivation philosophy (IMPORTANT -- applies to all drafting)

The manuscript aims to be a self-contained reference that can be used to
reproduce all derivation steps and logic years from now. Concretely:

1. DERIVE COMPLETELY FIRST. Write the full derivation including all
   intermediate steps. Decide where each piece lives (main text, appendix,
   SI) only after the derivation is complete. Never omit a step because
   "it is obvious" -- obvious to whom depends on background.

2. PLACEMENT HIERARCHY:
   - Main text: physical argument, key steps, final result. A reader should
     be able to follow the logic without pencil and paper.
   - Appendix: intermediate algebra needed to verify but not to understand.
     Examples: full gauge transformation of V^nl, orthogonalization proofs.
   - SI: numerical validation, extended data, convergence tests.

3. SHORTEN LAST. Start with the complete derivation and compress to fit
   journal constraints if necessary. Never start with a compressed derivation
   and try to expand it later.

4. COMPLETENESS OVER BREVITY in the derivation sections. Terseness is
   appropriate in the introduction and implementation sections but not in
   the theory section where completeness is the primary goal.

Paper 1 already follows this pattern: orthogonalization and nonorthogonal
basis propagation proofs are in the appendix, not the main theory section.
Follow the same pattern here.

---

## Writing style notes

### Source of truth
Paper 1 (Manuscript/source/paper1_main.tex) is the primary style reference.
All prose style decisions should be consistent with paper 1.

### Observed style characteristics from paper 1
- EQUATION-DENSE: Nearly every equation gets a \label{}. Equations are
  numbered and cross-referenced throughout.
- EXPLANATORY: Full sentences before and after every equation explaining
  what is being computed and what the symbols mean. No bare equation dumps.
- VOICE: Passive for methodology ("the propagation is performed using"),
  active for decisions and findings ("we will now show", "this allows for").
- ANNOTATION: Color commands \myred{}, \myblue{} used for tracked changes
  and revision notes during drafting. Use the same commands in paper 2.
- MACROS: \vb{} from physics package for bold matrix quantities (P, H, etc.).
  \imath for imaginary unit. Must be consistent with paper 1.
- STRUCTURE: Subsubsections used freely within theory section.
- CROSS-REFERENCES: Equations cross-referenced by \ref{}, not by "above/below."
- CITATIONS: \cite{} placed immediately after the claim, before the period.
  Multiple citations grouped: \cite{ref1,ref2,ref3}.

### Style target for paper 2
The derivation sections (sec_theory.tex) should match paper 1's completeness
and equation density. The surrounding narrative prose -- connective sentences
between derivation steps -- can be slightly tighter than paper 1: cut
redundant restatements of what an equation just showed, but keep all
physical interpretation sentences. The introduction and implementation
sections can be more concise than the theory section.
See DECISIONS.md for the formal decision on terseness level.

### What Claude should do when drafting
- Match paper 1 equation density and labeling style exactly.
- Write full explanatory sentences around equations.
- Use \myblue{} for all draft annotations, placeholders, and review flags.
  NEVER use \myred{} -- Jacek is color blind and cannot see red on screen.
- Flag any place where a derivation step is being skipped with a comment:
  % TODO: expand this step
- Never omit a derivation step silently.

---

## Theory-to-code connections (verified by code inspection)

| Physics | Theory | Code file | Function/variable |
|---------|--------|-----------|-------------------|
| Paramagnetic current matrix | p_ab=<a|(-ihbar grad+hbar k)|b> | VecPmatrix.cpp | VecPHmatrix() |
| k-vector Bloch correction | +hbar*k term | VecPmatrix.cpp L147-150 | psi_xC[i]+=I_t*kvec[0]*psi_C[i] |
| Nonlocal PP current | [r,V^nl]_ab | CurrentNlpp.cpp | CurrentNlpp(), AppNls_0xyz() |
| Magnus 1st order | Omega=-i/hbar*1/2*(H0+H1)*dt | Magnus.cpp | magnus() |
| H extrapolation | H1=2*H0-Hm1 | Magnus.cpp | extrapolate_Hmatrix() |
| BCH propagation | eq.23 paper1 | ELDYN/commutp2.cpp | commutp() |
| Density matrix prop | eq.23 paper1 | eldyn_ort.cpp | eldyn_ort() Ieldyn=1 |
| Density update | rho=rho_gs+sum_k P_k*|psi|^2 | GetNewRho_rmgtddft.cpp | GetNewRho_rmgtddft() |
| Current extraction | J_a=Re Tr[P*J_a_matrix] | RmgTddft.cpp L819-824 | zdotc(Pn0,Pxmatrix) |
| Berry phase | d=(eL/2pi)Im ln det S_a | RmgTddft.cpp L514,517 | Rmg_BP->tddft_Xml() |
| Spin handling | separate P_up,P_dn per spin | RmgTddft.cpp | rho.get_oppo(), compute_vxc |
| Mode switch | tddft_mode flag | RmgTddft.cpp L215,296 | VECTOR_POT vs EFIELD |

### C++ template dispatch
- <double,double>: Gamma-point real (finite, paper 1)
- <double,complex<double>>: Gamma-point complex
- <complex<double>,complex<double>>: k-points periodic (paper 2)

### Active vs inactive code
- RmgTddft.cpp: ACTIVE (called from Main.cpp)
- rmg_tddft.cpp: INACTIVE (class refactor, never called yet)
- CurrentOperator.cpp: DEAD CODE (if(0) block, never executes)
- eldyn_nonort.cpp: EXISTS but UNUSED (USPP path, not implemented)

---

## Identified gaps (from code inspection)

### Gap 1 -- i-factor sign consistency [RESOLVED in sec_theory_current.tex] 
VecPHmatrix() stores I_t*block_matrix (I_t=i) into Pxyz.
CurrentNlpp() also adds I_t*block_matrix.
Current extracted as Re(zdotc(P, Px)).
Physics: J_a = Re Tr[P * (-ihbar*grad + hbar*k + [r,V^nl])_ab]
Must trace sign and i-factor explicitly in derivation (section 2d).
Source of contentious discussion with Wenchang.

### Gap 2 -- CurrentOperator.cpp is dead code
Never called. Complex version computes wrong quantity (eigenvalue*r*psi).
Document in paper that current uses VecPHmatrix+CurrentNlpp.
Flag to Wenchang as cleanup item.

### Gap 3 -- Diamagnetic term absent, not documented
J_dia = (e/mc)*A(t)*rho = 0 after t=0. Valid for linear response.
Must state explicitly in sec_implementation. Limitation: no CW driving field.

### Gap 4 -- magnus() is real-only, complex path casts to double*
Element-wise 1/2*(H0+H1)*dt is mathematically correct for complex Hermitian
matrices, but the real-only comment and double* cast need documentation.

---

## Low-hanging fruit (from code inspection, session 1)

### Fruit 1 -- Berry phase vs current comparison [zero extra compute]
Infrastructure in code (BP_Xml, tddft_Xml). Add comparison figure.

### Fruit 2 -- Ground-state current -> 0 with k-points [zero extra compute]
Already printed by code (RmgTddft.cpp L503-509). Free sanity check.

### Fruit 3 -- Crystal symmetry of sigma(w) [zero extra compute]
Code applies Rmg_Symm->symm_vec(). Show sigma_xx=sigma_yy=sigma_zz for Si.

### Fruit 4 -- Spin current for CrI3 [trivial postprocessing]
J_spin = J_up - J_down trivially available from existing data.

### Fruit 5 -- Energy conservation periodic case [small extra compute]
Run longer Si simulation. Analogous to paper 1 Fig. 3c.

---

## Benchmark data availability

| System | RMG | Octopus | Notes |
|--------|-----|---------|-------|
| H2 chain (1D) | yes | yes | ready |
| hBN (2D) | yes | yes | ready |
| Si (3D) | yes | yes | ready |
| CrI3 (spin) | yes | yes | optical only, no MCD |
| NV diamond | no | no | not yet computed |

---

## Key references to cite (beyond paper 1)

- Mattiat & Luber JCTC 2022 -- velocity gauge, nonlocal PP correction
- Yabana & Bertsch PRB 1996 -- original real-space RT-TDDFT
- Octopus Castro et al. 2006 -- primary comparison code
- King-Smith & Vanderbilt PRB 1993 -- Berry phase polarization
- Resta PRL 1998 -- modern theory of polarization
- Kleinman & Bylander PRL 1982 -- separable pseudopotentials
- Pickard & Mauri PRB 2001 -- gauge origin, nonlocal potentials
- Starace PRA 1971 -- nonlocal PP correction to velocity operator
- Li et al. Chem Rev 2020 -- RT-TDDFT review
