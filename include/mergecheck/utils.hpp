#ifndef MERGECHECK_UTILS_HPP
#define MERGECHECK_UTILS_HPP

#include <string>

/**
 * If there is an error, print out an error message and exit the program.
 * Otherwise, do nothing.
 */
void checkError(int ErrorCode, const std::string &Action);

#endif /* MERGECHECK_UTILS_HPP */
