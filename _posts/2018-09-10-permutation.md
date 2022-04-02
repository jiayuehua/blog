---
layout: post
title:  "permutaion"
date:   2018-09-10 07:23:56 +0800
categories: jekyll update
---
1 2 3 4 的全排列
```cpp
#include <iostream>
#include <vector>
#include <iterator>
 
std::vector<std::string> foo(std::string s)
{
  std::vector<std::string> res;
  if (s.size() == 0)
  {
    return res;
  }
  else if (s.size() == 1)
  {
    res.push_back(s);
    return res;
  }
  else
  {
    std::vector<std::string> res2 = foo(std::string(s.begin() + 1, s.end()));
    for (std::vector<std::string>::iterator i = res2.begin(); i != res2.end(); ++i)
    {
      int sz = i->size();
      for (int j = 0; j <= sz; ++j)
      {
        std::string tmp = *i;
        tmp.insert(tmp.begin() + j, s[0]);
        res.push_back(tmp);
      }
    }
    return res;
  }
}
 
int main(int argc, char *argv[])
{
  std::vector <std::string> v = foo(“1234”);
  std::ostream_iterator<std::string> oit(std::cout, “\n”);
  std::copy(v.begin(), v.end(), oit);
  return 0;
}
```