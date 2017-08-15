#include "mergecheck/conflict.hpp"

std::ostream &printConflict(Conflict c, std::string local_ref,
                            std::string remote_ref, std::ostream &o) {
  if (!c.our) {
    o << "CONFLICT (modify/delete): " << c.their->path
      << " deleted in HEAD and modified in " << remote_ref << ".\n";
  } else if (!c.their) {
    o << "CONFLICT (modify/delete): " << c.our->path << " deleted in "
      << local_ref << " and modified in HEAD.\n";
  } else {
    std::string reason = "content";
    // "normal conflict"
    if (!c.ancestor) {
      reason = "add/add";
    }
    o << "CONFLICT (" << reason << "): Merge conflict in " << c.our->path
      << "\n";
  }
  return o;
}

