---
layout: post
title:  "A better ostream_iterator"
date:   2021-04-12 09:23:58 +0800
categories: jekyll update
tags:
  iterator
---
ostream_iterator 可方便的用于算法。但有个缺陷，使用ostream_iterator必须指定要输出类型的type，如ostream_iterator\<int\>。正如各种标准库的谓词可以不指定模板参数，ostream_iterator的设计也可改为不再需要指定模板参数，而将输出操作也就是复制函数改为模板成员函数
```cpp
#include <iterator>
#include <iosfwd>
//now don't need to specify output type, all template parameter has default type.
template <class charT=char, class traits=std::char_traits<charT> >
  class OstreamIter :
    public std::iterator<std::output_iterator_tag, void, std::ptrdiff_t, void, void>
{
  std::basic_ostream<charT,traits>* out_stream;
  const charT* delim;

public:
  typedef charT char_type;
  typedef traits traits_type;
  typedef std::basic_ostream<charT,traits> ostream_type;
  OstreamIter(ostream_type& s) : out_stream(&s), delim(0) {}
  OstreamIter(ostream_type& s, const charT* delimiter)
    : out_stream(&s), delim(delimiter) { }
  OstreamIter(const OstreamIter<charT,traits>& x)
    : out_stream(x.out_stream), delim(x.delim) {}
  ~OstreamIter() {}
  OstreamIter() =default;
  //now is template function
  template< class T >
  OstreamIter<charT,traits>& operator= (const T& value) {
    *out_stream << value;
    if (delim!=0) *out_stream << delim;
    return *this;
  }

  OstreamIter<charT,traits>& operator*() { return *this; }
  OstreamIter<charT,traits>& operator++() { return *this; }
  OstreamIter<charT,traits>& operator++(int) { return *this; }
};
```
使用ctad，现在我们的OstreamIter的对象初始化时不再需要指定OstreamIter模板参数了。

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <range/v3/all.hpp>
int main()
{
  std::vector v{0,1,2,3,4};
  //std::ostream_iterator<int> oit(std::cout,",");
  OstreamIter o(std::cout,",");
  ranges::copy(v,o);
  std::cout<<"\n";
  std::copy(v.begin(),v.end(),o);
  std::cout<<"\n";
  ranges::copy(v,oit);
  std::cout<<"\n";
}
```
[godbold](https://godbolt.org/z/9njGMYdd1)
