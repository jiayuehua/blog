---
layout: post
title:  "Reproducible github Developer Environments "
date:   2023-08-15 08:00:03 +0800
categories: jekyll update
tags:
  github
---

我們以 github上 jiayuehua/CMakeVcpkgManifest 代码库  爲例，说明如何在github上建立可重复的开发环境。

# 1. 增加devcontainer 配置文件
在visual stuido code中 C-S-P,选择CodeSpaces:add devcontainer configure files... 生成devcontainer配置文件和Dockerfile。

修改这两文件为和CMakeVcpkgManifest的内容完全一致，因为CMakeVcpkgManifest正确的配置了sshd和安装了X11 forwarding所需要的程序。将改动push到github。

# 2 . 添加codespace, 在github上添加codespace。

# 3 .本地使用ssh 连接codespace
在本地使用ssh 连接codespace，使用ssh -X 连接，可以使用X11 forwarding。

    ~> gh codespace list                                                                                                    14m 1s
    NAME                          DISPLAY NAME    REPOSITORY                    BRANCH  STATE      CREATED AT
    opulent-doodle-7p7xpx9wqw2j5  opulent doodle  jiayuehua/CMakeVcpkgManifest  master  Available  29m 

    ~> gh codespace ssh --config > ~/.ssh/codespaces 

    ~> cd .ssh                                                                                                                  5s
    ~/.ssh> ln id_rsa codespaces.auto
    ~/.ssh> ln id_rsa.pub codespaces.auto.pub
    ~/.ssh> ls                               
    authorized_keys  codespaces  codespaces.auto  codespaces.auto.pub  config  id_rsa  id_rsa.pub  known_hosts  known_hosts.old
    ~/.ssh> cat config    
    Match all
    Include ~/.ssh/codespaces
    ~> ssh -X cs.opulent-doodle-7p7xpx9wqw2j5.master 
    Welcome to Ubuntu 22.04.2 LTS (GNU/Linux 5.15.0-1042-azure x86_64)

     * Documentation:  https://help.ubuntu.com
     * Management:     https://landscape.canonical.com
     * Support:        https://ubuntu.com/advantage

    This system has been minimized by removing packages and content that are
    not required on a system that users do not log into.

    To restore this content, you can run the 'unminimize' command.
    Last login: Tue Aug 15 07:25:11 2023 from 127.0.0.1

    @jiayuehua ➜ /workspaces/CMakeVcpkgManifest (master) $ xeyes

可以看到xeyes在本地显示。这里假定你使用wslg安装了ubuntu,ubuntu里X11 server。这时我们便可以在本地使用visual studio code 对带图形界面的代码库进行调试了。

     

     



