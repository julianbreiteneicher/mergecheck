#ifndef MERGECHECK_REMOTE_HPP
#define MERGECHECK_REMOTE_HPP

#include <git2.h>
#include <string>

void addRemote(git_repository *Repo, const std::string &RemoteUrl,
               const std::string &RemoteName, bool Verbose);

#endif /* MERGECHECK_REMOTE_HPP */
