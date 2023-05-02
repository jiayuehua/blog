---
layout: post
title:  "二叉树的反序列化"
date:   2013-07-10 09:25:58 +0800
categories: jekyll update
tags:
  parser 
---
经过艰苦卓绝的努力，实现了二叉树的反序列化，

二叉树需要反序列化以如下格式表示的二叉树

(hello(world )(soccer )) //hello代表父节点的值，world和soccer代表子节点的值，每个节点要么没有子节点，要么同时存在左右节点，比如右节点为空的表示为(hello(world )( ))。

使用boost.spirit实现

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
     
     
    struct BinTree;
    typedef boost::variant<
    std::string,
    boost::recursive_wrapper<BinTree>
    >BinTreeNode;
     
    struct BinTree
    {
        std::string data;
        BinTreeNode left;
        BinTreeNode right;
    };
     
    const int tabsize = 4;
    void printtab(int tabstop)
    {
        for (int i = 0; i < tabstop; ++i)
        {
            std::cout<<" ";
        }
    }
    struct BinTreePrinter;
    struct BinTreeNodePrinter: public boost::static_visitor<>
    {
        BinTreeNodePrinter(int tabstop = 0):tabstop_(tabstop){}
        void operator()(const std::string& s)const
        {
            printtab(tabstop_);std::cout<<s<<std::endl;
        }
        void operator()(const BinTree& m_xml)const;
    private:
        int tabstop_;
    };
    struct BinTreePrinter
    {
        BinTreePrinter(int tabstop = 0):tabstop_(tabstop){}
        void operator()(const BinTree & m_xml)const
        {
            printtab(tabstop_);std::cout<<"("<<std::endl;
            printtab(tabstop_);std::cout<<m_xml.data<<std::endl;
            boost::apply_visitor(BinTreeNodePrinter(tabstop_+tabsize),m_xml.left);
            boost::apply_visitor(BinTreeNodePrinter(tabstop_+tabsize),m_xml.right);
            printtab(tabstop_);std::cout<<")"<<std::endl;
        }
    private:
        int tabstop_;
    };
    void BinTreeNodePrinter::operator()(const BinTree& m_xml)const
    {
        BinTreePrinter m(tabstop_);
        m(m_xml);
    }
     
    BOOST_FUSION_ADAPT_STRUCT
    (BinTree,
    (std::string,data)
    (BinTreeNode,left)
    (BinTreeNode,right)
    )
     
    template <typename Iterator>
    class Grammar: public qi::grammar<Iterator, BinTree(),ascii::space_type>
    {
    public:
        Grammar():Grammar::base_type(xml){
            using ascii::char_;
            using ascii::string;
            using qi::lit;
            using namespace qi::labels;
            using qi::lexeme;
            start_tag = '(' ;
            end_tag = ')';
            node %= xml| text;
            text %= lexeme[*(char_ - '('-')')] ;
            xml %= start_tag
                >> text>>node>>node
                >> end_tag;
        }
    private:
        qi::rule<Iterator, BinTree(),ascii::space_type> xml;
        qi::rule<Iterator, void(),ascii::space_type> start_tag;
        qi::rule<Iterator, void(),ascii::space_type> end_tag;
        qi::rule<Iterator, std::string(),ascii::space_type> text;
        qi::rule<Iterator, BinTreeNode(),ascii::space_type> node;
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
        BinTree ast;
        std::string::const_iterator iter = storage.begin();
        std::string::const_iterator end = storage.end();
        bool r = qi::phrase_parse(iter, end, grammar , ascii::space, ast);
     
        if (r && iter == end)
        {
            std::cout << "-------------------------\n";
            std::cout << "parsing succeeded\n";
            std::cout << "-------------------------\n";
            BinTreePrinter printer;
            printer(ast);
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
