#ifndef MERGECHECK_MERGE_HPP
#define MERGECHECK_MERGE_HPP

#include <git2.h>
#include <string>

size_t merge(git_repository *Repo, const std::string &OurBranch,
             const std::string &TheirBranch, bool PrintConflicts, bool Verbose);

#endif /* MERGECHECK_MERGE_HPP */
