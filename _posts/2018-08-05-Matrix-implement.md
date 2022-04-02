---
layout: post
title:  "TC++PL matrix analysis"
date:   2018-08-05 07:23:51 +0000
categories: jekyll update
---
C++程序设计语言 第四版 有一章专门介绍了matrix，但其中没有讲解do_slice的实现，该matrix库github链接 https://github.com/asutton/origin-google/blob/master/origin/math/matrix/matrix.impl/slice.hpp 我们谈谈它的do_slice实现
```cpp
// Compute the extents and strides for the specified slice in the dim dimension.
// Note that dim is an integral_constant specifying the dimension in which the
// slice is computed.
template<std::size_t N>
  template<std::size_t D, std::size_t M>
  inline std::size_t
  matrix_slice<N>::do_slice_dim(const matrix_slice<M>& desc, slice s)
  {
    // If the starting point is past the extent, we're requesting the
    // entire slice.
    if (s.start >= desc.extents[D])
      s.start = 0;
 
    // If the lenght is large or the slice requests more elements than are
    // available, make it stop at the right extent.
    if (s.length > desc.extents[D] || s.start + s.length > desc.extents[D])
      s.length = desc.extents[D] - s.start;
 
    // If the stride over-runs the edge of the matrix, re-compute the length
    // so that we stop after the right number of increments. This is:
    //
    //    l = ceil(d/s)
    //
    // where d is the distance from the start to the extent, and s is the
    // stride. 
    if (s.start + s.length * s.stride > desc.extents[D])
      s.length = ((desc.extents[D] - s.start) + s.stride - 1) / s.stride;
 
    // Compute the extents and stride in this dimension.
    extents[D] = s.length;
    strides[D] = desc.strides[D] * s.stride;
    return s.start * desc.strides[D];
  }
 
// Slicing a single column is the same as a 1-count slice at the current
// dimension. Here, dim is an integral_constant specifying the dimension
// being sliced.
template<std::size_t N>
  template<std::size_t D, std::size_t M>
    inline std::size_t
    matrix_slice<N>::do_slice_dim(const matrix_slice<M>& desc, std::size_t n)
    {
      return do_slice_dim<D>(desc, slice(n, 1, 1));
    }
 
 
// Translate the slice arguments is {s, args...} into the output slice, and
// return the offset of the first element of the matrix. Note that the returned
// element is effectively computed as the dot product of the starting offsets of
// each slice argument with the strides of this slice.
//
// All of the heavy lifting is done in do_slice_dim.
//
// TODO: It is possible to reduce the dimensionality of the resulting slice when
// we encounter slice::none arguments or plain index. An index requests only a
// single element, so we can effectively compute drop the corresponding
// dimension.
 
template<std::size_t N>
  template<std::size_t M, typename T, typename... Args>
    inline std::size_t
    matrix_slice<N>::do_slice(const matrix_slice<M>& desc, 
                              const T& s, 
                              const Args&... args)
    {
      constexpr std::size_t D = N - sizeof...(Args) - 1;
      std::size_t m = do_slice_dim<D>(desc, s);
      std::size_t n = do_slice(desc, args...);
      return m + n;
    }
 
template<std::size_t N>
  template<std::size_t M>
    inline std::size_t
    matrix_slice<N>::do_slice(const matrix_slice<M>& desc)
    {
      return 0; 
    }
```
首先该矩阵是多维的，而不是二维的，现实中多维的例子比如时钟，可以看成三维矩阵，10:05:03，可以看成是24*60*60的矩阵中的一个元素，10:05:03转换为秒可以这么计算10* 50 * 60+5 * 60+3 * 1,从代码中可以看到do_slice计算可以分为两步，第一步先计算最低维，第二步计算高维，用我们的例子来说就是先使用do_slice_dim计算10 * 60 * 60，然后计算5 * 60+3 * 1.

第7行的Do_slice_dim实现分析

第9-18行将参数调整为合适的slice 

第26行 s代表本维的切割，如果切割的结束位置超过本维的长度，调整切割的长度，调整算法是本维长度减去切割起始位置+本维跨度-1；然后除以本维跨度。

  0 	  1 	2	  3 	  4 	  5 	  6

比如本维长度为7，跨度为2，start位置为0，（7+2-1)/2=4，切割后本维的四个下标是0,2,4,6。
 if (s.start + s.length * s.stride > desc.extents[D])
      s.length = ((desc.extents[D] - s.start) + s.stride - 1) / s.stride;

30行 子矩阵本维长度为本维切割的长度，子矩阵本维跨度为本维切割跨度乘以本维跨度。
另一do_slice_dim的实现用于递归结束时do_slice_dim的调用。
挺巧妙的。