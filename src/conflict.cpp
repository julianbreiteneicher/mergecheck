#include "mergecheck/conflict.hpp"

std::ostream &printConflict(Conflict C, const std::string &LocalRef,
                            const std::string &RemoteRef, std::ostream &O) {
  if (C.Our == nullptr) {
    O << "CONFLICT (modify/delete): " << C.Their->path
      << " deleted in HEAD and modified in " << RemoteRef << ".\n";
  } else if (C.Their == nullptr) {
    O << "CONFLICT (modify/delete): " << C.Our->path << " deleted in "
      << LocalRef << " and modified in HEAD.\n";
  } else {
    std::string reason = "content";
    // "normal conflict"
    if (C.Ancestor == nullptr) {
      reason = "add/add";
    }
    O << "CONFLICT (" << reason << "): Merge conflict in " << C.Our->path
      << "\n";
  }
  return O;
}
