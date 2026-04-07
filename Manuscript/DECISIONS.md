# Design Decisions Log
# RMG Periodic RT-TDDFT — Paper 2

## How to use this file
Log every non-trivial decision here with date and rationale.
Before making a decision that has been discussed before, check here first.
This prevents relitigating the same choices across sessions.

---

## Manuscript structure decisions

### 2026-04-07 — Theory subsections 2d and 2e MERGED
Decision: Combine "current operator" (2d) and "nonlocal pseudopotentials" (2e)
into a single subsection titled something like "Current Operator and Nonlocal
Pseudopotential Contributions."
Rationale: The PP correction [r,V^nl] is integral to defining the correct
current operator. Separating them forces the reader to mentally reconnect
them. The merged subsection flows more naturally.

### 2026-04-07 — Single sec_theory.tex initially, split later if needed
Decision: Keep all theory in one file sec_theory.tex until it exceeds
approximately 500 lines or becomes unwieldy. Split into per-subsection files
only when needed.
Rationale: Avoids premature fragmentation. Easier to navigate and search
in early drafting stage.

### 2026-04-07 — paper2/ subfolder for active draft
Decision: New paper lives in Manuscript/paper2/ not in Manuscript/ root.
Rationale: Keeps working area clean, separates active draft from template
and reference materials. paper1_main.tex stays in Manuscript/source/.

### 2026-04-07 — source/ subfolder for read-only reference documents
Decision: paper1_main.tex, existing derivation .tex files, and references.bib
go in Manuscript/source/ and are treated as read-only.
Rationale: Clear separation of reference vs active material. Collaborators
can see what is source vs draft.

---

## Theory and physics decisions

### 2026-04-07 — Perturbative kick only (no continuous A(t))
Decision: Paper 2 covers only the perturbative momentum kick regime
(A applied as delta function at t=0, then A=0 for all t>0).
The continuous driving field case (A(t) present throughout propagation)
is explicitly flagged as future work in section 3 and the outlook.
Rationale: This is what is actually implemented and validated.
The commented-out code in RmgTddft.cpp lines 571-579 shows the continuous
field path exists but is not activated.

### 2026-04-07 — Diamagnetic term: zero by construction, must be stated
Decision: Section 3 (implementation) must explicitly state that J_dia = 0
after t=0 because A(t>0) = 0, and that this is valid only in linear response.
Rationale: Solid-state readers will expect to see J_para + J_dia discussed.
Silence on J_dia would be confusing or look like an error.

### 2026-04-07 — Norm-conserving PP only
Decision: Paper 2 covers NCPP only. USPP would require S≠1 in the
propagation (eldyn_nonort.cpp path), which is not implemented.
State this as an explicit limitation in section 3.
Rationale: The code asserts this (RmgTddft.cpp line 287).

### 2026-04-07 — First-order Magnus only
Decision: Use only Omega_1 = -i/hbar * 1/2*(H0+H1)*dt.
The second-order term Omega_2 = (1/12*hbar^2)*dt^2*[H0,H1] is not included.
Forward-reference to paper 1 validation (energy conservation tests).
Rationale: Validated in paper 1. Sufficient for small dt.

### 2026-04-07 — BCH (Ieldyn=1) only for complex case
Decision: For k≠Gamma (complex H matrices), only the BCH commutator expansion
propagator is used (Ieldyn=1). The diagonalization path (Ieldyn=2) is not
supported for complex matrices (eldyn_ort.cpp explicitly errors on this).
State this in section 2f.
Rationale: This is the actual code behavior.

### 2026-04-07 — Berry phase included in section 2h
Decision: Include Berry phase polarization as an alternative observable
in section 2h, with comparison to current-integration result as a
validation figure for at least one benchmark system.
Rationale: Implementation exists in code (BP_Xml), costs nothing extra,
provides internal consistency check that reviewers will appreciate.

---

## Notation decisions (see also notation.tex for full definitions)

### 2026-04-07 — Atomic units throughout
Decision: Use atomic units (hbar=1, m_e=1, e=1, 4*pi*epsilon_0=1) throughout
theory sections. State this explicitly at the start of section 2.
Exception: SI units used when quoting experimental quantities or comparing
with experiment.
Rationale: Standard in electronic structure theory. Simplifies all equations.
Paper 1 uses atomic units. Consistency with paper 1 required.

### 2026-04-07 — imaginary unit symbol
Decision: Use \imath (i with no dot, rendered as i) consistent with paper 1.
Not \mathrm{i} or plain i.
Rationale: Paper 1 uses \imath throughout. Consistency required.

### 2026-04-07 — Electron charge sign convention
Decision: Electron charge is e > 0 (positive), so electron has charge -e.
Current density includes explicit minus sign: J = -e * v * rho.
Rationale: Standard physics convention. Avoids sign errors in J_para/J_dia.
Must be stated explicitly at first use of J to prevent confusion.

---


### 2026-04-07 -- Terseness level: section-dependent
Decision: Writing style is NOT uniform across the paper. Explicit rules:
  - sec_theory.tex: match paper 1 completeness and equation density exactly.
    Physical interpretation sentences kept. Restatement sentences (those that
    simply restate in words what the equation just showed) trimmed throughout.
    Derivations are NEVER compressed to save space -- move to appendix instead.
  - sec_intro.tex: more concise than paper 1. No redundant restatements.
  - sec_implementation.tex: terse and decision-focused. Results speak.
  - sec_benchmarks.tex: figure-driven, minimal prose.
Rationale: Theory section serves as long-term reference material (read
many times by students/postdocs). Introduction and implementation are read
once. Different audiences and use cases justify different styles.

### 2026-04-07 -- Writing style source of truth
Decision: Paper 1 (Manuscript/source/paper1_main.tex) is the definitive
style reference for all aspects of writing -- prose, equation formatting,
label naming, citation placement, use of color annotation commands.
When in doubt about any style question, check paper 1 first.
Rationale: Consistency between paper 1 and paper 2 is important since they
form a series and will often be read together.

## Open decisions (not yet resolved)

### OPEN — Exact title of merged subsection 2d+2e
Candidates:
  (a) "Current Operator and Nonlocal Pseudopotential Contributions"
  (b) "Electronic Current Density and Pseudopotential Corrections"
  (c) "The Current Operator in the Velocity Gauge"
Status: Defer until section is drafted.

### OPEN — Whether to include spin current J_spin = J_up - J_down for CrI3
Decision pending availability of separate spin-channel current data.
See Fruit 4 in CONTEXT.md.

### OPEN — How many NV-center supercell sizes to include
Minimum 4, ideally 5, spanning order of magnitude in defect concentration.
Pending: computations not yet started.
