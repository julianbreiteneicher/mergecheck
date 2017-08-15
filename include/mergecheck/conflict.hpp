#ifndef MERGECHECK_CONFLICT_HPP
#define MERGECHECK_CONFLICT_HPP

#include <git2.h>
#include <ostream>
#include <string>

typedef struct {
  const git_index_entry *ancestor;
  const git_index_entry *our;
  const git_index_entry *their;
} Conflict;

std::ostream &printConflict(Conflict c, std::string local_ref,
                            std::string remote_ref, std::ostream &o);

#endif /* MERGECHECK_CONFLICT_HPP */

