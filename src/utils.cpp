#include <iostream>

#include <git2.h>

#include "mergecheck/utils.hpp"

void check_error(int error_code, const std::string action) {
  const git_error *error = giterr_last();
  if (!error_code) {
    return;
  }

  std::cerr << "Error " << error_code << " " << action << " - "
            << ((error && error->message) ? error->message : "???") << "\n";

  exit(EXIT_FAILURE);
}
