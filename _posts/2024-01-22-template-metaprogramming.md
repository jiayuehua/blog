---
layout: post
title:  "adapting c++20 ranges algorithms for most metaprogramming"
date:   2024-01-22 08:00:03 +0800
categories: jekyll update
tags:
  metaprogramming
---


daisy在cppnow 2023的演讲中展示了可极大简化模板元算法的技巧。
以排序为例
基本思路是 
将type_list中的每个Type映射到他们的index,
然后对index数组做sort，结果为sorted_indices,
最终type_list中各个元素按照在sorted_indices新位置重新安放，返回结果type_list.

    #include <type_traits>
    #include <ranges>
    #include <array>
    #include <algorithm>
    template <class...>
    struct type_list {};

    template <class List, auto Key>
    struct Sort {};

    template<size_t N, class ... Ts>
    using at_t = std::tuple_element_t<N, std::tuple<Ts...>>;
    template<class ... Ts, auto Key>
    struct Sort<type_list<Ts...>, Key>
        : std::type_identity<decltype([]<size_t... Idx>(std::index_sequence<Idx...>) {
          constexpr auto sorted_indices = [] {
            std::array idxs = {Idx...};
            std::ranges::sort(idxs, [](size_t i, size_t j) {
              using variant_t = std::variant<std::type_identity<Ts>...>;
              std::array vars = {variant_t{std::in_place_index_t<Idx>{}}...  };
              return std::visit(Key, vars[i]) < std::visit(Key, vars[j]);
              });
            return idxs;
            
            }();
            return type_list<at_t<sorted_indices[Idx], Ts...>...>{};
        }(std::index_sequence_for<Ts...>{})

      )

    > {};

    int main(){

     using type=  Sort<type_list<double,int,char>, []<class T>(std::type_identity<T>){ return sizeof(T);}>::type ;
     static_assert(std::is_same_v<type, type_list<char,int,double> >);

    }

这里的排序算法的comparator非常巧妙，

        [](size_t i, size_t j) {
                using variant_t = std::variant<std::type_identity<Ts>...>;
              std::array vars = {variant_t{std::in_place_index_t<Idx>{}}...  };
              return std::visit(Key, vars[i]) < std::visit(Key, vars[j]);
              }


因为i 和j是运行时变量，而我们需要编译时取到type_list中第i个元素和第j个元素，做法是构造一个variant的数组，variant的每项依次构造为std::type_identity\<Ts\>...,然后使用std::visit访问数组中的第i项和第j项，因为该lambda是constexpr的，所以可以用于编译时调用。

Key是以std::type_indentity为参数的仿函数

      []<class T>(std::type_identity<T>){ return sizeof(T);}

这里使用type_identity\<T\>而不是T做参数是因为,T可能是void或者数组或者不完全类型等不能作为参数的类型，而使用type_indentity\<T\>做参数可解决该问题。

main函数展示了如何使用该方法依据类型的size对typelist中的type排序。
