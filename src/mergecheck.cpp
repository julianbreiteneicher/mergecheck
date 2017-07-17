#include <iostream>
#include <sstream>
#include <string>

#include <boost/program_options.hpp>
#include <git2.h>

#include "mergecheck/string_utils.hpp"
#include "mergecheck/utils.hpp"

namespace po = boost::program_options;

int main(int argc, char *argv[]) {
  // strings for command line args
  std::string our_branch, their_branch;
  const char *our_branch_short, *their_branch_short;
  std::string remote_url;
  std::string remote_name;
  std::string repo_path;
  bool verbose = false;
  bool add_remote = false;

  // read command line options
  po::options_description description("mergecheck usage");
  description.add_options()("help,h", "Display this help message")(
      "repo", po::value<std::string>(&repo_path)->required(),
      "Path to the repository")("our",
                                po::value<std::string>(&our_branch)->required(),
                                "Commit ref for the destination tree")(
      "their", po::value<std::string>(&their_branch)->required(),
      "Commit ref to merge into the \"our\" branch")(
      "remote-url", po::value<std::string>(&remote_url),
      "Add a new remote to the repository before merging.\n(This is not an "
      "in-memory operation and changes the repository on disk!)")(
      "remote-name", po::value<std::string>(&remote_name),
      "Name for the new  remote (e.g. \'upstream\')")(
      "verbose,v", "Print all conflicts. Otherwise, "
                   "only the number of conflicts is "
                   "printed.");

  po::positional_options_description p;
  po::variables_map vm;

  try {
    po::store(po::command_line_parser(argc, argv)
                  .options(description)
                  .positional(p)
                  .run(),
              vm);
    if (vm.size() == 0 || vm.count("help")) {
      std::cout << description;
      return EXIT_SUCCESS;
    }
    bool has_remote_url_param = vm.count("remote-url") > 0;
    bool has_remote_name_param = vm.count("remote-name") > 0;
    if (has_remote_url_param && has_remote_name_param) {
      add_remote = true;
    } else if (has_remote_url_param ^ has_remote_name_param) {
      std::cerr << "Error: Options \"remote-url\" and \"remote-name\" have to "
                   "be provided together.";
      return EXIT_FAILURE;
    }

    po::notify(vm);
  } catch (const std::exception &ex) {
    std::cerr << "\n" << ex.what() << "\n\n";
    return EXIT_FAILURE;
  }

  verbose = vm.count("verbose") > 0;

  // try to do some very simple normalization of the command line arguments,
  // e.g. removing whitespaces and trailing slashes (for urls)
  trim(our_branch);
  trim(their_branch);
  trim(repo_path);
  trim(remote_name);
  trim(remote_url);
  while (remote_url.back() == '/') {
    remote_url.pop_back();
  }

  int error;
  git_repository *repo = nullptr;

  git_libgit2_init();

  std::cout << "Opening repository..." << std::endl;
  error = git_repository_open(&repo, repo_path.c_str());
  check_error(error, "opening repository");

  //  check if specified remote already exists
  if (add_remote) {
    git_remote *remote = nullptr;
    std::cout << "Checking if remote \'" << remote_name
              << "\' already exists..." << std::endl;
    error = git_remote_lookup(&remote, repo, remote_name.c_str());
    if (error < 0) {
      std::cout << "Adding remote \'" << remote_name << "\'..." << std::endl;
      error = git_remote_create(&remote, repo, "upstream", remote_url.c_str());
      check_error(error, "adding remote");
    } else {
      // remote already exists; check if url is the same
      auto url = git_remote_url(remote);
      if (remote_url.compare(std::string(url)) == 0) {
        std::cout << "Remote \'" << remote_name
                  << "\' already exists with the provided url. Continuing..."
                  << std::endl;
      } else {
        std::cerr << "Error: Remote \'" << remote_name
                  << "\' already exists with the url \'" << url
                  << "\', which differs from the one specified by the user. "
                  << "Please use a different name." << std::endl;
        return EXIT_FAILURE;
      }
    }

    std::cout << "Fetching remote \'" << remote_name << "\'..." << std::endl;
    git_fetch_options fetch_opts;
    error = git_fetch_init_options(&fetch_opts, GIT_FETCH_OPTIONS_VERSION);
    check_error(error, "git_fetch_options");
    error = git_remote_fetch(remote, NULL, &fetch_opts, NULL);
    check_error(error, "fetching remote");

    git_remote_free(remote);
  }

  // start the merge

  git_reference *our_ref, *their_ref;
  git_annotated_commit *our_head, *their_head;
  git_commit *ours, *theirs;
  const git_index_entry *ancestor, *our, *their;

  error = git_reference_lookup(&our_ref, repo, our_branch.c_str());
  check_error(error, "git_remote_lookup");
  error = git_reference_lookup(&their_ref, repo, their_branch.c_str());
  check_error(error, "git_remote_lookup");

  // get short name for nicer program output
  error = git_branch_name(&our_branch_short, our_ref);
  check_error(error, "git_branch_name");
  error = git_branch_name(&their_branch_short, their_ref);
  check_error(error, "git_branch_name");

  error = git_annotated_commit_from_ref(&our_head, repo, our_ref);
  check_error(error, "git_annotated_commit");
  error = git_annotated_commit_from_ref(&their_head, repo, their_ref);
  check_error(error, "git_annotated_commit");

  error = git_commit_lookup(&ours, repo, git_annotated_commit_id(our_head));
  check_error(error, "git_commit_lookup");
  error = git_commit_lookup(&theirs, repo, git_annotated_commit_id(their_head));
  check_error(error, "git_commit_lookup");

  std::cout << "Attempting to merge..." << std::endl;

  git_index *merge_index;
  git_merge_options merge_opts;
  error = git_merge_init_options(&merge_opts, GIT_MERGE_OPTIONS_VERSION);
  check_error(error, "git_merge_init_options");

  error = git_merge_commits(&merge_index, repo, ours, theirs, &merge_opts);
  check_error(error, "git_merge_commits");

  // get conflicts
  git_index_conflict_iterator *conflict_it;
  git_index_conflict_iterator_new(&conflict_it, merge_index);
  check_error(error, "iterator empty");

  size_t conflicts = 0;
  std::stringstream ss;
  while ((error = git_index_conflict_next(&ancestor, &our, &their,
                                          conflict_it)) == 0) {
    conflicts++;

    if (verbose) {
      if (!our) {
        ss << "CONFLICT (modify/delete): " << their->path
           << " deleted in HEAD and modified in " << their_branch_short
           << ".\n";
      } else if (!their) {
        ss << "CONFLICT (modify/delete): " << our->path << " deleted in "
           << our_branch_short << " and modified in HEAD.\n";
      } else {
        std::string reason = "content";
        // "normal conflict"
        if (!ancestor) {
          reason = "add/add";
        }
        ss << "CONFLICT (" << reason << "): Merge conflict in " << our->path
           << "\n";
      }
      std::cout << ss.str();

      // empty the stringstream
      ss.str(std::string());
      ss.clear();
    }
  }
  git_index_conflict_iterator_free(conflict_it);

  std::cout << "Finished merging.\n";
  if (conflicts > 0) {
    std::cout << "Found " << conflicts << " conflicts in total." << std::endl;
  } else {
    std::cout << "Good news, everyone! Branches can be merged automatically "
                 "without conflicts."
              << std::endl;
  }

  if (error != GIT_ITEROVER) {
    std::cerr << "An error occured during rebase!\n";
  }

  // clean up merge stuff...
  git_index_free(merge_index);
  git_commit_free(ours);
  git_commit_free(theirs);
  git_annotated_commit_free(our_head);
  git_annotated_commit_free(their_head);
  git_reference_free(our_ref);
  git_reference_free(their_ref);

  ///// end of merge

  // clean up...
  git_repository_free(repo);
  git_libgit2_shutdown();

  if (conflicts > 0) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
