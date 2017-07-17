# mergecheck
Tool for checking in-memory if a git merge would produce merge conflicts.

```
mergecheck usage:
  -h [ --help ]         Display this help message
  --repo arg            Path to the repository
  --our arg             Commit ref for the destination tree
  --their arg           Commit ref to merge into the "our" branch
  --remote-url arg      Add a new remote to the repository before merging.
                        (This is not an in-memory operation and changes the
                        repository on disk!)
  --remote-name arg     Name for the new  remote (e.g. 'upstream')
  -v [ --verbose ]      Print all conflicts. Otherwise, only the number of
                        conflicts is printed.
```

```
Example usage:

mergecheck -v --repo "/path/to/repo" --remote-url "http://example.com/other/repo.git" --remote-name "upstream"  --our "refs/remotes/upstream/master" --their "refs/remotes/origin/branch"
```
