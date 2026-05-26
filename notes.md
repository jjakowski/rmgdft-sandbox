# Starting a new Claude session in this worktree

*Audience: me (Jacek). What to do when sitting down to work in*
*`rmgdft-sandbox-explore-new/`.*

---

## 1. Sync the worktree

```bash
cd ~/Development/RMG/rmgdft-sandbox-explore-new
git fetch origin
git merge origin/develop
```

## 2. Launch Claude

```bash
claude
```

Claude automatically loads the three CLAUDE.md files:

- `/home/j2c/Development/RMG/CLAUDE.md` (root — project context)
- `~/Development/RMG/rmgdft-sandbox-explore-new/CLAUDE.md` (this worktree)
- `~/Development/RMG/rmgdft-sandbox-develop/CLAUDE.md` (RT-TDDFT file map)

It does **not** automatically load `session_context.md` /
`session_progress.md`. Your opening prompt nudges it to.

## 3. Paste an opening prompt

Fill in the three fields and paste as the first message:

> I'm starting a new session.
>
> **Topic:** \<one-line description of what you're exploring or prototyping\>
> **Goal:** \<what you want to come away with at the end\>
> **Related to:** \<broader work area: non-collinear spin RT-TDDFT, Ehrenfest, etc.\>
>
> Read this worktree's `CLAUDE.md`, then handle `session_context.md` and
> `session_progress.md` per the convention described there. If creating
> them, use the Topic/Goal/Related above to fill in the corresponding
> fields in `session_context.md`.

### Worked example

```text
I'm starting a new session.

Topic: understanding how the current density J(r,t) is implemented on
       the real-space grid
Goal:  build a mental model of the implementation before extending
       J(r,t) to non-collinear spinors
Related to: non-collinear spin RT-TDDFT

Read this worktree's CLAUDE.md, then handle session_context.md and
session_progress.md per the convention described there. If creating
them, use the Topic/Goal/Related above to fill in the corresponding
fields in session_context.md.
```

### A few more example Topic/Goal pairs

| Topic | Goal | Related to |
|---|---|---|
| annotate `Magnus.cpp` — BCH expansion → physics | finish the annotation pass started in `rmg_tddft.cpp` | TDDFT refactor annotation |
| prototype non-collinear spinor wavefunctions in the TD loop | scratch implementation of the 2-component propagator | non-collinear spin RT-TDDFT |
| trace where nuclear positions feed into `HmatrixUpdate.cpp` | identify the hook point for Ehrenfest force feedback | Ehrenfest dynamics |

## 4. While working

- Let Claude update `session_progress.md` as files are touched, built,
  tested, or committed.
- Commit experimental code as you go so nothing is lost:
  ```bash
  git add <files>
  git commit -m "explore: <what you tried>"
  git push
  ```

## 5. Ending the session — three options

**(a) Stopping for the day, will continue here later**
Make sure `session_progress.md` is up to date. Commit + push. Done.

**(b) Exploration produced something worth real development**
Create a new feature branch off `develop` (not off `explore-new`) and
re-implement cleanly there. The exploration record stays in this worktree's
session files for reference.

**(c) This worktree's exploration is complete; moving to a new topic**
Retire the worktree per CLAUDE.md's *"Retiring a worktree"* section: rename
the session files with the `_explore_new_frozen` suffix, prepend the freeze
header, commit, push. Then create a fresh worktree for the next exploration.

---

## What NOT to do

- **Do not edit `CLAUDE.md`** to record today's topic. `CLAUDE.md` is
  durable; today's topic lives in `session_context.md`.
- **Do not skip the opening prompt** — without it, Claude won't read the
  session files (it doesn't auto-load them).
