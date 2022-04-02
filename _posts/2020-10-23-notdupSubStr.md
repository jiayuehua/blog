---
layout: post
title:  "查找字符串最长的不包含重复字符的子串"
date:   2020-10-23 09:23:58 +0800
categories: jekyll update
---
给定一个字符串，查找最长的不包含重复字符的子串

用法例子：  
./findSubStr   asfdsadfkassadsf   
sadfk

```cpp
#include <vector>
#include <iostream>
#include <iterator>
#include <string_view>
auto notdupcharSubString(std::string_view s)
{
  const char*      p = s.data();
  std::vector<int> char2pos(256, -1);
  int              b   = 0;
  int              len = 0;
  std::pair        sub{0, 0};

  int i = 0;
  for (; i < s.size(); ++i) {
    if (char2pos[*(p + i)] != -1) {
      int sublen = i - b;
      if (sublen > len) {
        len = sublen;
        sub = {b, i};
      }
      for (; b <= char2pos[*(p + i)]; ++b) {
         char2pos[*(p + b)] = -1;
      }
    }
    char2pos[*(p + i)] = i;
  }
  int sublen = i - b;
  if (sublen > len) {
    len = sublen;
    sub = {b, i};
  }
  return sub;
}
int main(int argc, char** argv)
{
  if (argc != 2) {
    return 0;
  }
  auto p      = notdupcharSubString(argv[1]);
  auto [b, e] = p;
  std::copy(argv[1] + b, argv[1] + e, std::ostream_iterator<char>(std::cout, ""));
}
```
