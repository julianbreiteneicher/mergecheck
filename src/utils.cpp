#include <iostream>

#include <git2.h>

#include "mergecheck/utils.hpp"

void checkError(int ErrorCode, const std::string &Action) {
  const git_error *Error = giterr_last();
  if (!ErrorCode) {
    return;
  }

  std::cerr << "Error " << ErrorCode << " " << Action << " - "
            << ((Error && Error->message) ? Error->message : "???") << "\n";

  exit(EXIT_FAILURE);
}
