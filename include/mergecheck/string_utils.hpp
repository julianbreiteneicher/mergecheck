#ifndef MERGECHECK_STRING_UTILS_HPP
#define MERGECHECK_STRING_UTILS_HPP

#include <string>

/**
 * Remove all whitespaces from the left.
 */
std::string &LeftTrim(std::string &S);

/**
 * Remove all whitespaces from the right.
 */
std::string &RightTrim(std::string &S);

/**
 * Remove whitespaces from both sides of the string.
 */
std::string &Trim(std::string &S);

#endif /* MERGECHECK_STRING_UTILS_HPP */
