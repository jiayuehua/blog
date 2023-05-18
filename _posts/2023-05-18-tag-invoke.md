---
layout: post
title:  "cpo和tag_invoke"
date:   2023-05-18 08:00:02 +0800
categories: jekyll update
tags:
  container 
---

customization point object(cpo)和tag_invoke由eric niebler提出，在ranges和executor中有很多应用。
cpo可用于对库中的一些仿函数进行定制，而tag_invoke用于实现cpo,用于用户的类区分对于不同库的定制。

    #include <gtest/gtest.h>
    #include <folly/Utility.h>
    #include <folly/lang/CustomizationPoint.h>
    #include <folly/functional/Invoke.h>
    template<class T>
    struct Tag {
    };

    template<class T>
    constexpr auto tag_c = Tag<T>{};
    namespace A
    {
    struct FooCpo {
     template<typename T>
     auto operator()(Tag<T> t) const
         noexcept(folly::is_nothrow_tag_invocable_v<FooCpo, Tag<T> >)
         -> folly::tag_invoke_result_t<FooCpo, Tag<T>> {
       return folly::tag_invoke(*this, t);
     }
    };
    struct BarCpo {
         template<typename A>
         auto operator()(A&&a ) const
             noexcept(folly::is_nothrow_tag_invocable_v<BarCpo, A>)
             -> folly::tag_invoke_result_t<BarCpo, A> {
           return folly::tag_invoke(*this, (A&&)a);
         }
    };
    FOLLY_DEFINE_CPO(BarCpo, doathing)
    FOLLY_DEFINE_CPO(FooCpo, dosomething)
    }

    namespace B
    {
    struct FooCpo {
     template<typename A>
     auto operator()(Tag<A> t) const
         noexcept(folly::is_nothrow_tag_invocable_v<FooCpo, Tag<A> >)
         -> folly::tag_invoke_result_t<FooCpo, Tag<A>> {
       return folly::tag_invoke(*this, t);
     }
    };
    FOLLY_DEFINE_CPO(FooCpo, dosomething)
    }
    namespace My
    {
    class SomeClass
    {
      friend void tag_invoke(folly::cpo_t<A::dosomething>, Tag<SomeClass>)
      { std::cout << "A someclass\n";
      }
      friend void tag_invoke(folly::cpo_t<A::doathing>, const  SomeClass& some)
      { std::cout << "A doathing someclass\n";
      }
      friend void tag_invoke(folly::cpo_t<B::dosomething>, Tag<SomeClass>)
      { std::cout << "B someclass\n";
      }
    };

    }

    TEST(cpo, basic)
    {
      My::SomeClass c;
      A::doathing(c);
    }
    TEST(cpo, advance)
    {
      A::dosomething(tag_c<My::SomeClass>);
      B::dosomething(tag_c<My::SomeClass>);
    }

上文的 BarCpo 示范了如何定义一个cpo, BarCpo有一个模板成员operator(),内部调用tag_invoke, 
    FOLLY_DEFINE_CPO(BarCpo, doathing) 帮我们定义了BarCpo类型的doathing cpo.

类My::SomeClass friend void tag_invoke(folly::cpo_t<A::doathing>, const  SomeClass& some)实现了对A::doathing的定制。TEST(cpo,basic)中的A:::doathing(c)调用了我们实现的定制。

My::SomeClass 的友元void tag_invoke(folly::cpo_t<A::dosomething>, Tag<SomeClass>)和 friend void tag_invoke(folly::cpo_t<B::dosomething>, Tag<SomeClass>)
示范了对于不同名字空间A和B的dosomthing cpo，tag_invoke可方便的分别定制.


