
# The Optimal Restart Prompt
 * Here is the exact prompt template I recommend you use at the start of every new session on this project:
```
  I am continuing work on the RMG periodic RT-TDDFT manuscript (paper 2, 
  follow-up to Jakowski et al. JCTC 2025). Please fetch and read these 
  files from GitHub repo jjakowski/rmgdft-sandbox, branch explore:

  1. Manuscript/STATUS.md
  2. Manuscript/CONTEXT.md  
  3. Manuscript/DECISIONS.md
  4. Manuscript/notation.tex
  5. Manuscript/paper2/[active section file]

  After reading them, summarize in 3-4 sentences what the project is, 
  where we are, and what the immediate next task is. Then we will proceed.
```


* That is it. With those five files fetched in the first exchange, a new session will be fully oriented. The summary it produces back to you serves as a confirmation that it understood correctly — if it gets something wrong, you correct it before any work begins.

For a session focused on a specific subsection, add one line at the end:

```
  Today we are working specifically on [subsection name]. 
  The goal for this session is [specific deliverable].
```

For example:

```
  Today we are working specifically on the current operator derivation 
  (sec_theory.tex, subsection 2d). The goal is to derive the paramagnetic 
  current in density matrix form and verify the sign/i-factor convention 
  against VecPHmatrix() in the code.
```

---

### Restart prompt template
Paste this at the start of a new session:

```
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
```

