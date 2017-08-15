#ifndef MERGECHECK_UTILS_HPP
#define MERGECHECK_UTILS_HPP

#include <string>

/**
 * If there is an error, print out an error message and exit the program.
 * Otherwise, do nothing.
 */
void check_error(int error_code, const std::string action);

#endif /* MERGECHECK_UTILS_HPP */

