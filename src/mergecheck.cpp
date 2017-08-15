#include <iostream>
#include <sstream>
#include <string>

#include <boost/program_options.hpp>
#include <git2.h>

#include "mergecheck/merge.hpp"
#include "mergecheck/rebase.hpp"
#include "mergecheck/remote.hpp"
#include "mergecheck/string_utils.hpp"
#include "mergecheck/utils.hpp"

namespace po = boost::program_options;

int main(int argc, char *argv[]) {
  // global cmd-line arguments
  std::string repo_path;
  std::string remote_url;
  std::string remote_name;
  bool verbose = false;
  bool print_conflicts = false;
  bool add_remote = false;

  // cmd-line arguments for 'merge' subcommand
  std::string merge_our_branch, merge_their_branch;

  // cmd-line arguments for 'rebase' subcommand
  std::string rebase_upstream_branch, rebase_branch;
  std::string rebase_onto_commit;

  /* clang-format off */
  po::options_description global("Global options");
  global.add_options()
    ("repo", po::value<std::string>(&repo_path)->required(),
       "Path to the repository")
    ("remote-url", po::value<std::string>(&remote_url),
       "Add a new remote to the repository.\n(This is not an im-memory "
       "operation and changes the repository!)")
    ("remote-name", po::value<std::string>(&remote_name),
       "Name for the new  remote (e.g. \'upstream\')")
    ("print-conflicts", "List all conflicts.")
    ("verbose,v", "Be verbose.")("help,h", "Print this help text.")
  ;

  po::options_description global_pos_dummy("Global positional options");
  global_pos_dummy.add_options()
    ("command", po::value<std::string>(), "command to execute")
    ("subargs", po::value<std::vector<std::string>>(), "Arguments for command")
  ;

  po::options_description all("Allowed options");
  all.add(global).add(global_pos_dummy);

  po::positional_options_description pos;
  pos.add("command", 1).add("subargs", -1);

  po::options_description merge_desc("Options for \'merge\' command");
  merge_desc.add_options()
    ("our", po::value<std::string>(&merge_our_branch)->required(),
       "Commit/Ref that reflects the destination tree")
    ("their", po::value<std::string>(&merge_their_branch)->required(),
       "Commit/Ref to merge in to \"our\" commit")
  ;

  po::options_description rebase_desc("Options for \'rebase\' command");
  rebase_desc.add_options()
    ("upstream", po::value<std::string>(&rebase_upstream_branch)->required(),
       "Upstream branch to compare against. Can be any valid commit.")
    ("branch", po::value<std::string>(&rebase_branch)->required(),
       "Branch to rebase onto another branch. Can be any valid commit.")
    ("onto", po::value<std::string>(&rebase_onto_commit),
       "(Optional) If specified, the new commits will start at this point. "
       "Can be any valid commit."
       "If unspecified, the starting point will be <upstream>.")
  ;
  /* clang-format on */

  po::variables_map vm;
  po::parsed_options parsed = po::command_line_parser(argc, argv)
                                  .options(all)
                                  .positional(pos)
                                  .allow_unregistered()
                                  .run();
  po::store(parsed, vm);

  if (vm.size() == 0 || vm.count("help") || vm.count("command") != 1) {
    std::cout << "USAGE: mergecheck [command] [global options] "
                 "[command-specific options]\n\n";
    std::cout << "Available commands:\n"
              << "  merge\t\t\tJoin two development histories together\n"
              << "  rebase\t\tReapply commits on top of another base tip\n"
              << "\n";
    std::cout << global << "\n";
    std::cout << merge_desc << "\n";
    std::cout << rebase_desc;
    return EXIT_SUCCESS;
  }

  verbose = vm.count("verbose") > 0;
  print_conflicts = vm.count("print-conflicts") > 0;
  std::string cmd = vm["command"].as<std::string>();

  bool has_remote_url_param = vm.count("remote-url") > 0;
  bool has_remote_name_param = vm.count("remote-name") > 0;
  if (has_remote_url_param && has_remote_name_param) {
    add_remote = true;
  } else if (has_remote_url_param ^ has_remote_name_param) {
    std::cerr << "Error: Options \"remote-url\" and \"remote-name\" have to "
                 "be provided together.";
    return EXIT_FAILURE;
  }

  // try to do some very simple normalization of the command line arguments,
  // e.g. removing whitespaces and trailing slashes (for urls)
  trim(repo_path);
  trim(remote_name);
  trim(remote_url);
  while (remote_url.back() == '/') {
    remote_url.pop_back();
  }

  // parse command-specifig cmd args

  // Collect all the unrecognized options from the first pass. This will
  // include the (positional) command name, so we need to erase that.
  std::vector<std::string> opts =
      po::collect_unrecognized(parsed.options, po::include_positional);
  opts.erase(opts.begin());

  if (cmd == "merge") {
    try {
      po::store(po::command_line_parser(opts).options(merge_desc).run(), vm);
      po::notify(vm);
    } catch (const std::exception &ex) {
      std::cerr << "\n" << ex.what() << "\n\n";
      return EXIT_FAILURE;
    }
    trim(merge_our_branch);
    trim(merge_their_branch);
  } else if (cmd == "rebase") {
    try {
      po::store(po::command_line_parser(opts).options(rebase_desc).run(), vm);
      po::notify(vm);
    } catch (const std::exception &ex) {
      std::cerr << "\n" << ex.what() << "\n\n";
      return EXIT_FAILURE;
    }
    trim(merge_our_branch);
    trim(merge_their_branch);
  } else {
    std::cerr << "Error! No legal command was specified! Use \"--help\" for "
                 "details.\n";
    return EXIT_FAILURE;
  }

  // done with command line parsing

  int error;
  git_repository *repo = nullptr;
  git_libgit2_init();

  if (verbose) {
    std::cout << "Opening repository..." << std::endl;
  }
  error = git_repository_open(&repo, repo_path.c_str());
  check_error(error, "opening repository");

  if (add_remote) {
    addRemote(repo, remote_url, remote_name, verbose);
  }

  size_t conflicts = 0;
  if (cmd == "merge") {
    conflicts = merge(repo, merge_our_branch, merge_their_branch,
                      print_conflicts, verbose);
  } else if (cmd == "rebase") {
    if (vm.count("onto") == 1) {
      conflicts = rebase(repo, rebase_upstream_branch, rebase_branch,
                         rebase_onto_commit, print_conflicts, verbose);
    } else {
      conflicts = rebase(repo, rebase_upstream_branch, rebase_branch,
                         print_conflicts, verbose);
    }
  }

  // clean up...
  git_repository_free(repo);
  git_libgit2_shutdown();

  if (conflicts > 0) {
    std::cout << "Found " << conflicts << " conflicts in total." << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Good news, everyone! Branches can be merged automatically "
               "without conflicts."
            << std::endl;
  return EXIT_SUCCESS;
}

