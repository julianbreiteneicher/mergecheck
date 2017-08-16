#include <iostream>
#include <sstream>

#include "mergecheck/conflict.hpp"
#include "mergecheck/rebase.hpp"
#include "mergecheck/utils.hpp"

namespace {
size_t rebaseHelper(git_repository *Repo, const char *UpstreamBranch,
                    const char *Branch, const char *Onto, bool PrintConflicts,
                    bool Verbose) {
  int error;

  git_reference *UpstreamRef, *BranchRef, *OntoRef;
  git_annotated_commit *UpstreamCommit, *BranchCommit, *OntoCommit;

  // get "UpstreamBranch" commit
  error = git_reference_lookup(&UpstreamRef, Repo, UpstreamBranch);
  if (!error) {
    error = git_annotated_commit_from_ref(&UpstreamCommit, Repo, UpstreamRef);
    checkError(error, "git_annotated_commit");
  } else {
    // try to parse as commit id
    git_oid Oid{};
    error = git_oid_fromstrp(&Oid, UpstreamBranch);
    checkError(error, "git_oid_fromstrp");
    error = git_annotated_commit_lookup(&UpstreamCommit, Repo, &Oid);
    checkError(error, "git_annotated_commit");
  }

  // get "Branch" commit
  error = git_reference_lookup(&BranchRef, Repo, Branch);
  if (!error) {
    error = git_annotated_commit_from_ref(&BranchCommit, Repo, BranchRef);
    checkError(error, "git_annotated_commit");
  } else {
    // try to parse as commit id
    git_oid Oid{};
    error = git_oid_fromstrp(&Oid, Branch);
    checkError(error, "git_oid_fromstrp");
    error = git_annotated_commit_lookup(&BranchCommit, Repo, &Oid);
    checkError(error, "git_annotated_commit");
  }

  // get "Onto" commit
  if (Onto) {
    error = git_reference_lookup(&OntoRef, Repo, Onto);
    if (!error) {
      error = git_annotated_commit_from_ref(&OntoCommit, Repo, OntoRef);
      checkError(error, "git_annotated_commit");
    } else {
      // try to parse as commit id
      git_oid Oid{};
      error = git_oid_fromstrp(&Oid, Onto);
      checkError(error, "git_oid_fromstrp");
      error = git_annotated_commit_lookup(&OntoCommit, Repo, &Oid);
      checkError(error, "git_annotated_commit");
    }
  }

  size_t Conflicts = 0;
  git_rebase *Rebase;

  git_rebase_options Opts{};
  git_rebase_init_options(&Opts, GIT_REBASE_OPTIONS_VERSION);
  Opts.inmemory = 1;

  if (Onto) {
    git_rebase_init(&Rebase, Repo, BranchCommit, UpstreamCommit, OntoCommit,
                    &Opts);
  } else {
    git_rebase_init(&Rebase, Repo, BranchCommit, UpstreamCommit, nullptr,
                    &Opts);
  }

  git_rebase_operation *RebaseOp;
  while (git_rebase_next(&RebaseOp, Rebase) == 0) {
    git_commit *RebaseCommit;
    git_commit_lookup(&RebaseCommit, Repo, &RebaseOp->id);
    std::string CommitMsg(git_commit_summary(RebaseCommit));

    if (Verbose) {
      std::cout << "Applying commit \"" << CommitMsg << "\"" << std::endl;
    }

    git_commit_free(RebaseCommit);

    git_index *RebaseIndex;
    git_rebase_inmemory_index(&RebaseIndex, Rebase);

    if (git_index_has_conflicts(RebaseIndex)) {
      // get conflicts
      git_index_conflict_iterator *ConflictIt;
      git_index_conflict_iterator_new(&ConflictIt, RebaseIndex);

      Conflict C{};
      while ((error = git_index_conflict_next(&C.Ancestor, &C.Our, &C.Their,
                                              ConflictIt)) == 0) {
        Conflicts++;
        if (PrintConflicts) {
          printConflict(C, CommitMsg, CommitMsg, std::cout);
        }
      }
      git_index_conflict_iterator_free(ConflictIt);
    }

    git_index_free(RebaseIndex);
  }

  error = git_rebase_finish(Rebase, nullptr);
  checkError(error, "git_rebase_finish");

  // clean up

  git_annotated_commit_free(UpstreamCommit);
  git_annotated_commit_free(BranchCommit);
  git_reference_free(UpstreamRef);
  git_reference_free(BranchRef);
  if (Onto) {
    git_annotated_commit_free(OntoCommit);
    git_reference_free(OntoRef);
  }

  git_rebase_free(Rebase);

  return Conflicts;
}
} // namespace

size_t rebase(git_repository *Repo, const std::string &UpstreamBranch,
              const std::string &Branch, bool PrintConflicts, bool Verbose) {
  return rebaseHelper(Repo, UpstreamBranch.c_str(), Branch.c_str(), nullptr,
                      PrintConflicts, Verbose);
}

size_t rebase(git_repository *Repo, const std::string &UpstreamBranch,
              const std::string &Branch, const std::string &OntoCommit,
              bool PrintConflicts, bool Verbose) {
  return rebaseHelper(Repo, UpstreamBranch.c_str(), Branch.c_str(),
                      OntoCommit.c_str(), PrintConflicts, Verbose);
}
