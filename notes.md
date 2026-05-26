# Typical 'rmgdft-sandbox-explore' session — step by step

  1. Update explore with latest develop (so you start from current code)
  cd ~/Development/RMG/rmgdft-sandbox-explore
  git merge develop

  2. Update the CLAUDE.md — fill in what you're exploring today:
  ## Current exploration
  **Topic:** [e.g., "how is the current density J(r,t) computed on the grid"]
  **Goal:** [e.g., "understand before extending to non-collinear spin"]
  **Related to:** non-collinear spin RT-TDDFT

  3. Start Claude from this folder
  claude
  Claude will automatically load:
  - /home/j2c/Development/RMG/CLAUDE.md (root — who you are, project goals)
  - /home/j2c/Development/RMG/rmgdft-sandbox-explore/CLAUDE.md (what you're exploring today)
  - /home/j2c/Development/RMG/rmgdft-sandbox-develop/CLAUDE.md (RT-TDDFT file map)

  4. Ask Claude your questions — for example:
  - "Show me how CurrentOperator.cpp implements J(r,t) and what formula it corresponds to"
  - "I want to prototype X — which files should I modify?"

  5. If you write experimental code — commit it so you don't lose it:
  git add <files>
  git commit -m "explore: [what you tried]"
  git push

  6. End of session — two outcomes:
  - Idea was useful → open a PR or merge into develop-Ehrenfest
  - Just exploration → leave it on explore, notes stay in CLAUDE.md for next time

