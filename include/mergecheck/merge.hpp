#ifndef MERGECHECK_MERGE_HPP
#define MERGECHECK_MERGE_HPP

#include <git2.h>
#include <string>

size_t merge(git_repository *repo, std::string our_branch,
             std::string their_branch, bool print_conflicts, bool verbose);

#endif /* MERGECHECK_MERGE_HPP */

