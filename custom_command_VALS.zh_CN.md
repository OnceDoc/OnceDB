## 修改 Redis 源码，创建查找值符合给定模式的键值对的新命令 VALS

Redis 是一个使用 C 语言编写的键值对存储数据库。Redis 自带的命令不提供查找值符合给定模式的键的功能。因此，如果要实现这一功能，我们需要创建一个新命令。  
  
我们希望创建的这一命令与 Redis 自带的 KEYS 命令非常相似，功能都是根据给定模式查找相应的键或键值对，最大的不同是新命令用键值对中的值来匹配给定模式，而 KEYS 命令用键值对中的键进行匹配。参考 KEYS 命令。我们可以将新命令命名为 VALS，将这一命令的基本语法设置为：

    VALS pattern

明确了要创建的命令的名称和语法后，我们就可以开始修改 Redis 源码了。

#### 第一步、在 redisCommandTable 数组中加入新条目

如果修改的是 3.2 版的 Redis，redisCommandTable 数组在 src/server.c 文件里；如果是 3.0 版的，数组在 src/redis.c 文件里。  
  
Redis 支持的所有命令都存储在 redisCommandTable 数组中：

    struct redisCommand redisCommandTable[] = {
        {"get",getCommand,2,"rF",0,NULL,1,1,1,0,0},
        {"set",setCommand,-3,"wm",0,NULL,1,1,1,0,0},
        ...

数组中的每个元素都是一个 redisCommand 结构体，其中记录了关于一条命令的详尽信息，在数组中加入 vals 命令（推荐和功能相似的 keys 命令放在一起）：

        {"vals",valsCommand,2,"rS",0,NULL,0,0,0,0,0},  

命令条目中各个项的意义分别为：  

* 命令的名字："vals"
* 命令的实现函数：valsCommand
* 参数的数量：2
* 字符串形式的 FLAG："rS"（r 表示命令是读命令，不会修改 key space；S 表示如果命令在 Lua 脚本中执行，那么会对输出进行排序以得出确定的输出）
* 位掩码形式的 FLAG：0
*  一个可选的函数，用于从命令中取出 key 参数：NULL
* 第一个 key 参数的位置：0
* 最后一个 key 参数的位置：0
* 从 first 参数和 last 参数之间，所有 key 的步数：0
* 执行这个命令耗费的总微秒数：0
* 命令被执行的总次数：0

#### 第二步、声明命令的实现函数  

在 Redis 3.2 的 src/server.h 或 Redis 3.0 的 src/redis.h 中加入 vals 命令的实现函数 valsCommand 的声明（推荐和功能相似的 keys 命令放在一起）： 

    void valsCommand(client *c);

#### 第三步、在 KEYS 命令的实现函数所在 的 db.c 中加入命令的实现函数 

valsCommand 函数的具体代码如下：

    void valsCommand(client *c) {
        dictIterator *di;
        dictEntry *de;
        sds pattern = c->argv[1]->ptr;
        int plen = sdslen(pattern), allvals;
        unsigned long numreps = 0;
        void *replylen = addDeferredMultiBulkLength(c);

        // 遍历整个数据库，返回值和模式匹配的键值对
        di = dictGetSafeIterator(c->db->dict);
        allvals = (pattern[0] == '*' && pattern[1] == '\0');
        while((de = dictNext(di)) != NULL) {
            sds key = dictGetKey(de);
            robj *keyobj = createStringObject(key,sdslen(key));

            // 只处理未过期的键值对
            if (expireIfNeeded(c->db,keyobj) == 0) {
                robj *valobj = dictGetVal(de);

                // 只处理值类型为字符串的键值对
                if (valobj->type == OBJ_STRING) {
                    sds val = valobj->ptr;

                    // 将值和模式进行比对
                    if (allvals || stringmatchlen(pattern,plen,val,sdslen(val),0)) {
                        addReplyBulk(c,keyobj);
                        addReplyBulk(c,valobj);
                        numreps += 2;
                    }
                }
            }
            decrRefCount(keyobj);
        }
        dictReleaseIterator(di);
        setDeferredMultiBulkLength(c,replylen,numreps);
    }

#### 第四步、测试

重新编译（make）后，运行服务器端 Redis（src/redis-server），再在另一个终端中运行客户端 Redis（src/redis-cli），就可以使用新命令 VALS 了。  
  
VALS 命令使用示例：  

    redis> MSET key1 one key2 two key3 three key4 four
    "OK"
    redis> VALS *o*
    1) "key2"
    2) "two"
    3) "key1"
    4) "one"
    5) "key4"
    6) "four"
    redis> VALS t??
    1) "key2"
    2) "two"
    redis> VALS *
    1) "key2"
    2) "two"
    3) "key3"
    4) "three"
    5) "key1"
    6) "one"
    7) "key4"
    8) "four"
    redis> 

