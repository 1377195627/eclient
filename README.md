eclient
适用于win上校园网用inode客户端等802.1x拨号的客户端
目前支持开机自启，自动连接

前端由易语言编写

核心程序由c/c++编写，移植于linux下的njit-client，有能力者希望可以继续更新下去


编译：
编译需要的东西和运行需要的东西 下有需要的易语言的自绘支持库和用到的模块，还有编译client需要的pcap的头文件等
ConsoleApplication3就是client的源码，在vs2015下新建
eclient由易语言编译
notify.dll是易语言与c之间通信的中转站，但是会造成定时自动退出，所以最后一个版本client被编译成命令行程序

运行：
需要安装winpcap和vc2015运行库，编译需要的东西和运行需要的东西 下有

eclient beta5终结版下载
https://raw.githubusercontent.com/1377195627/eclient/master/eClientBeta6%E7%BB%88%E7%BB%93%E7%89%88.zip

eclient beta5最终版下载
https://raw.githubusercontent.com/1377195627/eclient/master/eClientBeta5%E6%9C%80%E7%BB%88%E7%89%88.zip

以往的适用于更新说明

运行需要安装winpcap以及vc2015运行库（暂无法确定是否还有别的运行库需要）
由于前端程序是易语言写的，杀软可能会误报，请点信任

eclient理论上不再更新，但会开放源码
https://github.com/1377195627/eclient

beta6
本版本为终结版
修复开始连接client程序闪退问题

beta5
本版本为最终版
将原dll编译成命令行版，拨号结果直接看命令行，理论上不会再自动退出


beta4
解决用端时间就奔溃问题（不完美解决，把提示都去掉了，下个版本回归）

beta3
调整ui
添加开机自启（保存密码自动登录）
其它优化


beta2
添加保存密码功能，开机自启还没弄，需要安装winpcap，还有安装各种运行库，暂时无法确定是要安装哪几个，还有需要在win8.1上运行
