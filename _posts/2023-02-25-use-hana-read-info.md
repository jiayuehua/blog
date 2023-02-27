---
layout: post
title:  "使用boost hana的静态反射功能读取结构体的字段"
date:   2023-02-25 09:24:58 +0800
categories: jekyll update
tags:
   static-reflection 
---
做数据的序列化和反序列化时候，如果有静态反射，会极大简化要做的工作。这是因为有静态反射的话，很多解析和格式化的工作可以由同一个模板函数完成。

这里我们使用boost hana演示静态反射。展示如何读取不同结构体成员的数据。为了简单，这些结构体各字段都是数值。数据从字符串里读取，字符串里的数以空格分割。

如

    struct Point{
       int x;
    };

    struct Pair{
       int  x;
       int y;
    };

Point 类型的数据p存在字符串"15"里，Pair类型的数据存放在字符串"3 4"里。
完整的代码:

    #include <iostream>
    #include <string_view>
    #include <boost/hana.hpp>
    namespace hana = boost::hana;
    struct Point{
      BOOST_HANA_DEFINE_STRUCT(
        Point,
       (int , x)
      );
      static inline const char * fmtString = "%d";
    };
    struct Pair{
      BOOST_HANA_DEFINE_STRUCT(
        Pair,
       (int , x),
       (int , y)
      );
      static inline const char * fmtString = "%d %d";
    };

    template<class U>
    U ReadInfo(std::string_view buf)
    {
      U p;
      hana::unpack(hana::accessors<U>(),[&buf,&p]<class... T>(T&&... f){
        auto n = std::sscanf(buf.data(), p.fmtString, &hana::second(f)(p)...);
        if(n != hana::size(p))
        {
          throw std::string_view ("read fail");
        }
      });
      return p;
    }

    int main(){
      Point p{3};
      p = ReadInfo<Point>("15");
      Pair pa = ReadInfo<Pair>("3 5");
      std::cout<<"p.x: "<< p.x <<std::endl;
      std::cout<<"pa.x: "<< pa.x ; 
      std::cout<<" , pa.y: "<< pa.y <<std::endl;
    }

可以看到，我们首先需要修改Point和Pair类的定义，使用 BOOST_HANA_DEFINE_STRUCT 来让类支持静态反射，ReadInfo函数模板是核心的部分，模板参数U是要读取的类型如Point,buf是存放数据的字符串。同时加入了静态成员fmtString用于表示从sscanf中读取时候的格式。      
对于支持反射的类U来说，我们可以将其当做tuple来处理，tuple的每一个元素是一个pair,pair的first是U中某字段的名字，而second是对应的值。hana\::unpack类似标准库的std\::apply，也就是对于tuple，应用一个函数到tuple，函数的每个参数对应于tuple的每个元素。这里将tuple每个元素的second的地址依次传递给sscanf，sscanf从函数参数buf中读取各个元素的值，对应于结构体如Point和Pair各个字段的值。    
main函数中我们测试了我们的ReadInfo，可以看到ReadInfo非常通用，即可成功用于读取Point，也可读取Pair。一切要归功于hana的静态反射。