#include <iostream>

#include "mergecheck/remote.hpp"
#include "mergecheck/utils.hpp"

void addRemote(git_repository *repo, std::string remote_url,
               std::string remote_name, bool verbose) {
  int error;

  git_remote *remote = nullptr;
  if (verbose) {
    std::cout << "Checking if remote \'" << remote_name
              << "\' already exists..." << std::endl;
  }
  error = git_remote_lookup(&remote, repo, remote_name.c_str());
  if (error < 0) {
    if (verbose) {
      std::cout << "Adding remote \'" << remote_name << "\'..." << std::endl;
    }
    error = git_remote_create(&remote, repo, "upstream", remote_url.c_str());
    check_error(error, "adding remote");
  } else {
    // remote already exists; check if url is the same
    auto url = git_remote_url(remote);
    if ((remote_url.compare(std::string(url)) == 0)) {
      if (verbose) {
        std::cout << "Remote \'" << remote_name
                  << "\' already exists with the provided url. Continuing... "
                  << std::endl;
      }
    } else {
      std::cerr << "Error: Remote \'" << remote_name
                << "\' already exists with the url \'" << url
                << "\', which differs from the one specified by the user. "
                << "Please use a different name." << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  if (verbose) {
    std::cout << "Fetching remote \'" << remote_name << "\'..." << std::endl;
  }
  git_fetch_options fetch_opts;
  error = git_fetch_init_options(&fetch_opts, GIT_FETCH_OPTIONS_VERSION);
  check_error(error, "git_fetch_options");
  error = git_remote_fetch(remote, NULL, &fetch_opts, NULL);
  check_error(error, "fetching remote");

  git_remote_free(remote);
}

