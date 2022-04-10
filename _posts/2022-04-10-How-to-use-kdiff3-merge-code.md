---
layout: post
title:  "如何使用kdiff3处理merge时的冲突"
date:   2022-04-10 09:23:58 +0800
categories: jekyll update
tags:
  kdiff3 git
---
[kdiff3](https://download.kde.org/stable/kdiff3/)是天下最好用的比较和merge工具，支持linux windows和mac。下文我们将通过例子介绍如何使用kdiff3 来处理git merge时的冲突。
我们以工程https://github.com/jiayuehua/kdiff3-demo 为例说明如何使用kdiff3解决冲突
开始时master中的kdiff3-demo.cpp内容如下，

    // kdiff3-demo.cpp : Defines the entry point for the application.
    //

    #include "kdiff3-demo.h"

    using namespace std;

    int main()
    {
      cout << "Hello CMake." << endl;
      return 0;
    }

然后我们创建名为deque的分支，加入对deque排序的代码，并且提交

    // kdiff3-demo.cpp : Defines the entry point for the application.
    //

    #include "kdiff3-demo.h"
    #include <algorithm>
    #include <deque>
    #include <iostream>
    #include <functional>

    using namespace std;

    int main()
    {
      std::deque<int> v{ 1,2,9,5,7 };
      std::sort(v.begin(), v.end(), std::greater{});
      cout << "Hello CMake." << endl;
      return 0;
    }

然后我们切换回master分支，加入对vector排序的代码并且提交

    // kdiff3-demo.cpp : Defines the entry point for the application.
    //

    #include "kdiff3-demo.h"
    #include <iostream>
    #include <algorithm>
    #include <vector>

    using namespace std;



    int main()
    {
      std::vector<int> v1{ 1,3,5,9,7 };
      std::sort(v1.begin(), v1.end(), [](int l, int r) {
        return l > r;
        });
      cout << "Hello CMake." << endl;
      return 0;
    }

然后我们在master上合并deque分支的代码

    git merge deque


现在看到的代码文本内容：

    // kdiff3-demo.cpp : Defines the entry point for the application.
    //

    #include "kdiff3-demo.h"
    <<<<<<< HEAD
    #include <iostream>
    #include <algorithm>
    #include <vector>
    =======
    #include <algorithm>
    #include <deque>
    #include <iostream>
    #include <functional>
    >>>>>>> deque

    using namespace std;



    int main()
    {
    <<<<<<< HEAD
      std::vector<int> v1{ 1,3,5,9,7 };
      std::sort(v1.begin(), v1.end(), [](int l, int r) {
        return l > r;
        });
    =======
      std::deque<int> v{ 1,2,9,5,7 };
      std::sort(v.begin(), v.end(), std::greater{});
    >>>>>>> deque
      cout << "Hello CMake." << endl;
      return 0;
    }

可以看到有冲突，这时到了我们使用kdiff3显神威的地方了。
在工程的根目录下用右键快捷菜单选择tortoise中的resolve... ，
![rosolve](/images/tortoisegit.png "tortoisegit")

能看到出现冲突的文件列表，
![conflicts-list](/images/conflicts-list.png "conflicts list")
依次对于列表中的每一个文件双击便可唤起kdiff3，用于解决冲突。
![kdiff3](/images/kdiff3.png "kdiff3")


这时A窗口显示基，也就是master HEAD分支和deque分支的共同的祖先commit ID所对应的文本，B窗口显示本地的代码，C窗口显示远方这里是deque分支的代码。而底部output窗口显示合并的结果。

这时便可以使用merge菜单下拉列表进行合并，
![merge](/images/merge-menu.png "merge menu")

比如我们想全部使用deque分支的代码，可以选择 _Choose C Everywhere_ ,或者对所有有冲突的地方都使用本地代码，可以选择 _Choose B For All Unsolved Conflicts_ 。而工具栏中的这四个图标，
![diff-button](/images/diff-button.png "diff buttion") 分别用于跳转到上一个差异，下一个差异，上一个冲突，下一个冲突。另外底部的窗口你可直接编辑合并的结果，如改写变量名字，添加一行代码等。

编译完成后保存，回到tortoisegit解决冲突窗口列表，对每个文件都完成合并后，选择ok按钮。然后提交，就完成了合并。这里我们选择所有差异都是用C，也就是deque分支的代码。

kdiff3很方便易用吧，要配置tortoisegit使用kdiff3进行合并，需要在tortoisegit的setting中将diff viewer 配置为  _"C:\Program Files\KDiff3\kdiff3.exe" %base %mine --L1 %bname --L2 %yname_ 
![diff-viewer](/images/diff-viewer.png "diff viewer")

 将mergetool 配置为 _"C:\Program Files\KDiff3\kdiff3.exe"  %base %mine %theirs -o %merged --L1 %bname --L2 %yname --L3 %tname_ 。这里假定kdiff3位于"C:\Program Files\KDiff3\kdiff3.exe"。
![merge-viewer](/images/merge-viewer.png "merge viewer")

 
