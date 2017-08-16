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
  std::string RepoPath;
  std::string RemoteUrl;
  std::string RemoteName;
  bool Verbose = false;
  bool PrintConflicts = false;
  bool AddRemote = false;

  // cmd-line arguments for 'merge' subcommand
  std::string MergeOurBranch, MergeTheirBranch;

  // cmd-line arguments for 'rebase' subcommand
  std::string RebaseUpstreamBranch, RebaseBranch;
  std::string RebaseOntoCommit;

  /* clang-format off */
  po::options_description Global("Global options");
  Global.add_options()
    ("repo", po::value<std::string>(&RepoPath)->required(),
       "Path to the repository")
    ("remote-url", po::value<std::string>(&RemoteUrl),
       "Add a new remote to the repository.\n(This is not an im-memory "
       "operation and changes the repository!)")
    ("remote-name", po::value<std::string>(&RemoteName),
       "Name for the new  remote (e.g. \'upstream\')")
    ("print-conflicts", "List all conflicts.")
    ("verbose,v", "Be verbose.")("help,h", "Print this help text.")
  ;

  po::options_description GlobalPositionalDummy("Global positional options");
  GlobalPositionalDummy.add_options()
    ("command", po::value<std::string>(), "command to execute")
    ("subargs", po::value<std::vector<std::string>>(), "Arguments for command")
  ;

  po::options_description All("Allowed options");
  All.add(Global).add(GlobalPositionalDummy);

  po::positional_options_description Pos;
  Pos.add("command", 1).add("subargs", -1);

  po::options_description MergeDesc("Options for \'merge\' command");
  MergeDesc.add_options()
    ("our", po::value<std::string>(&MergeOurBranch)->required(),
       "Commit/Ref that reflects the destination tree")
    ("their", po::value<std::string>(&MergeTheirBranch)->required(),
       "Commit/Ref to merge in to \"our\" commit")
  ;

  po::options_description RebaseDesc("Options for \'rebase\' command");
  RebaseDesc.add_options()
    ("upstream", po::value<std::string>(&RebaseUpstreamBranch)->required(),
       "Upstream branch to compare against. Can be any valid commit.")
    ("branch", po::value<std::string>(&RebaseBranch)->required(),
       "Branch to rebase onto another branch. Can be any valid commit.")
    ("onto", po::value<std::string>(&RebaseOntoCommit),
       "(Optional) If specified, the new commits will start at this point. "
       "Can be any valid commit."
       "If unspecified, the starting point will be <upstream>.")
  ;
  /* clang-format on */

  po::variables_map Vm;
  po::parsed_options Parsed = po::command_line_parser(argc, argv)
                                  .options(All)
                                  .positional(Pos)
                                  .allow_unregistered()
                                  .run();
  po::store(Parsed, Vm);

  if (Vm.empty() || Vm.count("help") || Vm.count("command") != 1) {
    std::cout << "USAGE: mergecheck [command] [global options] "
                 "[command-specific options]\n\n";
    std::cout << "Available commands:\n"
              << "  merge\t\t\tJoin two development histories together\n"
              << "  rebase\t\tReapply commits on top of another base tip\n"
              << "\n";
    std::cout << Global << "\n";
    std::cout << MergeDesc << "\n";
    std::cout << RebaseDesc;
    return EXIT_SUCCESS;
  }

  Verbose = Vm.count("verbose") > 0;
  PrintConflicts = Vm.count("print-conflicts") > 0;
  std::string Command = Vm["command"].as<std::string>();

  bool HasRemoteUrlParam = Vm.count("remote-url") > 0;
  bool HasRemoteNameParam = Vm.count("remote-name") > 0;
  if (HasRemoteUrlParam && HasRemoteNameParam) {
    AddRemote = true;
  } else if (HasRemoteUrlParam ^ HasRemoteNameParam) {
    std::cerr << "Error: Options \"remote-url\" and \"remote-name\" have to "
                 "be provided together.";
    return EXIT_FAILURE;
  }

  // try to do some very simple normalization of the command line arguments,
  // e.g. removing whitespaces and trailing slashes (for urls)
  Trim(RepoPath);
  Trim(RemoteName);
  Trim(RemoteUrl);
  while (RemoteUrl.back() == '/') {
    RemoteUrl.pop_back();
  }

  // parse command-specifig cmd args

  // Collect all the unrecognized options from the first pass. This will
  // include the (positional) command name, so we need to erase that.
  std::vector<std::string> Opts =
      po::collect_unrecognized(Parsed.options, po::include_positional);
  Opts.erase(Opts.begin());

  if (Command == "merge") {
    try {
      po::store(po::command_line_parser(Opts).options(MergeDesc).run(), Vm);
      po::notify(Vm);
    } catch (const std::exception &Ex) {
      std::cerr << "\n" << Ex.what() << "\n\n";
      return EXIT_FAILURE;
    }
    Trim(MergeOurBranch);
    Trim(MergeTheirBranch);
  } else if (Command == "rebase") {
    try {
      po::store(po::command_line_parser(Opts).options(RebaseDesc).run(), Vm);
      po::notify(Vm);
    } catch (const std::exception &Ex) {
      std::cerr << "\n" << Ex.what() << "\n\n";
      return EXIT_FAILURE;
    }
    Trim(MergeOurBranch);
    Trim(MergeTheirBranch);
  } else {
    std::cerr << "Error! No legal command was specified! Use \"--help\" for "
                 "details.\n";
    return EXIT_FAILURE;
  }

  // done with command line parsing

  int error;
  git_repository *Repo = nullptr;
  git_libgit2_init();

  if (Verbose) {
    std::cout << "Opening repository..." << std::endl;
  }
  error = git_repository_open(&Repo, RepoPath.c_str());
  checkError(error, "opening repository");

  if (AddRemote) {
    addRemote(Repo, RemoteUrl, RemoteName, Verbose);
  }

  size_t Conflicts = 0;
  if (Command == "merge") {
    Conflicts =
        merge(Repo, MergeOurBranch, MergeTheirBranch, PrintConflicts, Verbose);
  } else if (Command == "rebase") {
    if (Vm.count("onto") == 1) {
      Conflicts = rebase(Repo, RebaseUpstreamBranch, RebaseBranch,
                         RebaseOntoCommit, PrintConflicts, Verbose);
    } else {
      Conflicts = rebase(Repo, RebaseUpstreamBranch, RebaseBranch,
                         PrintConflicts, Verbose);
    }
  }

  // clean up...
  git_repository_free(Repo);
  git_libgit2_shutdown();

  if (Conflicts > 0) {
    std::cout << "Found " << Conflicts << " conflicts in total." << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Good news, everyone! Branches can be merged automatically "
               "without conflicts."
            << std::endl;
  return EXIT_SUCCESS;
}
