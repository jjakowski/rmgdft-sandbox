
# RMG-DFT: Upstream Sync and Annotation Preservation Plan
*Created: May 2026 — Personal reference for Jacek Jakowski*

---

## Why This Document Exists

This document was created at a specific moment in the development history of
the RMG-DFT project. Here is what was happening at that time and why decisions
were made the way they were.

### Background

I (Jacek Jakowski) am developing RT-TDDFT extensions in RMG-DFT, working in a
personal fork (`jjakowski/rmgdft-sandbox`) of the upstream repo
(`RMGDFT/rmgdft`). I maintain several git worktrees locally, each checked out
on a different branch, so I can work on multiple things simultaneously without
switching branches.

Over several months, I had been using the `explore` branch/worktree as a
sandbox for two purposes:

1. **Annotating the RT-TDDFT source code** — adding detailed physics
   explanations as comments directly into the C++ source files. This was
   important because the code was written by collaborators (Emil Briggs,
   Wenchang Lu) in a style unfamiliar to me (real-space grid C++), and I
   needed to reconstruct the physics formulas being implemented in order to
   build on top of them.

2. **Writing a manuscript** — the `Manuscript/` folder and `docs/` folder in
   the `explore` branch contain a growing LaTeX manuscript and theory notes
   for a paper on RT-TDDFT with nonlocal pseudopotentials and vector-potential
   perturbations.

### The Problem This Document Addresses

When I was ready to sync my local repo with the upstream (which had received
significant updates and refactoring), I discovered:

- **`RmgTddft.cpp` was deleted entirely from upstream** — the file I had
  carefully annotated no longer exists in the new version of the code. The
  refactoring replaced it with something else (new file name, split into
  multiple files, or reorganized).

- **`CurrentNlpp.cpp` and `VecPmatrix.cpp` were unchanged** in upstream —
  my annotations in those two files can be safely carried forward.

- **The `explore` branch also contains my entire manuscript** — simply
  overwriting or force-rebasing `explore` would destroy months of manuscript
  work that lives only there.

This created a conflict: I need to sync with upstream (to get bug fixes,
performance improvements, and new features), but I also need to preserve my
annotations and manuscript, and understand how the refactored code relates
to the old annotated code.

### The Solution

The solution has four parts:

1. **Create a git tag** (`pre-TDDFT-annotation`) that permanently marks the
   exact state of `develop` at this moment — before the upstream sync. This
   tag is the anchor point for all future comparisons. It can never be lost,
   even after the sync rewrites the branch history.

2. **Leave `explore` frozen** — never rebase it onto the new upstream. It
   becomes a permanent historical reference containing both the annotated
   (pre-refactor) source files and the manuscript.

3. **Sync `develop` with upstream** — bring in all the refactored code.

4. **Create a fresh `explore-new` branch** off the updated `develop` — a
   clean slate for future exploration of the refactored codebase.

---

## Repository and Worktree Map

| Local directory | Branch | Purpose |
|---|---|---|
| `rmgdft-sandbox/` | `master` | Strict upstream mirror — never modified directly |
| `rmgdft-sandbox-develop/` | `develop` | Integration branch — synced with upstream |
| `rmgdft-sandbox-explore/` | `explore` | **FROZEN** — annotations + manuscript (see below) |
| `rmgdft-sandbox-Ehrenfest/` | `develop-Ehrenfest` | Active Ehrenfest dynamics feature development |
| `rmgdft-sandbox-explore-new/` | `explore-new` | Clean exploration of refactored upstream code |

**Remote remotes:**
- `origin` — personal fork: `github.com/jjakowski/rmgdft-sandbox`
- `upstream` — official repo: `github.com/RMGDFT/rmgdft`

---

## The Tag: `pre-TDDFT-annotation`

**What it marks:** The exact commit of `develop` at the moment just before
the upstream sync was performed in May 2026. At this commit, `develop`
contained the pre-refactor version of `RmgTddft.cpp` — the same version
that was annotated in `explore`.

**Where it lives:** Both in local repo and pushed to `origin`.

**Why it matters:** After the upstream sync, `develop` will point to the
refactored code and `RmgTddft.cpp` will be gone. The tag preserves permanent
access to the old version for comparison.

**How to create it (if for some reason it needs to be recreated):**
```bash
# This should already be done — check first:
git tag | grep pre-TDDFT

# If missing, recreate on the pre-sync commit:
git tag pre-TDDFT-annotation <commit-sha>
git push origin pre-TDDFT-annotation
```

---

## The Frozen `explore` Branch

The `explore` branch was intentionally left un-rebased onto the new upstream.
It contains two categories of work that must be preserved:

### Category 1 — Annotated C++ source files (3 files)

These files have detailed physics explanations added as block comments:

| File | What the annotation explains |
|---|---|
| `TDDFT/RMG_TDDFT/RmgTddft.cpp` | Full RT-TDDFT algorithm: predictor-corrector Magnus propagator, velocity gauge vs. length gauge, known theory gaps (GAP-1 through GAP-8) |
| `TDDFT/RMG_TDDFT/CurrentNlpp.cpp` | Nonlocal PP correction to the current operator: theory of i[V_NL, r] commutator |
| `TDDFT/RMG_TDDFT/VecPmatrix.cpp` | Momentum matrix elements for velocity gauge: Bloch k-correction, GEMM assembly |

**Important:** `RmgTddft.cpp` was deleted entirely in the upstream refactoring.
The annotated version in `explore` is the only surviving documented copy of
the pre-refactor algorithm. It is the Rosetta Stone for understanding what
the new code does.

### Category 2 — Manuscript and documentation

```
Manuscript/          — LaTeX manuscript (paper2) on RT-TDDFT with NL-PP
docs/                — Theory notes: rmg_tddft_theory.tex, tddft_memory_tutorial.tex
CLAUDE.md            — Project context notes for Claude Code sessions
session_context.md   — Session state snapshots
session_progress.md  — Development progress log
```

---

## Step-by-Step Execution Plan

### PHASE 1 — Create the tag (do this FIRST)

```bash
cd ~/Development/RMG/rmgdft-sandbox-develop/
git tag pre-TDDFT-annotation
git push origin pre-TDDFT-annotation

# Verify:
git tag | grep pre-TDDFT
```

### PHASE 2 — Understand what upstream changed in TDDFT (before syncing)

#  JACEK: see what Emil changed  wrt to   Jacek  version (right before Jacek  changes were added)
# git diff develop..upstream/develop --stat -- TDDFT/

```bash
cd ~/Development/RMG/rmgdft-sandbox/
git fetch upstream

# See all files added/removed/modified in TDDFT directory:
git diff develop..upstream/develop --stat -- TDDFT/

# See specifically what replaced RmgTddft.cpp:
git diff develop..upstream/develop --name-status -- TDDFT/

#-- above command should show  which files were changed by Emil wrt to   Jacek  version (right before Jacek  changes were added)  
# added/removed/modified:
# M       TDDFT/ELDYN/commutp2.cpp
# M       TDDFT/RMG_TDDFT/CMakeLists.txt
# D       TDDFT/RMG_TDDFT/RmgTddft.cpp
# A       TDDFT/RMG_TDDFT/rmg_rotate_sint.cpp
# M       TDDFT/RMG_TDDFT/rmg_tddft.cpp
# M       TDDFT/RMG_TDDFT/rmg_tddft_energy.cpp
```

### PHASE 3 — Sync master and develop with upstream

```bash
# Sync master:
cd ~/Development/RMG/rmgdft-sandbox/
git fetch upstream
#git checkout master
git switch master     # use this instead of 'checkout'
git merge upstream/master
git push origin master

# Sync develop:
cd ~/Development/RMG/rmgdft-sandbox-develop/
git fetch upstream
git merge upstream/develop
git push origin develop
```

### PHASE 4 — Create fresh explore-new branch

```bash
cd ~/Development/RMG/rmgdft-sandbox-develop/
#git checkout -b explore-new
git git switch -c explore-new    # modern version -c=create
git push origin explore-new
git switch develop               # go back to "develop" branch

# Set up worktree:
cd ~/Development/RMG/
git -C rmgdft-sandbox worktree add rmgdft-sandbox-explore-new explore-new
```

---

## Reference Commands for Navigation

These are the key comparison commands for understanding what changed and where
annotations need to be rewritten for the new code.

### Compare your annotations against the pre-tag develop

*"What exactly did I add to explore vs the old code?"*
```bash
# All changes (manuscript + code):
git diff pre-TDDFT-annotation..explore --stat

# Only the annotated cpp files:
git diff pre-TDDFT-annotation..explore -- TDDFT/RMG_TDDFT/

# One specific file:
git diff pre-TDDFT-annotation..explore -- TDDFT/RMG_TDDFT/RmgTddft.cpp
```

### Compare pre-tag develop against new upstream (what refactoring did)

*"What did Emil/Wenchang change in the refactoring?"*
```bash
# Summary of all TDDFT changes:
git diff pre-TDDFT-annotation..upstream/develop --stat -- TDDFT/

# Full diff of TDDFT directory:
git diff pre-TDDFT-annotation..upstream/develop -- TDDFT/

# What replaced RmgTddft.cpp specifically:
git diff pre-TDDFT-annotation..upstream/develop -- TDDFT/RMG_TDDFT/
```

### View the old annotated file (Rosetta Stone)

*"Show me the pre-refactor annotated version of RmgTddft.cpp"*
```bash
git show explore:TDDFT/RMG_TDDFT/RmgTddft.cpp
```

### View the new upstream version of a file

```bash
git show upstream/develop:TDDFT/RMG_TDDFT/<filename>
```

### Compare current explore against current develop (after sync)

*"How far behind is explore from the new develop?"*
```bash
git diff develop..explore --stat
git log develop..explore --oneline
```

---

## Quick Reference: Tag and Branch Summary

| Reference | Commit points to | Use for |
|---|---|---|
| `pre-TDDFT-annotation` | `develop` just before May 2026 sync | Anchor for all comparisons |
| `explore` | Frozen — annotations + manuscript | Historical reference, Rosetta Stone |
| `develop` | Current upstream (post-sync) | Active development base |
| `explore-new` | Fresh off post-sync develop | Future exploration of refactored code |

---

## Next Development Tasks (as of May 2026)

After the sync is complete, the priority tasks are:

1. **Understand the refactoring** — Use Claude CLI to compare the annotated
   `explore` version of `RmgTddft.cpp` against whatever replaced it in
   upstream. The goal is to understand what changed structurally and whether
   the physics gaps documented in GAP-1 through GAP-8 are still present.

2. **Re-annotate the new code** — Once the refactored TDDFT files are
   understood, write new annotations in `explore-new` for the current code.

3. **Nonlocal PP forces for Ehrenfest** — The active development task in
   `develop-Ehrenfest`: add the nonlocal pseudopotential contribution to
   Ehrenfest forces. Reference: `Force/Force.cpp` (ground-state version),
   `TDDFT/RMG_TDDFT/CurrentNlpp.cpp` (the current operator analogue).

4. **Non-collinear spin RT-TDDFT** — Planned extension: two-component spinor
   wavefunctions throughout the TD loop. Not yet started.

---

*End of document. Last updated: May 2026.*




