## HMSELECT pattern field pattern [field pattern ...] 

> 时间复杂度：O(N)，N 是数据库中 key 的数量，假设数据库中 key 的名称、field 对应的 value 和指令中给出的 pattern 长度有限。  
  
返回所有同时满足以下条件的 key：  

1. 名称模糊匹配指令中第一个 pattern；
2. key 对应的 value 是一个包含指令中所有 field 的哈希对象，且每个 field 对应的 value 模糊匹配指令中紧接在该 field 后的 pattern.

虽然该指令的时间复杂度为 O(N)，常数时间仍然是比较低的。例如，在一台入门级计算机上运行的 Redis 可以在 40ms 内完成数据库内一百万条 key 的扫描。  
  
警告：HSELECT 应该只在开发环境下谨慎地使用。当数据库很大时，使用这个指令会影响性能。  
  
指令中使用的 pattern 是 glob 风格的：  

* h?llo matches hello, hallo and hxllo
* h*llo matches hllo and heeeello
* h[ae]llo matches hello and hallo, but not hillo
* h[^e]llo matches hallo, hbllo, ... but not helloP
* h[a-b]llo matches hallo and hbllo

如果要逐字匹配，请使用 \ 转义特殊字符。

### 返回值

返回 1 个数组：符合条件的 key 列表。

### 示例

redis>  HMSET hash1 field1 "one" field2 "two"  
  
OK  
  
redis>  HSET hash2 field1 "one"  
  
(integer) 1  
  
redis>  HMSELECT hash* field1 o* field2 *o  
  
1) "hash1"  
  
redis>

### 源码修改

#### src/server.c

在 redisCommandTable 中加入 hmselect 条目（条目中第 3 项为 -4，表示指令的参数数量 >= 4）：

    struct redisCommand redisCommandTable[] = {
        ...
        {"hmselect",hmselectCommand,-4,"rS",0,NULL,1,1,1,0,0},
        ...
    };

#### src/server.h

在 Commands prototypes 部分加入 hmselectCommand 函数的声明：

    /* Commands prototypes */
    ...
    void hmselectCommand(client *c);
    ...

#### src/t_hash.c

在末尾加入 hmselectCommand 函数：

    void hmselectCommand(client *c) {
        if ((c->argc % 2) == 1) {
            addReplyError(c,"wrong number of arguments for HMSELECT");
            return;
        }

        dictIterator *di;
        dictEntry *de;
        sds pattern = c->argv[1]->ptr, hpattern;
        int i, plen = sdslen(pattern), allkeys, hplen, anyhval;
        unsigned long numkeys = 0;
        void *replylen = addDeferredMultiBulkLength(c);

        di = dictGetSafeIterator(c->db->dict);
        allkeys = (pattern[0] == '*' && pattern[1] == '\0');
        while((de = dictNext(di)) != NULL) {
            sds key = dictGetKey(de);
            robj *keyobj;

            if (allkeys || stringmatchlen(pattern,plen,key,sdslen(key),0)) {
                keyobj = createStringObject(key,sdslen(key));
                if (expireIfNeeded(c->db,keyobj) == 0) {
                    robj *valobj = dictGetVal(de);
                    if (valobj->type == OBJ_HASH) {
                        for (i = 2; ; i += 2) {
                            if (hashTypeExists(valobj,c->argv[i])) {
                                robj *hvalobj;
                                if ((hvalobj = hashTypeGetObject(valobj, c->argv[i])) != NULL) {
                                    sds hval = hvalobj->ptr;
                                    hpattern = c->argv[i+1]->ptr;
                                    hplen = sdslen(hpattern);
                                    anyhval = (hpattern[0] == '*' && hpattern[1] == '\0');
                                    if (anyhval || stringmatchlen(hpattern,hplen,hval,sdslen(hval),0)) {
                                        if (i + 2 < c->argc) {
                                            decrRefCount(hvalobj);
                                            continue;
                                        } else {
                                            addReplyBulk(c,keyobj);
                                            numkeys++;
                                        }
                                    }
                                    decrRefCount(hvalobj);
                                }
                            }
                            break;
                        }
                    }
                }
                decrRefCount(keyobj);
            }
        }
        dictReleaseIterator(di);
        setDeferredMultiBulkLength(c,replylen,numkeys);
    }