# rmgdft-sandbox-explore-new

Branch: `explore-new` — dual-purpose sandbox for code exploration and early-stage prototyping. Successor to the now-frozen `explore` worktree.
Based on: `develop` after the May 2026 upstream sync (post-refactor; `RmgTddft.cpp` deleted, replaced by `rmg_tddft.cpp` and helpers).

---

## Purpose

This worktree serves two distinct modes of work, same as the old `explore`:

**Mode 1 — Code exploration (read-oriented)**
Safe space to trace call chains, add temporary print statements, insert
assertions, or instrument code to understand what RMG-DFT is actually doing
at runtime. Nothing here is expected to be production-ready. Commits are
optional and disposable.

Annotation of source files with physics block comments also lives in this
mode. The current focus is re-annotating the refactored TDDFT files, using
the frozen `explore` branch as a Rosetta Stone (see "Current focus" below).

**Mode 2 — Early-stage prototyping**
Scratchpad for new physics ideas (non-collinear spin RT-TDDFT, Ehrenfest
extensions) before they justify a dedicated feature branch. Code here may
be incomplete, physically incorrect, or purely illustrative of an approach.

**This worktree is never merged directly into `develop` or `master`.**
Finished work is either cherry-picked or re-implemented cleanly on a proper
feature branch off `develop`.

---

## Relationship to the `explore` branch

The `explore` worktree is **retired as an active session** — no new code work
happens there. But it contains three categories of content, only two of which
are purely historical:

1. **Retired session records** (`session_context*_explore_frozen.md`,
   `session_progress*_explore_frozen.md`) — historical only. Do not edit.
2. **Frozen annotated source files** (`TDDFT/RMG_TDDFT/RmgTddft.cpp` and
   its companions) — Rosetta Stone for the pre-refactor algorithm. The
   `RmgTddft.cpp` annotated version is the only documented record of the
   old code, since upstream deleted that file in the May 2026 refactor.
3. **Live canonical documentation** — the `Manuscript/` folder (paper2 on
   RT-TDDFT with NL-PP) and the `docs/` folder (`rmg_tddft_theory.tex`,
   `tddft_memory_tutorial.tex`, …). These are **not retired** — they remain
   the canonical physics-to-code mapping and are consulted from any active
   worktree. They are kept on `explore` as their permanent home.

Useful commands:

```bash
# View the old annotated source (Rosetta Stone, category 2):
git show explore:TDDFT/RMG_TDDFT/RmgTddft.cpp

# Read the live theory documentation (category 3):
ls ~/Development/RMG/rmgdft-sandbox-explore/Manuscript/
ls ~/Development/RMG/rmgdft-sandbox-explore/docs/

# Diff annotations vs. pre-sync develop:
git diff pre-TDDFT-annotation..explore -- TDDFT/RMG_TDDFT/

# Diff upstream refactoring vs. pre-sync develop:
git diff pre-TDDFT-annotation..upstream/develop -- TDDFT/RMG_TDDFT/
```

The `pre-TDDFT-annotation` tag is the anchor for the diff comparisons.

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

## Sync with `develop` before each session

```bash
cd ~/Development/RMG/rmgdft-sandbox-explore-new/
git fetch origin
git merge origin/develop
```

Conflicts are expected occasionally — resolve in favor of `develop` for any
file outside your active annotation or experiment.

---

## Current focus — annotation of refactored TDDFT files

Active annotation pass: walking the post-May-2026 refactored TDDFT files and
adding physics block comments, cross-referencing the frozen `explore` branch
where the pre-refactor `RmgTddft.cpp` lives as a Rosetta Stone.

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

Companion document: `TDDFT_refactor_mapping.tex` (committed in Session 1)
maps pre-refactor → post-refactor structure in detail.

---

## Most frequently explored code paths

These are the files most likely to be relevant for exploration or
prototyping in the RT-TDDFT / Ehrenfest / spin context.

### RT-TDDFT time propagation spine (post-refactor)

```
rmg_tddft.cpp
  └─► HmatrixUpdate.cpp          # rebuild H at each step
        └─► Magnus.cpp            # exponential propagator exp(-iHΔt)
              └─► GetNewRho_rmgtddft.cpp   # update electron density ρ(t)
```

### Key files

| File | Module | What to look for |
|---|---|---|
| `TDDFT/RMG_TDDFT/rmg_tddft.cpp` | TDDFT | Outer time loop — entry point for any TD extension |
| `TDDFT/RMG_TDDFT/HmatrixUpdate.cpp` | TDDFT | H rebuild — where nuclear position updates feed back in |
| `TDDFT/ELDYN/Magnus.cpp` | TDDFT | Propagator — key target for spinor / non-collinear work |
| `TDDFT/RMG_TDDFT/GetNewRho_rmgtddft.cpp` | TDDFT | Density update — source of Ehrenfest forces |
| `Force/Force.cpp` | Force | Ground-state forces — reference for nonlocal PP force term |
| `Force/CorrectForces.cpp` | Force | Force corrections |
| `RMG/Spin/` | Spin | Spin-polarized routines — reference for non-collinear extension |
| `Headers/prototypes_tddft.h` | Headers | All TDDFT function signatures in one place |

---

## Per-session context files

The worktree uses a three-tier separation of context, each file with a
different lifetime. Claude should respect this separation and not mix them.

| File | Lifetime | Contents |
|---|---|---|
| `CLAUDE.md` | **Durable** — changes rarely | Worktree identity, build, sync, exploration map, durable rules for Claude. This file. |
| `session_context.md` | **Per-session** — rewritten each session | Current task: goal, what's being explored or annotated, what's been understood about the code so far. |
| `session_progress.md` | **Per-session** — appended each session | Concrete file-level state: what was modified, what's committed, what's pending, next actions. |
| `session_context_explore_frozen.md` | Historical | Frozen snapshot from the old `explore` branch. Reference only — do not edit. |
| `session_progress_explore_frozen.md` | Historical | Frozen snapshot from the old `explore` branch. Reference only — do not edit. |

When starting a session, look for active `session_context.md` /
`session_progress.md`. If absent, create them using the skeletons below —
fill in the placeholders with the session's actual content immediately. Do
not commit blank templates.

### `session_context.md` skeleton

````markdown
# Session Context — <one-line topic>
**Date:** <YYYY-MM-DD>
**Branch:** `<branch>` (`<worktree-path>/`)
**Session goal:** <one-line goal>

---

## What is being explored / prototyped
<one short paragraph: which code path, which physics question, or which
implementation idea is the focus of this session>

## Files read and understood so far
| File | What it does | Key takeaway |
|---|---|---|
| `<path/file.cpp>` | <one line> | <one line> |

## Key physics or implementation insights reconstructed
- <bullet: formula, invariant, buffer reuse, gauge choice, etc.>

## How to verify this prototype / feature
- **Build:** see `build-notes-bigroo64.md` (machine) + CLAUDE.md Build section (worktree).
- **Test case(s):** <input file, test geometry, or unit-test path>
- **Expected output:** <observable, energy, force, current — and tolerance>
- **Regression baseline:** <reference run or commit to compare against>

## Open questions / gaps
- <bullet>

## Prior session references (optional)
List frozen records or live docs from other worktrees that may inform this
session. Consult only if relevant — findings do not auto-propagate.
- `session_context_explore_frozen.md` — <one-line summary of what's there>
- `~/Development/RMG/rmgdft-sandbox-explore/Manuscript/` — live RT-TDDFT manuscript
- `~/Development/RMG/rmgdft-sandbox-explore/docs/rmg_tddft_theory.tex` — live theory doc
- (none, if this is a clean-slate session)

## Next steps
- <bullet>
````

### `session_progress.md` skeleton

````markdown
# Session Progress — Concrete File State and Next Actions
**Date:** <YYYY-MM-DD>
**Branch:** `<branch>`

---

## Files Modified
### `<path/file.cpp>`
**What was added/changed:**
- <bullet>

**Git status:** <Modified — uncommitted | Committed: <sha> | Pushed>

---

## New Files Created
### `<path/file.ext>`
- <one line: what it is>
- **Status:** <Draft | Committed | Compiling cleanly>

---

## Build / test status
| Run | Date | Commit | Test case | Result | Notes |
|---|---|---|---|---|---|
| 1 | YYYY-MM-DD | <sha> | <input> | PASS/FAIL | <short note or log path> |

(Append a row per run; never delete rows. This is a running log of "does
this prototype still work?")

---

## Pending
- <bullet>

## Next actions
- <bullet>
````

### Retiring a worktree

When a worktree's session is finished and a new worktree is spun up for a
different exploration, retire the old worktree's session files instead of
deleting them:

1. Rename in place:
   - `session_context.md` → `session_context_<worktree-name>_frozen.md`
   - `session_progress.md` → `session_progress_<worktree-name>_frozen.md`
2. Prepend a freeze header to each file:
   ```
   # FROZEN RECORD — <worktree-name>, <date range or sync milestone>
   # Do not edit.
   ```
3. Commit and push. The retired worktree itself can remain as a worktree (for
   read-only reference) or be removed via `git worktree remove …` — its branch
   and frozen session files survive in the repo either way.

The next worktree's `session_context.md` can then list the new frozen file
under "Prior session references" if its content is relevant.

---

## Notes for Claude CLI sessions

### Durable rules (apply in every session)

- **Reconstruct physics formulas, not just C++ syntax.** Relate real-space
  grid quantities to their Gaussian basis set analogs wherever possible
  (this is Jacek's native idiom).
- **Don't assume this branch is clean.** It may contain leftover print
  statements, commented-out experiments, or partially implemented ideas
  from previous sessions. Always check `git diff origin/develop` before
  starting to understand what is already modified.
- **Mark prototype code distinctly:**
  ```cpp
  // EXPLORE-NEW: <brief description of what this is testing>
  // Not production-ready — lives in explore-new branch only
  ```
- **Graduating code:** when a prototype here is worth real development,
  create a new feature branch off `develop` (not off `explore-new`) and
  re-implement cleanly there.

### Current-pass rules (while annotation is the active focus)

- **Read `session_context.md` and `session_progress.md` first.** They record
  the current task and what has already been annotated. If they are absent,
  the session is a fresh one — create them per the convention above.
- **Use the Rosetta Stone.** Annotation of post-refactor files should
  cross-reference the frozen `explore` version of `RmgTddft.cpp` whenever
  the physics is unclear from the new code alone.

---

## Next development tasks

(from `README-Jacek-sync-annotation.md`)

1. **Complete annotation of refactored TDDFT files** — work through the
   "NOT STARTED" rows in the Current focus table. Priority: the propagator
   spine (`rmg_tddft.cpp` → `HmatrixUpdate.cpp` → `Magnus.cpp` →
   `GetNewRho_rmgtddft.cpp`).
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
