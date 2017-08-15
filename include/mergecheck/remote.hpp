#ifndef MERGECHECK_REMOTE_HPP
#define MERGECHECK_REMOTE_HPP

#include <git2.h>
#include <string>

void addRemote(git_repository *repo, std::string remote_url,
               std::string remote_name, bool verbose);

#endif /* MERGECHECK_REMOTE_HPP */
