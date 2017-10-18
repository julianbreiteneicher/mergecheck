# mergecheck
Tool for checking in-memory if a git merge/rebase would produce conflicts.

```
USAGE: mergecheck [command] [global options] [command-specific options]

Available commands:
  merge                 Join two development histories together
  rebase                Reapply commits on top of another base tip

Global options:
  --repo arg            Path to the repository
  --remote-url arg      Add a new remote to the repository.
                        (This is not an im-memory operation and changes the
                        repository!)
  --remote-name arg     Name for the new  remote (e.g. 'upstream')
  --print-conflicts     List all conflicts.
  -v [ --verbose ]      Be verbose.
  -h [ --help ]         Print this help text.

Options for 'merge' command:
  --our arg             Commit/Ref that reflects the destination tree
  --their arg           Commit/Ref to merge in to "our" commit

Options for 'rebase' command:
  --upstream arg        Upstream branch to compare against. Can be any valid
                        commit.
  --branch arg          Branch to rebase onto another branch. Can be any valid
                        commit.
  --onto arg            (Optional) If specified, the new commits will start at
                        this point. Can be any valid commit.If unspecified, the
                        starting point will be <upstream>.
                        conflicts is printed.
```

## Build instructions:
```
mkdir build && cd build
cmake ..
make
```

## Example usage:

### merge
```
mergecheck merge --repo "/path/to/repo" --remote-url "http://example.com/other/repo.git" --remote-name "upstream" --print-conflicts --our "refs/remotes/upstream/master" --their "refs/remotes/origin/branch"
```

### rebase
```
mergecheck rebase --repo "/path/to/repo" --remote-url "http://example.com/other/repo.git" --remote-name "upstream" --print-conflicts --upstream "refs/remotes/upstream/master" --branch "refs/remotes/origin/branch"
```
