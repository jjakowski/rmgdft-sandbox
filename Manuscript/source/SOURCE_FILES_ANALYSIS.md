# Source Files Analysis
# Manuscript/source/SOURCE_FILES_ANALYSIS.md
#
# Purpose: Quick-reference guide for any Claude session working on paper 2.
# Read this BEFORE opening the derivation .tex files. It tells you what is
# in each file, what is usable, what is incomplete, and where the errors are.
# Last updated: April 2026 (after full read of both derivation files).

---

## Files in this folder

| File | Size | Description |
|------|------|-------------|
| `paper1_main.tex` | ~104 KB | Full manuscript of Jakowski et al. JCTC 2025 (paper 1). Read-only reference. |
| `references.bib` | ~96 KB | Bibliography for paper 1. Contains essentially all references needed for paper 2. |
| `NEAT-notes-RMG_main.tex` | ~52 KB | Jacek's newer, cleaner derivation notes. Restructured from project notes. Gaussian units. **Recommended starting point for theory sections 2b–2d.** |
| `NEAT-notes-RMG_References.bib` | ~16 KB | Bibliography for the RMG notes file. |
| `NEAT-project-notes_main.tex` | ~62 KB | Jacek's older, more complete working notes. Contains material not in the RMG notes. Gaussian units. **Required reading for section 2h (Berry phase) and the nonlocal PP / Starace discussion.** |
| `NEAT-project-notes_References.bib` | ~16 KB | Bibliography for the project notes. Same keys as the RMG notes bib. |

---

## paper1_main.tex — what to use from it

This is the template for paper 2. Key sections to adapt:

| Paper 1 section | Adapt for paper 2 | Notes |
|-----------------|-------------------|-------|
| Abstract | Rewrite entirely — periodic systems, velocity gauge, k-points | Keep 4-sentence structure |
| Introduction | Update: add periodic RT-TDDFT context, competing codes | Keep cite pattern |
| Section 2.2 (Density matrix) | Adapt for paper 2 section 2f | Nearly identical; add note on complex H for k≠Γ |
| Magnus expansion (eqs. 8–13) | Cite rather than repeat in paper 2 | Cross-reference paper 1 |
| BCH commutator expansion (eq. 23) | Cite rather than repeat | Cross-reference paper 1 |
| Section 3 (Implementation) | Template for paper 2 section 3 | Different algorithm needed |
| Figures style | Follow exactly | Same journal |

The density matrix propagation subsection (`\subsubsection{Density matrix propagation}`) has been adapted for paper 2 and now lives in `Manuscript/paper2/sec_theory.tex` (subsection 2.6). Do not re-derive from paper 1 — use the version already in sec_theory.tex.

---

## NEAT-notes-RMG_main.tex — detailed inventory

**Title in file:** "NEAT-project-notes" (same title as the older file — confusing but correct)
**Date:** November 2024
**Units:** Gaussian (CGS) throughout — convert to atomic units for manuscript
**Status:** Newer and cleaner than project notes; some sections still incomplete

### Section structure

| Lines | Section | Content | Usability |
|-------|---------|---------|-----------|
| 116–163 | Classical Hamiltonian | Lagrangian → canonical momentum → Legendre → H = (p−qA)²/2m + qφ. Full derivation. | ✅ Complete. Foundation for section 2a. |
| 165–243 | Quantum Hamiltonian + gauge invariance of E, B | E/B fields invariant under gauge transform. Vanderbilt Hamiltonian with Zeeman term. | ✅ Complete. Use for section 2a/2b. |
| 252–295 | LG and VG potentials | States A^LG=0, φ^LG=r·E; A^VG=−c∫E dt, φ^VG=0. Gauge function χ=−r·A. | ✅ Ready to transcribe to section 2b/2c. |
| 302–428 | General gauge transformation of TDSE | Derives: (1) wavefunction phase factor e^{iqχ/ℏc}, (2) gauge transformation rules for Ψ, Ô, p̂, π̂. Sets β=q/ℏc. | ✅ Complete and rigorous. Use for section 2b. |
| 429–472 | Example 1: gauge invariance of kinetic momentum | Verifies ⟨Ψ|π̂|Ψ⟩ = ⟨Ψ'|π̂'|Ψ'⟩. Full calculation. | ✅ Ready. |
| 475–497 | Example 2: explicit transformation of π̂ | Shows π̂' = U π̂ U† = p̂' − (q/c)(A+∇χ). | ✅ Ready. |
| 499–523 | Example 3: transformation of ∂/∂t operator | Derives iℏ∂_t → (q/c)χ̇ + iℏ∂_t. | ✅ Ready. |
| 526–605 | Example 4: TDSE gauge transformation | Derives H' = UHU† − (q/c)χ̇. Full derivation of velocity gauge H. | ✅ Ready. |
| 607–716 | Length vs velocity gauge derivation | LG Hamiltonian → VG Hamiltonian via χ=−r·A₀. Shows dipole term cancels. VG boxed result includes UVU†. | ⚠️ Mostly complete. See **ERROR** note below. |
| 719–753 | Unfinished notes: velocity gauge matrix elements | LG and VG Hamiltonians written. Paramagnetic/diamagnetic identification. Linear response matrix elements. | ⚠️ Incomplete but useful for section 2d. |
| 754–805 | Maxwell + TDKS for periodic systems | Bloch state form. TDKS for Bloch orbital. Current density from Bloch states. | ✅ **Key content for section 2d.** Use Bloch current formula (lines 800–805). **See SIGN ERROR note below.** |

### ERRORS in NEAT-notes-RMG_main.tex

**Sign error — kinetic energy in Bloch TDKS (lines 784–796):**
The file writes `−(1/2m)(−iℏ∇ + ℏk + eA/c)²` with a leading minus sign on the kinetic energy. This is **wrong**. The correct expression is `+(1/2m)(−iℏ∇ + ℏk + eA/c)²`. The correct sign appears in NEAT-project-notes_main.tex (lines 950–962). **Do not copy the kinetic energy sign from this file.**

**Self-note on sign of q (line 297, commented out in later version):**
The original notes flagged "check the sign — the minus sign in front of A often comes from q=−1 for electrons." This is resolved: in atomic units with e_electron = −1, the Hamiltonian is H = (1/2)(p + A)² for electrons. No ambiguity remains.

---

## NEAT-project-notes_main.tex — detailed inventory

**Title in file:** "NEAT-project-notes"
**Date:** November 2024
**Units:** Gaussian (CGS) throughout — convert to atomic units for manuscript
**Status:** Older and less organized but **contains essential content** not in the RMG notes

### Section structure

| Lines | Section | Content | Usability |
|-------|---------|---------|-----------|
| 112–150 | Introduction | Milestones 1–5 (velocity gauge → vector potential → spin → Ehrenfest). | 📌 Context only, not for manuscript |
| 159–250 | Momentum operator overview | Same content as RMG notes section 1. Duplicate. | Use RMG notes version. |
| 200–250 | Classical Hamiltonian derivation | Full Lagrangian derivation. Duplicate of RMG notes. | Use RMG notes version. |
| 256–296 | General rt-TDDFT Hamiltonian | Vanderbilt Hamiltonian. LG/VG potentials. Gauge function. | Use RMG notes version. |
| 271–425 | Gauge transformation of TDSE | Same Examples 1–4 as RMG notes but at paragraph level (less prominent in TOC). | Use RMG notes version. |
| 635–716 | Length vs velocity gauge | Same derivation as RMG notes. χ=−r·A₀ derivation. | Use RMG notes version. |
| 748–783 | tmp: velocity gauge matrix elements | LG/VG Hamiltonians. Paramagnetic/diamagnetic. Duplicate of RMG notes section but labeled "tmp". | Skip — use RMG notes version. |
| 785–851 | **Starace / nonlocal PP / hypervirial** | **Unique content.** Local vs nonlocal potential distinction. Heisenberg picture: ṙ = i[H,r]. Velocity matrix elements from commutator. Translational symmetry and nonlocal PP. Hypervirial relation with nonlocal correction eq. ⟨j|p|n⟩ = (εₙ−εⱼ)⟨j|r|n⟩ + ⟨j|[r,V^nl]|n⟩. | ✅ **Critical for section 2d and Appendix A.** |
| 854–862 | **Berry phase polarization** | **Unique content.** Gamma-point Berry phase formula: ⟨μᵢ⟩ = −f_occ(eLᵢ/2π)Im ln det ⟨φₙ|e^{−2πix̂/L}|φₘ⟩. From Octopus PCCP 2015. | ✅ **Use for section 2h.** |
| 864–880 | **Hypervirial relation** | **Unique content.** Standard hypervirial plus nonlocal correction. Gauge origin problem for real-space grids. | ✅ **Use for section 2d / hypervirial check.** |
| 884–922 | Velocity gauge sketch (Luber) | Outline of LG→VG transformation following Luber. Incomplete — stops before finishing. | ⚠️ Incomplete. Useful only as a checklist of steps. |
| 925–1010 | Maxwell + TDKS (periodic) | Bloch state representation. TDKS for Bloch orbital u_{n,k}. **Correct positive sign on kinetic energy.** Current density formula (lines 966–971). | ✅ **Use Bloch current formula. Correct sign here.** |
| 1012–1035 | **Density current derivation** | **Unique content — incomplete.** Starts from continuity equation, differentiates ρ=Ψ†Ψ, inserts TDSE. Stops at line 1027 with J = (i/ℏ)(...) left blank. Hamiltonian hermiticity stated but current not extracted. | ⚠️ **Incomplete. Must be completed for section 2d.** The completion is now in sec_theory_current.tex. |
| 1038–1047 | Light-matter interactions | Bullet points only. No content. | 🗑️ Ignore. |

### Key equations from NEAT-project-notes to use

**Velocity operator from Heisenberg EOM (lines 812–837):**
```
ṙ(t) = i[H,r]
⟨a|ṙ|b⟩ = i⟨a|[H,r]|b⟩ = i⟨a|[T+V,r]|b⟩
         = ⟨a|p̂|b⟩ + i⟨a|[V,r]|b⟩
```
For local V: [V,r] = 0 → velocity = p̂ (standard result).
For nonlocal V: [V,r] ≠ 0 → extra Starace term.

**Hypervirial with nonlocal PP (lines 873–878):**
```
i⟨φⱼ|p̂|φₙ⟩ = (εₙ−εⱼ)⟨φⱼ|r̂|φₙ⟩ + ⟨φⱼ|[r̂,V^nl]|φₙ⟩
```
This is the key equation connecting velocity gauge to length gauge. If [r̂,V^nl] is omitted, the two gauges disagree → wrong spectra.

**Bloch current density (lines 966–971):**
```
j(r,t) = (1/m) Re Σ_{n,k} u*_{n,k}(r,t) · (−iℏ∇ + ℏk + eA/c) · u_{n,k}(r,t)
```
This is the correct formula with all three terms. The k-vector term is highlighted in blue in the source. **Correct kinetic sign here** — do not use the RMG notes version for this equation.

---

## Unit conversion rules (Gaussian → atomic units)

All derivations in both notes files use Gaussian (CGS) units. The manuscript uses atomic units. Conversion rules:

| Gaussian | Atomic units | Note |
|----------|-------------|------|
| ℏ | 1 | |
| m_e | 1 | |
| e (charge magnitude) | 1 | |
| c | 1/α ≈ 137 | α = fine structure constant |
| q = −e (electron) | −1 | |
| (q/ℏc)χ in phase | −χ | since q=−1, ℏ=1, c→1 in phase factor |
| (1/2m)(p − qA/c)² | (1/2)(p + A)² | for electron with q=−1, c=1 |
| E = −∂A/∂t − ∇φ | same | |
| χ = −r·A | same (A is now in a.u.) | |

**Practical rule:** In all expressions, set ℏ=1, m=1, and replace q/c → q → −1 for electrons. Every (q/ℏc) factor becomes −1.

---

## What has already been derived in the manuscript (as of April 2026)

These derivations from the notes have been transcribed and completed:

| Content | Source in notes | Status in manuscript |
|---------|-----------------|---------------------|
| Density matrix propagation (BCH) | paper1_main.tex sec 2.2 | ✅ In sec_theory.tex subsection 2.6 |
| k-dependent Kohn-Sham equation | RMG notes lines 793–805, project notes lines 960–971 | ✅ In sec_theory.tex subsection 2.6 |
| Current operator: velocity from Heisenberg EOM | project notes lines 812–837 | ✅ In sec_theory_current.tex |
| Current from continuity equation | project notes lines 1012–1031 (incomplete) | ✅ Completed in sec_theory_current.tex |
| Para/diamagnetic decomposition | RMG notes lines 737–753 | ✅ In sec_theory_current.tex |
| k-vector correction for Bloch states | project notes lines 960–971 | ✅ In sec_theory_current.tex |
| Hypervirial relation + nonlocal correction | project notes lines 864–878 | ✅ In sec_theory_current.tex |
| Sign convention (i-factor in code) | code inspection | ✅ In sec_theory_current.tex |
| Gauge transformation LG→VG (H^VG) | RMG notes 607–716, project notes 635–716 | ✅ In app_gauge_nlpp.tex |
| KB commutator [r_α, V^nl]: full algebra | project notes 785–851 (partial) | ✅ Completed in app_gauge_nlpp.tex |
| KB commutator matrix element + code connection | code inspection | ✅ In app_gauge_nlpp.tex |

## What still needs to be derived / written

| Content | Source to use | Target section |
|---------|--------------|---------------|
| EM interaction, plane wave, long-wavelength approx | RMG notes lines 252–295 | sec_theory.tex 2.1/2.2 |
| Gauge transformation Examples 1–4 | RMG notes lines 429–605 | sec_theory.tex 2.3 |
| Full LG→VG derivation in manuscript style | RMG notes lines 607–692 | sec_theory.tex 2.3 |
| Spin-collinear extension | (no notes — needs new derivation) | sec_theory.tex 2.7 |
| Berry phase polarization | project notes lines 854–862 | sec_theory.tex 2.8 |
| Berry phase ↔ current integration equivalence | (not in notes) | sec_theory.tex 2.8 or Appendix |

---

## Reference key concordance

Both bib files use the same cite keys. Key keys used in the theory sections:

| Cite key | Reference |
|----------|-----------|
| `Sakurai-2nd_ed` | Sakurai & Napolitano, Modern Quantum Mechanics, 2nd ed. |
| `Mattiat-Luber-2022` | Mattiat & Luber, JCTC 2022 — velocity gauge RT-TDDFT, nonlocal PP correction |
| `Luber-rt-tddft` | Luber, JCTC 2021 — RT-TDDFT review |
| `rt-DFTB-Wong-2023` | Wong et al., JCTC 2023 — RT-DFTB velocity gauge |
| `Xiaosong-Li-2011-gauge` | Xiaosong Li, 2011 — gauge transformation in RT-TDDFT |
| `starace-1971` | Starace, PRA 1971 — nonlocal PP correction to velocity operator |
| `kleinman-bylander-1982` | Kleinman & Bylander, PRL 1982 — separable pseudopotentials |
| `yabana-bertsch-1996` | Yabana & Bertsch, PRB 1996 — original real-space RT-TDDFT |
| `octopus-rt-tddft-PCCP-2015` | Octopus PCCP 2015 — Berry phase in RT-TDDFT |
| `octopus-rt-tddft-JCP-2020` | Octopus JCP 2020 — Maxwell-TDDFT |
| `prb-2018-Yabana` | Yabana et al., PRB 2018 — macroscopic Maxwell-TDDFT |
| `spin-rt-tddft-Li-2014` | Li et al., 2014 — nonrelativistic spin dynamics |
| `spin-rt-tddft-Li-2016` | Li et al., 2016 — two-component spin dynamics |
| `Vanderbilt-book` | Vanderbilt, Berry Phases in Electronic Structure Theory (Cambridge) |
| `Springborg-book-2000` | Springborg, Methods of Electronic-Structure Calculations |
| `jakowski-rmg-tddft-2025` | Jakowski et al., JCTC 2025 (paper 1) |

The `references.bib` file (paper 1 bibliography) is the most complete and should be used as the primary bib file for paper 2. The two NEAT bib files are subsets of it.

---

## Quick checklist for a new Claude session

Before drafting any theory section:

1. Read this file (you are doing that now).
2. Fetch `Manuscript/STATUS.md` — find out what section is currently being drafted.
3. Fetch `Manuscript/DECISIONS.md` — check for any relevant formatting or convention decisions.
4. Fetch `Manuscript/notation.tex` — verify macro names before writing any LaTeX.
5. For theory sections 2b–2d: primary source is `NEAT-notes-RMG_main.tex`. Use NEAT-project-notes for the Starace/hypervirial content and the Bloch current formula.
6. For theory section 2h (Berry phase): primary source is `NEAT-project-notes_main.tex` lines 854–862.
7. For sections already written (`sec_theory_current.tex`, `app_gauge_nlpp.tex`): fetch those files directly — do not re-derive.
8. **Never copy the kinetic energy sign from NEAT-notes-RMG lines 784–796. Use the correct positive sign from NEAT-project-notes lines 950–962.**
