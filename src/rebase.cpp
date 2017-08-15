#include <iostream>
#include <sstream>

#include "mergecheck/conflict.hpp"
#include "mergecheck/rebase.hpp"
#include "mergecheck/utils.hpp"

namespace {
size_t rebaseHelper(git_repository *repo, const char *upstream_branch,
                    const char *branch, const char *onto, bool print_conflicts,
                    bool verbose) {
  int error;

  git_reference *upstream_ref, *branch_ref, *onto_ref;
  git_annotated_commit *upstream_commit, *branch_commit, *onto_commit;

  // get "upstream_branch" commit
  error = git_reference_lookup(&upstream_ref, repo, upstream_branch);
  if (!error) {
    error = git_annotated_commit_from_ref(&upstream_commit, repo, upstream_ref);
    check_error(error, "git_annotated_commit");
  } else {
    // try to parse as commit id
    git_oid oid;
    error = git_oid_fromstrp(&oid, upstream_branch);
    check_error(error, "git_oid_fromstrp");
    error = git_annotated_commit_lookup(&upstream_commit, repo, &oid);
    check_error(error, "git_annotated_commit");
  }

  // get "branch" commit
  error = git_reference_lookup(&branch_ref, repo, branch);
  if (!error) {
    error = git_annotated_commit_from_ref(&branch_commit, repo, branch_ref);
    check_error(error, "git_annotated_commit");
  } else {
    // try to parse as commit id
    git_oid oid;
    error = git_oid_fromstrp(&oid, branch);
    check_error(error, "git_oid_fromstrp");
    error = git_annotated_commit_lookup(&branch_commit, repo, &oid);
    check_error(error, "git_annotated_commit");
  }

  // get "onto" commit
  if (onto) {
    error = git_reference_lookup(&onto_ref, repo, onto);
    if (!error) {
      error = git_annotated_commit_from_ref(&onto_commit, repo, onto_ref);
      check_error(error, "git_annotated_commit");
    } else {
      // try to parse as commit id
      git_oid oid;
      error = git_oid_fromstrp(&oid, onto);
      check_error(error, "git_oid_fromstrp");
      error = git_annotated_commit_lookup(&onto_commit, repo, &oid);
      check_error(error, "git_annotated_commit");
    }
  }

  size_t conflicts = 0;
  git_rebase *rebase;

  git_rebase_options opts;
  git_rebase_init_options(&opts, GIT_REBASE_OPTIONS_VERSION);
  opts.inmemory = 1;

  if (onto) {
    git_rebase_init(&rebase, repo, branch_commit, upstream_commit, onto_commit,
                    &opts);
  } else {
    git_rebase_init(&rebase, repo, branch_commit, upstream_commit, NULL, &opts);
  }

  git_rebase_operation *rebase_op;
  while (git_rebase_next(&rebase_op, rebase) == 0) {
    git_commit *rebase_commit;
    git_commit_lookup(&rebase_commit, repo, &rebase_op->id);
    std::string commit_msg(git_commit_summary(rebase_commit));

    if (verbose) {
      std::cout << "Applying commit \"" << commit_msg << "\"" << std::endl;
    }

    git_commit_free(rebase_commit);

    git_index *rebase_index;
    git_rebase_inmemory_index(&rebase_index, rebase);

    if (git_index_has_conflicts(rebase_index)) {
      // get conflicts
      git_index_conflict_iterator *conflict_it;
      git_index_conflict_iterator_new(&conflict_it, rebase_index);

      Conflict c;
      while ((error = git_index_conflict_next(&c.ancestor, &c.our, &c.their,
                                              conflict_it)) == 0) {
        conflicts++;
        if (print_conflicts) {
          printConflict(c, commit_msg, commit_msg, std::cout);
        }
      }
      git_index_conflict_iterator_free(conflict_it);
    }

    git_index_free(rebase_index);
  }

  error = git_rebase_finish(rebase, NULL);
  check_error(error, "git_rebase_finish");

  // clean up

  git_annotated_commit_free(upstream_commit);
  git_annotated_commit_free(branch_commit);
  git_reference_free(upstream_ref);
  git_reference_free(branch_ref);
  if (onto) {
    git_annotated_commit_free(onto_commit);
    git_reference_free(onto_ref);
  }

  git_rebase_free(rebase);

  return conflicts;
}
}

size_t rebase(git_repository *repo, std::string upstream_branch,
              std::string branch, bool print_conflicts, bool verbose) {
  return rebaseHelper(repo, upstream_branch.c_str(), branch.c_str(), NULL,
                      print_conflicts, verbose);
}

size_t rebase(git_repository *repo, std::string upstream_branch,
              std::string branch, std::string onto_commit, bool print_conflicts,
              bool verbose) {
  return rebaseHelper(repo, upstream_branch.c_str(), branch.c_str(),
                      onto_commit.c_str(), print_conflicts, verbose);
}
