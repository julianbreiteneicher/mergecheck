#include <iostream>
#include <sstream>

#include "mergecheck/conflict.hpp"
#include "mergecheck/merge.hpp"
#include "mergecheck/utils.hpp"

size_t merge(git_repository *repo, std::string our_branch,
             std::string their_branch, bool print_conflicts, bool verbose) {
  int error;

  git_reference *our_ref, *their_ref;
  git_annotated_commit *our_head, *their_head;
  git_commit *ours, *theirs;
  const char *our_branch_short, *their_branch_short;

  // get "our_branch" commit
  error = git_reference_lookup(&our_ref, repo, our_branch.c_str());
  if (!error) {
    error = git_annotated_commit_from_ref(&our_head, repo, our_ref);
    check_error(error, "git_annotated_commit");
    // get short name for nicer program output
    error = git_branch_name(&our_branch_short, our_ref);
    check_error(error, "git_branch_name");
  } else {
    // try to parse as commit id
    git_oid oid;
    error = git_oid_fromstrp(&oid, our_branch.c_str());
    check_error(error, "git_oid_fromstrp");
    error = git_annotated_commit_lookup(&our_head, repo, &oid);
    check_error(error, "git_annotated_commit");
  }
  error = git_commit_lookup(&ours, repo, git_annotated_commit_id(our_head));
  check_error(error, "git_commit_lookup");

  // get "their_branch" commit
  error = git_reference_lookup(&their_ref, repo, their_branch.c_str());
  if (!error) {
    error = git_annotated_commit_from_ref(&their_head, repo, their_ref);
    check_error(error, "git_annotated_commit");
    // get short name for nicer program output
    error = git_branch_name(&their_branch_short, their_ref);
    check_error(error, "git_branch_name");
  } else {
    // try to parse as commit id
    git_oid oid;
    error = git_oid_fromstrp(&oid, their_branch.c_str());
    check_error(error, "git_oid_fromstrp");
    error = git_annotated_commit_lookup(&their_head, repo, &oid);
    check_error(error, "git_annotated_commit");
  }
  error = git_commit_lookup(&theirs, repo, git_annotated_commit_id(their_head));
  check_error(error, "git_commit_lookup");

  if (verbose) {
    std::cout << "Attempting to merge..." << std::endl;
  }

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
  Conflict c;
  while ((error = git_index_conflict_next(&c.ancestor, &c.our, &c.their,
                                          conflict_it)) == 0) {
    conflicts++;
    if (print_conflicts) {
      printConflict(c, our_branch_short ? our_branch_short : our_branch,
                    their_branch_short ? their_branch_short : their_branch,
                    std::cout);
    }
  }
  git_index_conflict_iterator_free(conflict_it);

  if (verbose) {
    std::cout << "Finished merging.\n";
  }

  if (error != GIT_ITEROVER) {
    std::cerr << "An error occured during merging!\n";
  }

  // clean up merge stuff...
  git_index_free(merge_index);
  git_commit_free(ours);
  git_commit_free(theirs);
  git_annotated_commit_free(our_head);
  git_annotated_commit_free(their_head);
  git_reference_free(our_ref);
  git_reference_free(their_ref);

  return conflicts;
}
