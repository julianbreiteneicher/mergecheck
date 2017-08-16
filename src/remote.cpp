#include <iostream>

#include "mergecheck/remote.hpp"
#include "mergecheck/utils.hpp"

void addRemote(git_repository *Repo, const std::string &RemoteUrl,
               const std::string &RemoteName, bool Verbose) {
  int error;

  git_remote *Remote = nullptr;
  if (Verbose) {
    std::cout << "Checking if remote \'" << RemoteName << "\' already exists..."
              << std::endl;
  }
  error = git_remote_lookup(&Remote, Repo, RemoteName.c_str());
  if (error < 0) {
    if (Verbose) {
      std::cout << "Adding remote \'" << RemoteName << "\'..." << std::endl;
    }
    error = git_remote_create(&Remote, Repo, "upstream", RemoteUrl.c_str());
    checkError(error, "adding remote");
  } else {
    // remote already exists; check if url is the same
    auto Url = git_remote_url(Remote);
    if (RemoteUrl == std::string(Url)) {
      if (Verbose) {
        std::cout << "Remote \'" << RemoteName
                  << "\' already exists with the provided url. Continuing... "
                  << std::endl;
      }
    } else {
      std::cerr << "Error: Remote \'" << RemoteName
                << "\' already exists with the url \'" << Url
                << "\', which differs from the one specified by the user. "
                << "Please use a different name." << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  if (Verbose) {
    std::cout << "Fetching remote \'" << RemoteName << "\'..." << std::endl;
  }
  git_fetch_options FetchOpts{};
  error = git_fetch_init_options(&FetchOpts, GIT_FETCH_OPTIONS_VERSION);
  checkError(error, "git_fetch_options");
  error = git_remote_fetch(Remote, nullptr, &FetchOpts, nullptr);
  checkError(error, "fetching remote");

  git_remote_free(Remote);
}
