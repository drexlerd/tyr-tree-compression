1. Read and continue keeping track of recent changes in LOG.md
2. Continue the iteration on reducing header dependencies.
We do not want to split every class into a single file since this would cause a massive blowup in the number of files.
Hence, make reasonable split, where it can result in lower compile resource usage (time, memory).
