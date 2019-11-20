# 该项目已经停止更新，新版的 oncedb-server.exe 请至 [onceoa.com](http://onceoa.com) 下载，自行从安装包中取中。oncedb-server.exe可免费使用，不限商业用途。

# 最新[开发者文档](http://onceoa.com/wiki/view/oncedb) 



## OnceDB

OnceDB基于Redis开发。将redis从一个键值对内存数据库，改造成支持查询和关系的NoSQL数据库。

- [OnceDB on Linux](https://github.com/OnceDoc/OnceDB)
- [OnceDB on Windows](https://github.com/OnceDoc/OnceDB.win)


## 安装

Linux 

    git clone https://github.com/OnceDoc/OnceDB.git
    cd OnceDB
    make

Windows

请到 [Release](https://github.com/OnceDoc/OnceDB.win/releases) 下载二进制可执行文件


## Redis参考

- [Redis指令参考](https://redis.io)
- [Redis on Linux](https://github.com/antirez/redis)
- [Redis on windows](https://github.com/MSOpenTech/Redis)



## 搜索指令说明

### search [key pattern] [operator] [value]

搜索字符串类型的健值

准备测试数据

    127.0.0.1:6379> set test1 'this is testing'
    OK
    127.0.0.1:6379> set test2 'this is article'
    OK
    127.0.0.1:6379> set test3 kris
    OK
    127.0.0.1:6379> set test4 10
    OK
    127.0.0.1:6379> set test5 100
    OK

= 完全匹配搜索: 在指定pattern的key中搜索含有指定值的key和值

    127.0.0.1:6379> search test* = kris
    1) "test3"
    2) "kris"

~ 模糊搜索: 搜索含有指定值的key及其值

    127.0.0.1:6379> search test* ~ is
    1) "test2"
    2) "this is article"
    3) "test1"
    4) "this is testing"

~| 包含并截取: 搜索含有指定值的key及其值，如果值过长
