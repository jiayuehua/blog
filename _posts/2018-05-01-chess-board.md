---
layout: post
title:  "chess board cover"
date:   2018-05-01 07:23:58 +0000
categories: jekyll update
---
1 * 2 瓷砖覆盖8 * 8地板
```cpp
#include <stdio.h>
#include <string.h>
enum {
  SZ = 8,
};
 
int foo(int a[][SZ])
{
  int r = 0;
  int i = 0, j = 0;
  bool hasfoundempty = false;
  for (i = 0; i < SZ; ++i) {
    for ( j = 0; j < SZ; ++j) {
      if (a[i][j] == 0) {
        hasfoundempty = true;
        goto LABEL;
      }
    }
  }
 
LABEL:
  if (!hasfoundempty)
  {
    return 1;
  }
  if (j+1<SZ && a[i][j+1] == 0)
  {
    a[i][j] = a[i][j+1] = 1;
    r += foo(a);
    a[i][j] = a[i][j+1] = 0;
  }
 
  if (i+1<SZ && a[i+1][j] == 0)
  {
    a[i][j] = a[i+1][j] = 1;
    r += foo(a);
    a[i][j] = a[i+1][j] = 0;
  }
  return r;
}
int main()
{
  int  a[SZ][SZ];
  memset(a[0],0, SZ*SZ*sizeof(int) );
  printf("result: %d\n", foo(a)) ;
}
```
