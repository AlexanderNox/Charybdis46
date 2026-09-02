# Repository workflow

- Build and validate firmware only with the existing GitHub Actions workflow.
- Do not initialize a local west workspace or attempt local firmware builds.
- Push the intended commit to `Develop`, then monitor every CI matrix job and
  fix failures through the same CI workflow.
