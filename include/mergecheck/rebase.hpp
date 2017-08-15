#ifndef MERGECHECK_REBASE_HPP
#define MERGECHECK_REBASE_HPP

#include <git2.h>
#include <string>

size_t rebase(git_repository *repo, std::string upstream_branch,
              std::string branch, bool print_conflicts, bool verbose);

size_t rebase(git_repository *repo, std::string upstream_branch,
              std::string branch, std::string onto_commit, bool print_conflicts,
              bool verbose);

#endif /* MERGECHECK_REBASE_HPP */

