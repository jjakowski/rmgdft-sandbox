# rmgdft-sandbox-explore

Branch: `explore` — dual-purpose sandbox for code exploration and early-stage prototyping.
Based on: `origin/develop` (branched off `develop`; periodically rebased or merged to stay current).

---

## Purpose

This worktree serves two distinct modes of work:

**Mode 1 — Code exploration (read-oriented)**
Safe space to trace call chains, add temporary print statements, insert
assertions, or instrument code to understand what RMG-DFT is actually doing
at runtime. Nothing here is expected to be production-ready. Commits are
optional and disposable.

**Mode 2 — Early-stage prototyping**
Scratchpad for new physics ideas before they are ready to justify a dedicated
feature branch (like `develop-Ehrenfest` was at its inception). Code here may
be incomplete, physically incorrect, or purely illustrative of an approach.

In both modes: **this worktree is never merged directly into `develop` or
`master`**. Finished work is either cherry-picked or re-implemented cleanly
on a proper feature branch.

---

## Build

```bash
source ~/source.miniconda
conda activate rmg-dev
cd ~/Development/RMG/rmgdft-sandbox-explore/build_explore/
export CC="$CONDA_PREFIX/bin/mpicc"
export CXX="$CONDA_PREFIX/bin/mpicxx"
export FC="$CONDA_PREFIX/bin/mpif90"
cmake .. -DCMAKE_PREFIX_PATH="$CONDA_PREFIX" -DHDF5_ROOT="$CONDA_PREFIX" > cmake.log 2>&1
make VERBOSE=1 -j 4 >& make.log
```

**Rebuild a single file after instrumentation or edits:**
```bash
touch ../TDDFT/RMG_TDDFT/RmgTddft.cpp && make -j 4
```

---

## Most Frequently Explored Code Paths

These are the files most likely to be relevant for exploration or
prototyping in the RT-TDDFT / Ehrenfest / spin context.

### RT-TDDFT time propagation spine
The call chain flows in this order:

```
RmgTddft.cpp
  └─► HmatrixUpdate.cpp      # rebuild H at each step
        └─► Magnus.cpp        # exponential propagator exp(-iHΔt)
              └─► GetNewRho_rmgtddft.cpp   # update electron density ρ(t)
```

### Key files table

| File | Module | What to look for |
|------|--------|-----------------|
| `TDDFT/RMG_TDDFT/RmgTddft.cpp` | TDDFT | Outer time loop — entry point for any TD extension |
| `TDDFT/RMG_TDDFT/HmatrixUpdate.cpp` | TDDFT | H rebuild — where nuclear position updates feed back in |
| `TDDFT/ELDYN/Magnus.cpp` | TDDFT | Propagator — key target for spinor / non-collinear work |
| `TDDFT/RMG_TDDFT/GetNewRho_rmgtddft.cpp` | TDDFT | Density update — source of Ehrenfest forces |
| `Force/Force.cpp` | Force | Ground-state forces — reference for nonlocal PP force term |
| `Force/CorrectForces.cpp` | Force | Force corrections |
| `RMG/Spin/` | Spin | Spin-polarized routines — reference for non-collinear extension |
| `Headers/prototypes_tddft.h` | Headers | All TDDFT function signatures in one place |

---

## Sync with `develop`

Before starting a new exploration or prototype, pull in the latest `develop`
so the explore branch does not drift too far behind:

```bash
# From rmgdft-sandbox-explore/
git fetch origin
git merge origin/develop        # or: git rebase origin/develop
```

Conflicts are expected occasionally — resolve in favor of `develop` for any
file outside your active experiment.

---

## Notes for Claude when working in this worktree

- **Exploration mode:** When tracing a call chain, always reconstruct the
  physics formula being implemented — do not just describe the C++ syntax.
  Relate grid quantities to their Gaussian basis set analogs wherever possible.

- **Prototyping mode:** When drafting new code here, mark all prototype
  functions and blocks with a comment header:
  ```cpp
  // EXPLORE: <brief description of what this is testing>
  // Not production-ready — lives in explore branch only
  ```
  This makes it unambiguous which code is experimental when diffing against
  `develop`.

- **Do not assume this branch is clean.** It may contain leftover print
  statements, commented-out experiments, or partially implemented ideas from
  previous sessions. Always check `git diff origin/develop` before starting
  to understand what is already modified.

- **Graduating code to a feature branch:** When a prototype in `explore`
  reaches the point of being worth proper development, the workflow is:
  1. Create a new feature branch off `develop` (not off `explore`)
  2. Re-implement the prototype cleanly there
  3. `explore` retains the messy original for reference

---

## Current Experiments

*(Update this section at the start of each working session.)*

| Label | Description | Status | Notes |
|-------|-------------|--------|-------|
| — | — | — | No active experiments yet |
