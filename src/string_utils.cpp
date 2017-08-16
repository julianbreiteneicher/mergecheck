#include <algorithm>

#include "mergecheck/string_utils.hpp"

std::string &LeftTrim(std::string &S) {
  S.erase(S.begin(),
          std::find_if(S.begin(), S.end(),
                       std::not1(std::ptr_fun<int, int>(std::isspace))));
  return S;
}

std::string &RightTrim(std::string &S) {
  S.erase(std::find_if(S.rbegin(), S.rend(),
                       std::not1(std::ptr_fun<int, int>(std::isspace)))
              .base(),
          S.end());
  return S;
}

std::string &Trim(std::string &S) { return LeftTrim(RightTrim(S)); }
