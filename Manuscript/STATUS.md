# Paper 2 — Status File
# RMG Periodic RT-TDDFT Manuscript

## How to use this file
Fetch this file at the START of every new session before doing any work.
Update the "Current state" and "Last action" fields at the END of every session.
Push to GitHub immediately after updating.

---

## Current state

**Active file:** none yet — skeleton files not yet created  
**Active section:** none yet  
**Last action:** Created STATUS.md, CONTEXT.md, DECISIONS.md, notation.tex (session 1)  
**Next task:** Create paper2_main.tex and four skeleton section files  
**Session date:** 2026-04-07  

---

## Immediate next tasks (in order)

1. Create `paper2/paper2_main.tex` — master file with \input{} calls, same
   achemso document class as paper1 (journal=jctcce)
2. Create skeleton `paper2/sec_intro.tex`
3. Create skeleton `paper2/sec_theory.tex` with all subsection headings
4. Create skeleton `paper2/sec_implementation.tex`
5. Create skeleton `paper2/sec_benchmarks.tex`
6. Begin drafting sec_theory.tex starting with subsection 2a (EM interaction,
   long-wavelength approximation)

---

## Blocking questions / open issues

- Sign/i-factor consistency in VecPHmatrix() vs theory not yet verified
  (contentious point with Wenchang — must be resolved in sec_theory.tex 2d)
- Confirm with Wenchang: is CurrentOperator.cpp intentionally dead code?
- Berry phase implementation: confirm it produces correct results before
  including Berry phase comparison figure in benchmarks

---

## Recently completed sessions

### Session 1 (2026-04-07)
- Read and understood full project context from CLAUDE.md files
- Fetched and inspected all key TDDFT source files from develop branch:
  RmgTddft.cpp, rmg_tddft.cpp, CurrentOperator.cpp, CurrentNlpp.cpp,
  VecPmatrix.cpp, HmatrixUpdate.cpp, GetNewRho_rmgtddft.cpp,
  Magnus.cpp, eldyn_ort.cpp, eldyn_nonort.cpp
- Established theory-to-code map (see CONTEXT.md)
- Identified 4 open gaps and 4 low-hanging fruit (see CONTEXT.md)
- Agreed manuscript structure (4 sections, single sec_theory.tex initially)
- Created all four restart files

---

## Files to fetch at session start

### Every session (mandatory, in this order)
```
python3 /home/claude/fetch_github.py "Manuscript/STATUS.md" explore
python3 /home/claude/fetch_github.py "Manuscript/CONTEXT.md" explore
python3 /home/claude/fetch_github.py "Manuscript/DECISIONS.md" explore
python3 /home/claude/fetch_github.py "Manuscript/notation.tex" explore
python3 /home/claude/fetch_github.py "Manuscript/paper2/[active section file]" explore
```

### Manuscript writing session (add to mandatory list above)
```
# If working on style or structure questions, also fetch paper 1 as style reference:
python3 /home/claude/fetch_github.py "Manuscript/source/paper1_main.tex" explore
```

### Code inspection session (add to mandatory list above)
```
# Fetch CLAUDE.md files for full worktree and branch context:
python3 /home/claude/fetch_github.py "CLAUDE.md" explore

# Fetch specific TDDFT source files from develop (canonical implementation):
python3 /home/claude/fetch_github.py "TDDFT/RMG_TDDFT/RmgTddft.cpp" develop
python3 /home/claude/fetch_github.py "TDDFT/RMG_TDDFT/CurrentNlpp.cpp" develop
python3 /home/claude/fetch_github.py "TDDFT/RMG_TDDFT/VecPmatrix.cpp" develop
python3 /home/claude/fetch_github.py "TDDFT/RMG_TDDFT/HmatrixUpdate.cpp" develop
python3 /home/claude/fetch_github.py "TDDFT/ELDYN/Magnus.cpp" develop
python3 /home/claude/fetch_github.py "Headers/rmg_tddft.h" develop

# IMPORTANT: use develop branch for code verification, not explore.
# explore may contain Jacek's annotations and exploratory changes.
# Always check: git diff origin/develop  before using explore code as reference.
```

### Restart prompt template
Paste this at the start of a new session:

  I am continuing work on the RMG periodic RT-TDDFT manuscript (paper 2,
  follow-up to Jakowski et al. JCTC 2025). Please fetch and read these files
  from GitHub repo jjakowski/rmgdft-sandbox, branch explore, using the
  fetch_github.py script:
    Manuscript/STATUS.md
    Manuscript/CONTEXT.md
    Manuscript/DECISIONS.md
    Manuscript/notation.tex
    Manuscript/paper2/[active section file]
  After reading, summarize in 3-4 sentences: what the project is, where we
  are, and what the immediate next task is. Then we will proceed.
  Today we are working on: [specific section/task].

  For code inspection sessions, also add:
    CLAUDE.md  (explore branch)
    TDDFT/RMG_TDDFT/[relevant file]  (develop branch)
