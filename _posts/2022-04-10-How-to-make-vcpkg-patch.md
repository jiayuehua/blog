---
layout: post
title:  "手把手教你如何为microsoft/vcpkg提交PR"
date:   2022-04-10 09:23:58 +0800
categories: jekyll update
tags:
  vcpkg
---
[vcpkg](www.github.com/microsoft/vcpkg)是微软出品的C++软件包管理器，包含超过1700个库。使用时如果喜欢的库不在microsoft/vcpkg中或者发现安装后有bug,或者需要升级库到新版本时，可以给microsoft/vcpkg提交PR。本文介绍提交升级vcpkg中库的PR。

以abseil库为例说明。
每个软件包的对应的cmake文件位于ports 目录
假定我们想将abseil由20210324升级到20211102版本.

## 修改vcpkg.json
将ports/abseil/vcpkg.json中的version-string字段改为新版本的version-string.
## 修改portfile.cmake
首先我们修改ports/abseil/portfile.cmake，将vcpkg_from_github中的PATCHS 参数中的所有内容删除，REF字段的commitID改为新版本20211102对应的git commitID. SHA512改为0.
如何获取新版本的SHA512呢？在vcpkg工程根目录下运行./vcpkg install abseil，终端打印出如下内容

    ...
    File does not have expected hash:

          File path: [ /home/rob/vcpkg/downloads/abseil-abseil-cpp-215105818dfde3174fe799600bb0f3cae233d0bf.tar.gz ]
      Expected hash: [ 00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000 ]
        Actual hash: [ 75d234eac76be8790cf09e3e1144e4b4cf5cacb61e46961a9e4a35b37d0fa85243afdd5de5f47a006ef96af6fc91ecc0c233297c4c32258c08d46304b3361330 ]
    ...  
 
将SHA512字段由0改为 75d234eac76be8790cf09e3e1144e4b4cf5cacb61e46961a9e4a35b37d0fa85243afdd5de5f47a006ef96af6fc91ecc0c233297c4c32258c08d46304b3361330  。


## 重新制作patch
由portfile.cmake中vcpkg_from_github函数可以看到abseil 20210321版本有两个patch，查看内容后发现其中一个已经合入官方abseil的20211102版本，因此对于新版本，我们只需要制作一个patch:fix-cxx-standard.patch。
### 安装abseil
    ~/vcpkg/$ ./vcpkg install abseil
    Computing installation plan...
    The following packages will be built and installed:
        abseil[core]:x64-linux -> 20211102.1
    Detecting compiler hash for triplet x64-linux...
    Restored 0 packages from /home/rob/.cache/vcpkg/archives in 6.639 us. Use --debug to see more details.
    Starting package 1/1: abseil:x64-linux
    Building package abseil[core]:x64-linux...
    -- Using cached abseil-abseil-cpp-215105818dfde3174fe799600bb0f3cae233d0bf.tar.gz.
    -- Extracting source /home/rob/vcpkg/downloads/abseil-abseil-cpp-215105818dfde3174fe799600bb0f3cae233d0bf.tar.gz
    -- Using source at /home/rob/vcpkg/buildtrees/abseil/src/cae233d0bf-c7dcd484c3.clean
    ...

从上文中可以看到abseil的20211102版本的源码位于/home/rob/vcpkg/buildtrees/abseil/src/cae233d0bf-c7dcd484c3.clean,
我们到/home/rob/vcpkg/buildtrees/abseil/src/cae233d0bf-c7dcd484c3.clean目录制作patch。

### 制作新patch
vcpkg使用git diff的输出制作patch，为了使用git diff。我们首先在
/home/rob/vcpkg/buildtrees/abseil/src/cae233d0bf-c7dcd484c3.clean 目录依次运行

    git init
    git add *
    git commit -m "tmp"
    
这样我们就在/home/rob/vcpkg/buildtrees/abseil/src/cae233d0bf-c7dcd484c3.clean中创建了一个git repo并提交了第一个commit.
然后需要制作 fix-cxx-standard.patch，我们先看看老的patch能否直接使用:

    ~/vcpkg/buildtrees/abseil/src/cae233d0bf-c7dcd484c3.clean$ patch -p1 < ../../../../ports/abseil/fix-cxx-standard.patch 
    patching file CMake/abslConfig.cmake.in
    patching file CMakeLists.txt
    Hunk #1 succeeded at 74 with fuzz 1 (offset 17 lines).
    patching file absl/base/CMakeLists.txt
    patching file absl/base/options.h


成功了，我们不需要自己修改options.h等文件了。
然后更新fix-cxx-standard.patch
 
    ~/vcpkg/buildtrees/abseil/src/cae233d0bf-c7dcd484c3.clean$ git diff . > ../../../../ports/abseil/fix-cxx-standard.patch

再次修改ports/abseil/portfile.cmake 将fix-cxx-standard.patch加回vcpkg_from_github PATCHS参数

    vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO abseil/abseil-cpp
        REF 215105818dfde3174fe799600bb0f3cae233d0bf #LTS 20211102, Patch 1
        SHA512 75d234eac76be8790cf09e3e1144e4b4cf5cacb61e46961a9e4a35b37d0fa85243afdd5de5f47a006ef96af6fc91ecc0c233297c4c32258c08d46304b3361330
        HEAD_REF master
        PATCHES
            # in C++17 mode, use std::any, std::optional, std::string_view, std::variant
            # instead of the library replacement types
            # in C++11 mode, force use of library replacement types, otherwise the automatic
            # detection can cause ABI issues depending on which compiler options
            # are enabled for consuming user code
          fix-cxx-standard.patch
    )

## 重新安装验证新patch能工作

    ~/vcpkg$ ./vcpkg remove abseil
    ~/vcpkg$ ./vcpkg install abseil
    Computing installation plan...
    The following packages will be built and installed:
        abseil[core]:x64-linux -> 20211102.1
    Detecting compiler hash for triplet x64-linux...
    Restored 0 packages from /home/rob/.cache/vcpkg/archives in 3.718 us. Use --debug to see more details.
    Starting package 1/1: abseil:x64-linux
    Building package abseil[core]:x64-linux...
    -- Using cached abseil-abseil-cpp-215105818dfde3174fe799600bb0f3cae233d0bf.tar.gz.
    -- Cleaning sources at /home/rob/vcpkg/buildtrees/abseil/src/cae233d0bf-9df62c4674.clean. Use --editable to skip cleaning for the packages you specify.
    -- Extracting source /home/rob/vcpkg/downloads/abseil-abseil-cpp-215105818dfde3174fe799600bb0f3cae233d0bf.tar.gz
    -- Applying patch fix-cxx-standard.patch
    -- Using source at /home/rob/vcpkg/buildtrees/abseil/src/cae233d0bf-9df62c4674.clean
    -- Configuring x64-linux-dbg
    -- Configuring x64-linux-rel
    -- Building x64-linux-dbg

## 提交更改
更改需要分两次提交，因为vcpkg的versions/中用到ports/中的git tree index的信息。
首先提交一次 ports的更改

    git add ports/abseil
    git commit -m"[abseil] update to 20211102"

然后修改versions

    ~/vcpkg$ ./vcpkg  x-add-version  --skip-version-format-check abseil 
    ~/vcpkg$ git status 
    On branch proxygen
    Your branch is ahead of 'origin/proxygen' by 2 commits.
      (use "git push" to publish your local commits)

    Changes not staged for commit:
      (use "git add <file>..." to update what will be committed)
      (use "git restore <file>..." to discard changes in working directory)
      modified:   versions/a-/abseil.json
      modified:   versions/baseline.json

然后再amend versions的修改
    
    git commit  -a --amend --no-edit

大功告成! 接下来，我们便可以将对abseil的更新push到自己fork的vcpkg仓库，然后向microsoft/vcpkg提交pr了。

## bonus: 制作一个port的多个patch

有时我们需要制作多个patch，针对不同的功能，这时需要在port的buildtree下的源码目录提交多次

        ~/openSrc/vcpkg/buildtrees/abseil/src/cae233d0bf-0b16e4be7c.clean master> git log
         34c4a53 (Wed Apr 13 11:39:33 2022) jiayuehua  second patch
         3209c04 (Wed Apr 13 11:37:44 2022) jiayuehua  first patch
         ae508e7 (Wed Apr 13 11:34:53 2022) jiayuehua  tmp

这里tmp是直接下载源码后的提交，"first patch"是第一个改动的提交，"second patch"是第二个改动的提交。确保源码目录所有改动都已经提交

        ~/openSrc/vcpkg/buildtrees/abseil/src/cae233d0bf-0b16e4be7c.clean master> git status
        On branch master
        nothing to commit, working tree clean

这时可运行vcpkgformatpatch

    ~/openSrc/vcpkg/buildtrees/abseil/src/cae233d0bf-0b16e4be7c.clean master> vcpkgformatpatch 
    0001-first-patch.patch
    0002-second-patch.patch

然后将0001-firt-patch.patch 和0002-second-patch.patch拷贝到 ~/openSrc/vcpkg/ports/abseil/目录，再修改 ~/openSrc/vcpkg/ports/abseil/portfile.cmake 中的vcpkg_from_github函数，将这两个patch作为PATCHES的参数

     vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO abseil/abseil-cpp
        REF 215105818dfde3174fe799600bb0f3cae233d0bf #LTS 20211102, Patch 1
        SHA512 75d234eac76be8790cf09e3e1144e4b4cf5cacb61e46961a9e4a35b37d0fa85243afdd5de5f47a006ef96af6fc91ecc0c233297c4c32258c08d46304b3361330
        HEAD_REF master
        PATCHES
          0001-first-patch.patch
          0002-second-patch.patch
    )

vcpkgformatpatch是我写的一个脚本，内容

    /usr/local/bin> cat vcpkgformatpatch 
    #!/bin/zsh
    CommitsNum=`git rev-list --count master`
    git format-patch --no-stat --no-signature "-$((CommitsNum-1))"
    sed -i -n '/^diff/,$p' 0*.patch

你便了解了制作多个patch的全部奥秘。

参考了 https://github.com/microsoft/vcpkg/pull/22017 我以前成功被merge的abseil的Pull request。
