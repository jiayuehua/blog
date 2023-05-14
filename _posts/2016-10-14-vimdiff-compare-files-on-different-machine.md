---
layout: post
title:  "使用vimdiff比较不同机器上的文件"
date:   2016-10-14 09:24:58 +0800
categories: jekyll update
tags:
  vim
---

假定要比较host上位于 /etc/nginx/conf/nginx.conf 和本地的nginx.conf
 vimdiff scp://someone@host//etc/nginx/conf/nginx.conf nginx.conf  
注意host 后有两个 '/' ，这时要比较的是绝对路径

如果要比较相对路径 
 vimdiff scp://someone@host/.bashrc .bashrc  
host后 只有一个'/'，这时在比较位于host 上~someone目录的.bashrc 和本目录的.bashrc
