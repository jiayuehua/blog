---
layout: post
title:  "括号匹配，使用boost-spirit实现"
date:   2013-07-10 09:24:58 +0800
categories: jekyll update
tags:
  quotes 
---
如下代码可以匹配四种类型的括号,大括号、小括号、中括号和尖括号。允许各种嵌套。并且允许输入串中包含其他字符。

使用boost.spirit 实现。真是艰难困苦，玉汝于成。

// 用法

./a.out data_file

//data_file内容例子

(hello (world))


    #include <boost/spirit/include/qi.hpp>
    #include <boost/spirit/include/phoenix_core.hpp>

    #include <boost/spirit/include/phoenix_operator.hpp>
    #include <boost/spirit/include/phoenix_stl.hpp>
    #include <boost/fusion/include/adapt_struct.hpp>
    #include <boost/variant/recursive_variant.hpp>
    #include <algorithm>
    #include <vector>
    #include <string>
    #include <iostream>
    #include <fstream>
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;
         
    struct ExprParentheses;
    struct ExprBrace;
    struct ExprSquareBrackets;
    struct ExprAngleBrackets;
    typedef boost::variant<
    std::string,
    boost::recursive_wrapper<ExprParentheses>,
    boost::recursive_wrapper<ExprBrace>,
    boost::recursive_wrapper<ExprSquareBrackets>,
    boost::recursive_wrapper<ExprAngleBrackets>
    >ExprNode;
    //These are Parentheses, Square brackets, Angle brackets and Brace
    struct ExprParentheses
    {
        std::string name;
        ExprNode left;
        ExprNode right;
    };
         
    struct ExprBrace
    {
        std::string name;
        ExprNode left;
        ExprNode right;
    };
    struct ExprSquareBrackets
    {
        std::string name;
        ExprNode left;
        ExprNode right;
    };
         
    struct ExprAngleBrackets
    {
        std::string name;
        ExprNode left;
        ExprNode right;
    };
    BOOST_FUSION_ADAPT_STRUCT
    (ExprParentheses,
        (std::string,name)
    (ExprNode,left)
    (ExprNode,right)
    )
    BOOST_FUSION_ADAPT_STRUCT
    (ExprBrace,
        (std::string,name)
    (ExprNode,left)
    (ExprNode,right)
    )
    BOOST_FUSION_ADAPT_STRUCT
    (ExprSquareBrackets,
        (std::string,name)
    (ExprNode,left)
    (ExprNode,right)
    )
    BOOST_FUSION_ADAPT_STRUCT
    (ExprAngleBrackets,
        (std::string,name)
    (ExprNode,left)
    (ExprNode,right)
    )
    const int tabsize = 4;
    void printtab(int tabstop)
    {
        for (int i = 0; i < tabstop; ++i)
        {
            std::cout<<" ";
        }
    }
    struct ExprNodePrinter: public boost::static_visitor<>
    {
        void operator()(const std::string&)const
        {
        }
        ExprNodePrinter(int tabstop = 0):tabstop_(tabstop){}
        void operator()(const ExprParentheses& m_xml)const;
        void operator()(const ExprBrace& m_xml)const;
        void operator()(const ExprSquareBrackets& m_xml)const;
        void operator()(const ExprAngleBrackets& m_xml)const;
    private:
        int tabstop_;
    };
    struct ExprPrinter
    {
        ExprPrinter(int tabstop, char left, char right):tabstop_(tabstop),left_(left), right_(right){}
        template <class T>
        void operator()(const T & m_xml)const
        {
            printtab(tabstop_);std::cout<<left_<<std::endl;
            boost::apply_visitor(ExprNodePrinter(tabstop_+tabsize),m_xml.left);
            printtab(tabstop_);std::cout<<right_<<std::endl;
            boost::apply_visitor(ExprNodePrinter(tabstop_),m_xml.right);
        }
    private:
        int tabstop_;
        char left_;
        char right_;
    };
    void ExprNodePrinter::operator()(const ExprBrace& m_xml)const
    {
        ExprPrinter m(tabstop_, '{', '}');
        m(m_xml);
    }
         
    void ExprNodePrinter::operator()(const ExprParentheses& m_xml)const
    {
        ExprPrinter m(tabstop_, '(', ')');
        m(m_xml);
    }
         
    void ExprNodePrinter::operator()(const ExprSquareBrackets& m_xml)const
    {
        ExprPrinter m(tabstop_, '[', ']');
        m(m_xml);
    }
    void ExprNodePrinter::operator()(const ExprAngleBrackets& m_xml)const
    {
        ExprPrinter m(tabstop_, '<', '>');
        m(m_xml);
    }
    template <typename Iterator>
    class Grammar: public qi::grammar<Iterator, ExprNode(),ascii::space_type>
    {
    public:
        Grammar():Grammar::base_type(node){
            using ascii::char_;
            using ascii::string;
            using qi::lit;
            using qi::eps;
            using namespace qi::labels;
            using qi::lexeme;
            text %= lexeme[*(
                char_ - '(' - ')' -'['-']'-'<'-'>'-'{'-'}'
                )];
            node %= brace_expr|parentheses_expr|squarebrackets_expr|anglebrackets_expr|text;
            brace_expr %= text>>'{'
                >>node
                >>'}'
                >>node;
            parentheses_expr %= text>>'('
                >>node
                >>')'
                >>node;
            squarebrackets_expr %= text>>'['
                >>node
                >>']'
                >>node;
            anglebrackets_expr %= text>>'<'
                >>node
                >>'>'
                >>node;
        }
    private:
        qi::rule<Iterator, ExprBrace(),ascii::space_type> brace_expr;
        qi::rule<Iterator, ExprParentheses(),ascii::space_type> parentheses_expr;
        qi::rule<Iterator, ExprSquareBrackets(),ascii::space_type> squarebrackets_expr;
        qi::rule<Iterator, ExprAngleBrackets(),ascii::space_type> anglebrackets_expr;
        qi::rule<Iterator, ExprNode(),ascii::space_type> node;
        qi::rule<Iterator, std::string(),ascii::space_type> text;
    };
    int main(int argc, char** argv)
    {
    char const* filename;
    if (argc > 1)
    {
    filename = argv[1];
    }
    else
    {
    std::cerr << "Error: No input file provided." << std::endl;
    return 1;
    }
    std::ifstream in(filename, std::ios_base::in);
         
    if (!in)
    {
    std::cerr << "Error: Could not open input file: "
    << filename << std::endl;
    return 1;
    }
    std::string storage; // We will read the contents here.
    in.unsetf(std::ios::skipws); // No white space skipping!
    std::copy(
    std::istream_iterator<char>(in),
    std::istream_iterator<char>(),
    std::back_inserter(storage));
        Grammar<std::string::const_iterator> grammar;
        ExprNode ast;
    std::string::const_iterator iter = storage.begin();
    std::string::const_iterator end = storage.end();
        bool r = qi::phrase_parse(iter, end, grammar , ascii::space, ast);
         
    if (r && iter == end)
    {
    std::cout << "-------------------------\n";
    std::cout << "parsing succeeded\n";
    std::cout << "-------------------------\n";
    ExprNodePrinter printer;
            boost::apply_visitor(printer,ast);
    return 0;
    }
    else
    {
    std::string context(iter, (end-iter)>30?iter+30:end);
    std::cout << "-------------------------\n";
    std::cout << "parsing failed\n";
    std::cout << "stopped at: \": " << context << "...\"\n";
    std::cout << "-------------------------\n";
    return 1;
    }
    }
