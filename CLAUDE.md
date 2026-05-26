# rmgdft-sandbox-explore-new

Branch: `explore-new` — active annotation and exploration of the refactored TDDFT code.
Based on: `develop` after the May 2026 upstream sync (post-refactor; `RmgTddft.cpp` deleted, replaced by `rmg_tddft.cpp` and helpers).

---

## Purpose

This worktree is the successor to the (now frozen) `explore` branch. Its two
modes of work are the same as before:

**Mode 1 — Annotation of refactored TDDFT code**
Add physics explanations to the post-refactor source files as block comments,
using the frozen `explore` branch as a Rosetta Stone for the deleted
pre-refactor `RmgTddft.cpp`. This is the dominant activity in early sessions.

**Mode 2 — Early-stage prototyping**
Scratchpad for new physics ideas (non-collinear spin RT-TDDFT, Ehrenfest
extensions) before they justify a dedicated feature branch.

**This worktree is never merged directly into `develop` or `master`.**
When a prototype graduates, it goes onto a fresh feature branch off `develop`.

---

## Relationship to the frozen `explore` branch

The `explore` branch is **frozen** — it contains the manuscript and the
annotated pre-refactor `RmgTddft.cpp` (which no longer exists in `develop`).
Treat it as a read-only historical reference.

```bash
# View the old annotated deleted file (Rosetta Stone):
git show explore:TDDFT/RMG_TDDFT/RmgTddft.cpp

# What annotations were added on explore (vs. the pre-sync develop):
git diff pre-TDDFT-annotation..explore -- TDDFT/RMG_TDDFT/

# What the upstream refactoring did (vs. the pre-sync develop):
git diff pre-TDDFT-annotation..upstream/develop -- TDDFT/RMG_TDDFT/
```

The `pre-TDDFT-annotation` tag is the anchor for all these comparisons.

---

## Build (Bigroo64)

```bash
source ~/source.miniconda
conda activate rmg-dev
cd ~/Development/RMG/rmgdft-sandbox-explore-new/
mkdir -p build_explore_new && cd build_explore_new/

export CC="$CONDA_PREFIX/bin/mpicc"
export CXX="$CONDA_PREFIX/bin/mpicxx"
export FC="$CONDA_PREFIX/bin/mpif90"

cmake .. -DCMAKE_PREFIX_PATH="$CONDA_PREFIX" \
    -DHDF5_ROOT="$CONDA_PREFIX" > cmake.log 2>&1
make VERBOSE=1 -j 4 >& make.log
```

**Rebuild a single file after edits:**
```bash
cd build_explore_new/
touch ../TDDFT/RMG_TDDFT/rmg_tddft.cpp && make -j 4
```

---

## Refactored TDDFT file map (post-May-2026 sync)

| File | Refactoring status | Annotation status |
|---|---|---|
| `TDDFT/RMG_TDDFT/rmg_tddft.cpp` | MODIFIED — main replacement for deleted `RmgTddft.cpp` | DONE — Session 1 (Task B) |
| `TDDFT/RMG_TDDFT/rmg_tddft_energy.cpp` | MODIFIED | NOT STARTED |
| `TDDFT/RMG_TDDFT/rmg_rotate_sint.cpp` | NEW | NOT STARTED |
| `TDDFT/ELDYN/commutp2.cpp` | MODIFIED | NOT STARTED |
| `TDDFT/RMG_TDDFT/CurrentNlpp.cpp` | UNCHANGED | DONE — Session 1 (Task A) |
| `TDDFT/RMG_TDDFT/VecPmatrix.cpp` | UNCHANGED | DONE — Session 1 (Task A) |
| `TDDFT/RMG_TDDFT/HmatrixUpdate.cpp` | CHECK STATUS | NOT STARTED |
| `TDDFT/ELDYN/Magnus.cpp` | CHECK STATUS | NOT STARTED |
| `TDDFT/RMG_TDDFT/GetNewRho_rmgtddft.cpp` | CHECK STATUS | NOT STARTED |

Companion document: `TDDFT_refactor_mapping.tex` (committed in Session 1) maps
pre-refactor → post-refactor structure in detail.

---

## Sync with `develop` before each session

```bash
cd ~/Development/RMG/rmgdft-sandbox-explore-new/
git fetch origin
git merge origin/develop
```

Conflicts are expected occasionally — resolve in favor of `develop` for any
file outside your active annotation or experiment.

---

## Notes for Claude CLI sessions

- **Read `STATUS.md` first — always, before anything else.** It records the
  state of the current annotation pass and which file is in flight.
- **Reconstruct physics formulas, not just C++ syntax.** Relate real-space
  grid quantities to their Gaussian basis set analogs wherever possible
  (this is Jacek's native idiom).
- **Use the Rosetta Stone.** Annotation of post-refactor files should
  cross-reference the frozen `explore` version of `RmgTddft.cpp` whenever
  the physics is unclear from the new code alone.
- **Mark prototype code distinctly:**
  ```cpp
  // EXPLORE-NEW: <brief description of what this is testing>
  // Not production-ready — lives in explore-new branch only
  ```
- **Graduating code:** when a prototype here is worth real development,
  create a new feature branch off `develop` (not off `explore-new`) and
  re-implement cleanly there.

---

## Next development tasks

(from `README-Jacek-sync-annotation.md`)

1. **Complete annotation of refactored TDDFT files** — work through the
   "NOT STARTED" rows in the file map above. Priority: the propagator spine
   (`rmg_tddft.cpp` → `HmatrixUpdate.cpp` → `Magnus.cpp` → `GetNewRho_rmgtddft.cpp`).
2. **Nonlocal PP forces for Ehrenfest** — active task on `develop-Ehrenfest`.
   References: `Force/Force.cpp` (ground-state), `TDDFT/RMG_TDDFT/CurrentNlpp.cpp`
   (current-operator analogue).
3. **Non-collinear spin RT-TDDFT** — planned, not yet started. Two-component
   spinor wavefunctions throughout the TD loop.

---

## Current experiments

*(Update this section at the start of each working session.)*

| Label | Description | Status | Notes |
|---|---|---|---|
| — | — | — | No active experiments yet |
