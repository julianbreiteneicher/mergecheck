#ifndef MERGECHECK_CONFLICT_HPP
#define MERGECHECK_CONFLICT_HPP

#include <git2.h>
#include <ostream>
#include <string>

typedef struct {
  const git_index_entry *Ancestor;
  const git_index_entry *Our;
  const git_index_entry *Their;
} Conflict ;

std::ostream &printConflict(Conflict C, const std::string &LocalRef,
                            const std::string &RemoteRef, std::ostream &O);

#endif /* MERGECHECK_CONFLICT_HPP */
