#include <iostream>
#include <sstream>

#include "mergecheck/conflict.hpp"
#include "mergecheck/merge.hpp"
#include "mergecheck/utils.hpp"

size_t merge(git_repository *Repo, const std::string &OurBranch,
             const std::string &TheirBranch, bool PrintConflicts,
             bool Verbose) {
  int error;

  git_reference *OurRef, *TheirRef;
  git_annotated_commit *OurHead, *TheirHead;
  git_commit *Ours, *Theirs;
  const char *OurBranchShort = nullptr;
  const char *TheirBranchShort = nullptr;

  // get "OurBranch" commit
  error = git_reference_lookup(&OurRef, Repo, OurBranch.c_str());
  if (!error) {
    error = git_annotated_commit_from_ref(&OurHead, Repo, OurRef);
    checkError(error, "git_annotated_commit");
    // get short name for nicer program output
    error = git_branch_name(&OurBranchShort, OurRef);
    checkError(error, "git_branch_name");
  } else {
    // try to parse as commit id
    git_oid Oid{};
    error = git_oid_fromstrp(&Oid, OurBranch.c_str());
    checkError(error, "git_oid_fromstrp");
    error = git_annotated_commit_lookup(&OurHead, Repo, &Oid);
    checkError(error, "git_annotated_commit");
  }
  error = git_commit_lookup(&Ours, Repo, git_annotated_commit_id(OurHead));
  checkError(error, "git_commit_lookup");

  // get "TheirBranch" commit
  error = git_reference_lookup(&TheirRef, Repo, TheirBranch.c_str());
  if (!error) {
    error = git_annotated_commit_from_ref(&TheirHead, Repo, TheirRef);
    checkError(error, "git_annotated_commit");
    // get short name for nicer program output
    error = git_branch_name(&TheirBranchShort, TheirRef);
    checkError(error, "git_branch_name");
  } else {
    // try to parse as commit id
    git_oid Oid{};
    error = git_oid_fromstrp(&Oid, TheirBranch.c_str());
    checkError(error, "git_oid_fromstrp");
    error = git_annotated_commit_lookup(&TheirHead, Repo, &Oid);
    checkError(error, "git_annotated_commit");
  }
  error = git_commit_lookup(&Theirs, Repo, git_annotated_commit_id(TheirHead));
  checkError(error, "git_commit_lookup");

  if (Verbose) {
    std::cout << "Attempting to merge..." << std::endl;
  }

  git_index *MergeIndex;
  git_merge_options MergeOpts{};
  error = git_merge_init_options(&MergeOpts, GIT_MERGE_OPTIONS_VERSION);
  checkError(error, "git_merge_init_options");

  error = git_merge_commits(&MergeIndex, Repo, Ours, Theirs, &MergeOpts);
  checkError(error, "git_merge_commits");

  // get conflicts
  git_index_conflict_iterator *ConflictIt;
  git_index_conflict_iterator_new(&ConflictIt, MergeIndex);
  checkError(error, "iterator empty");

  size_t Conflicts = 0;
  Conflict C{};
  while ((error = git_index_conflict_next(&C.Ancestor, &C.Our, &C.Their,
                                          ConflictIt)) == 0) {
    Conflicts++;
    if (PrintConflicts) {
      printConflict(C, OurBranchShort ? OurBranchShort : OurBranch,
                    TheirBranchShort ? TheirBranchShort : TheirBranch,
                    std::cout);
    }
  }
  git_index_conflict_iterator_free(ConflictIt);

  if (Verbose) {
    std::cout << "Finished merging.\n";
  }

  if (error != GIT_ITEROVER) {
    std::cerr << "An error occured during merging!\n";
  }

  // clean up merge stuff...
  git_index_free(MergeIndex);
  git_commit_free(Ours);
  git_commit_free(Theirs);
  git_annotated_commit_free(OurHead);
  git_annotated_commit_free(TheirHead);
  git_reference_free(OurRef);
  git_reference_free(TheirRef);

  return Conflicts;
}
