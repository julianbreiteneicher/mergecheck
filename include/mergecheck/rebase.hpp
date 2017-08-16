#ifndef MERGECHECK_REBASE_HPP
#define MERGECHECK_REBASE_HPP

#include <git2.h>
#include <string>

size_t rebase(git_repository *Repo, const std::string &UpstreamBranch,
              const std::string &Branch, bool PrintConflicts, bool Verbose);

size_t rebase(git_repository *Repo, const std::string &UpstreamBranch,
              const std::string &Branch, const std::string &OntoCommit,
              bool PrintConflicts, bool Verbose);

#endif /* MERGECHECK_REBASE_HPP */
