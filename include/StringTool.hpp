#ifndef STRING_TOOL_HPP
#define STRING_TOOL_HPP
#include <string>
#include <vector> // container for result

inline bool startswith(const std::string& str, const std::string& start)
{
  int srclen = str.size();
  int startlen = start.size();
  if (srclen >= startlen)
  {
    std::string temp = str.substr(0, startlen);
    if (temp == start)
      return true;
  }
  return false;
}

/**
 * @brief split string
 * @author https://www.cnblogs.com/dfcao/p/cpp-FAQ-split.html @egmkang
 */
std::vector<std::string> split(const std::string& str, const std::string& c)
{
  std::vector<std::string> result;
  std::string::size_type pos1, pos2;
  pos2 = str.find(c);
  pos1 = 0;
  while (std::string::npos != pos2)
  {
    result.push_back(str.substr(pos1, pos2 - pos1));

    pos1 = pos2 + c.size();
    pos2 = str.find(c, pos1);
  }
  if (pos1 != str.length())
    result.push_back(str.substr(pos1));
  return result;
}
#endif