#ifndef MERGECHECK_STRING_UTILS_HPP
#define MERGECHECK_STRING_UTILS_HPP

#include <string>

/**
 * Remove all whitespaces from the left.
 */
std::string &ltrim(std::string &s);

/**
 * Remove all whitespaces from the right.
 */
std::string &rtrim(std::string &s);

/**
 * Remove whitespaces from both sides of the string.
 */
std::string &trim(std::string &s);

#endif /* MERGECHECK_STRING_UTILS_HPP */

