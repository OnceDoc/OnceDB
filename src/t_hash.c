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
