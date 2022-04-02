---
layout: post
title:  "第一次向linux内核提交代码"
date:   2020-09-15 09:23:58 +0800
categories: jekyll update
---
windows wsl2上ubuntu没有自带perf，需要手工编译安装，编译时发现内核perf的三个bug：数组越界，返回悬空引用的指针，全局变量重复定义导致连接错误。已提交修复，希望能被接受。

[perf pull request](https://github.com/microsoft/WSL2-Linux-Kernel/pull/186)

有点小兴奋。
