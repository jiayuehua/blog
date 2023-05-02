---
layout: post
title:  "将16进制字符串转化为10进制数"
date:   2013-07-10 09:24:58 +0800
categories: jekyll update
tags:
  parser 
---
使用boost.spirit语法分析实现，暂时不能处理负数

    #include <boost/config/warning_disable.hpp>
    #include <boost/spirit/include/qi.hpp>
    #include <boost/spirit/include/phoenix_operator.hpp>
     
    #include <iostream>
    #include <string>
     
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;
    struct HEXNUM :qi::symbols<char,int>
    {
        
        HEXNUM()
        {
            add("0",0);
            add("1",1);
            add("2",2);
            add("3",3);
            add("4",4);
            add("5",5);
            add("6",6);
            add("7",7);
            add("8",8);
            add("9",9);
            add("a",10);
            add("A",10);
            add("b",11);
            add("B",11);
            add("c",12);
            add("C",12);
            add("d",13);
            add("D",13);
            add("e",14);
            add("E",14);
            add("f",15);
            add("F",15);
        }
    }hexnum;
    template<class Iter>
    class Grama:public qi::grammar<Iter, int()>
    {
    public:
        Grama():Grama::base_type(hexrule){
            using qi::_val;
            using qi::_1;
            using qi::eps;
            using qi::lit;
            using qi::char_;
     
            hexrule = (lit("0x")[_val=1]|lit("0X")[_val=0])>>
                    hexnum[_val=_1]>>*(hexnum [_val = 16*_val+_1] );
        }
    private:
        qi::rule<Iter, int()> hexrule;
    };
     
    int main()
    {
      std::cout << "/////////////////////////////////////////////////////////\n\n";
      std::cout << "\t\tHex Numerals Parser\n\n";
      std::cout << "/////////////////////////////////////////////////////////\n\n";
      std::cout << "Type a Hex Numeral ...or [q or Q] to quit\n\n";

      typedef std::string::const_iterator iterator_type;
      typedef Grama<iterator_type>        GRAMA;

      GRAMA grama;

      std::string str;
      unsigned    result;
      while (std::getline(std::cin, str)) {
        if (str.empty() || str[0] == 'q' || str[0] == 'Q') break;

        std::string::const_iterator iter = str.begin();
        std::string::const_iterator end  = str.end();
        bool                        r    = parse(iter, end, grama, result);

        if (r && iter == end) {
          std::cout << "-------------------------\n";
          std::cout << "Parsing succeeded\n";
          std::cout << "result = " << result << std::endl;
          std::cout << "-------------------------\n";
        } else {
          std::string rest(iter, end);
          std::cout << "-------------------------\n";
          std::cout << "Parsing failed\n";
          std::cout << "stopped at: \": " << rest << "\"\n";
          std::cout << "-------------------------\n";
        }
      }

      std::cout << "Bye... :-) \n\n";
      return 0;
    }

