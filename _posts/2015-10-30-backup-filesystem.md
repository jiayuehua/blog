---
layout: post
title:  "使用backupfs备份文件系统"
date:   2016-10-14 09:24:58 +0800
categories: jekyll update
tags:
  application
---
backupfs 是肇始于plan 9的文件系统，每天可以方便的将重要的文件的方便快捷的做备份，是程序员的利器，计算机程序设计艺术的作者Don knuth是backupfs的众多使用者之一。

1下载backupfs 安装包backupfs-1.0beta10.tar.bz2，

需要在客户机和服务器上都下载和解压

backupfs 下载地址位于 http://sourceforge.net/projects/backupfs/

下载到linux，完成后使用tar命令解压，

    server@thinkpad:~/downloads/tmp$ tar jxvf backupfs-1.0beta10.tar.bz2 

2在客户和服务器上安装backupfs

编译

    server@thinkpad:~/downloads/tmp/backupfs$ make

以root身份安装

    server@thinkpad:~/downloads/tmp/backupfs$ su

    root@thinkpad:~/downloads/tmp/backupfs$ make install

客户端执行同样的操作。

 

3在服务端生成公私钥

server@thinkpad:~/downloads/tmp/backupfs$ make ssh-keygen

将私钥(id_rsa)拷贝到备份根目录，并将其重命名为.id_rsa，假定备份根目录是/backup/foo，将id_rsa拷贝为/backup/foo/.id_rsa。

    server@thinkpad:~/downloads/tmp/backupfs$ sudo mkdir /backup/foo

    server@thinkpad:~/downloads/tmp/backupfs$ sudo cp id_rsa /backup/foo/.id_rsa

4 客户端下载和添加公钥

在客户端将第3步生成的公钥下载到客户端本地，并且添加到$HOME/.ssh/authorized_keys中

    echo id_rsa.pub >>$HOME/.ssh/authorized_keys

5  开始备份

在服务器上 以root身份备份客户数据，假定客户机器地址为foo，服务器上运行如下命令

    root@thinkpad:~$ backupfs foo:/etc /backup/foo

第一次使用时需要输入backupfs的密码，以后就不需要了，因为第一次执行时会将客户信息加入到/root/.ssh/known_host。

大功告成！恭喜你！！

让我们看看我们的备份目录吧

    server@thinkpad:/backup/foo/2015/10/22/$ ls /backup/foo/2015/10/22/etc

可以看到我们已经成功的将foo机器的etc备份到了服务器，路径中的数字表示备份日期。^
