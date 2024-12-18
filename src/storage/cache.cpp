/**
 * (04_cache)存储服务器类
 * wfl:2024-4-2
*/
#include "types.hpp"
#include "globals.hpp"
#include "cache.hpp"
 
// 根据键获取其值
int cache_c::get(char const* key,acl::string& value){
    // 构造键
    acl::string storage_key;
    storage_key.format("%s:%s",STORAGE_REDIS_PREFIX,key);
    // 检查redis连接池
    if(!g_rconns){
        logger_warn("Redis conntion pool is null, key: %s.",storage_key.c_str());
        return ERROR;
    }
    // 从连接池中获取一个Redis连接
    acl::redis_client* rconn=(acl::redis_client*) g_rconns->peek();
    if(!rconn){
        logger_warn("Peek redis conntion fail, key: %s.",storage_key.c_str());
        return ERROR;
    }
    // 持有此连接的Redis对象即为redis客户机
    acl::redis redis;
    redis.set_client(rconn);
    // 借助redis客户机根据键获取其值
    if(!redis.get(storage_key.c_str(),value)){
        logger_warn("get cache fail, key: %s",storage_key.c_str());
        g_rconns->put(rconn,false);
        return ERROR;
    }
    // 检查空值
    if(value.empty()){
        logger_warn("value is empty, key: %s",storage_key.c_str());
        g_rconns->put(rconn, false);
        return ERROR;
    }
    logger("get cache ok, key: %s, value: %s",storage_key.c_str(),value.c_str());
    g_rconns->put(rconn,true);
    return OK;
}
// 设置指定键的值
int cache_c::set(char const* key,char const* value,int timeout) const{
    // redis.setex(tracker_key.c_str(),value,timeout);
    // 构造键
    acl::string storage_key;
    storage_key.format("%s:%s",STORAGE_REDIS_PREFIX,key);
    // 检查redis连接池
    if(!g_rconns){
        logger_warn("Redis conntion pool is null, key: %s",storage_key.c_str());
        return ERROR;
    }
    // 从连接池中获取一个Redis连接
    acl::redis_client* rconn=(acl::redis_client*) g_rconns->peek();
    if(!rconn){
        logger_warn("Peek redis conntion fail, key: %s",storage_key.c_str());
        return ERROR;
    }
    // 持有此连接的Redis对象即为redis客户机
    acl::redis redis;
    redis.set_client(rconn);
    // 借助redis客户机设置指定键的值
    if(timeout<0) timeout=cfg_ktimeout;
    if(!redis.setex(storage_key.c_str(),value,timeout)){
        logger_warn("set cache fail, key: %s,value: %s, timeout: %d.",storage_key.c_str(),value,timeout);
        g_rconns->put(rconn,false);
        return ERROR;
    }
    logger("set cache ok, key: %s, value: %s, timeout: %d.",storage_key.c_str(),value,timeout);
    g_rconns->put(rconn,true);

    return OK;
}
// 删除指定键值对
int cache_c::del(char const* key) const{
    // redis.del_one(racker_key.c_str());
    // 构造键
    acl::string storage_key;
    storage_key.format("%s:%s",STORAGE_REDIS_PREFIX,key);
    // 检查redis连接池
    if(!g_rconns){
        logger_warn("Redis conntion pool is null, key: %s",storage_key.c_str());
        return ERROR;
    }
    // 从连接池中获取一个Redis连接
    acl::redis_client* rconn=(acl::redis_client*) g_rconns->peek();
    if(!rconn){
        logger_warn("Peek redis conntion fail, key: %s",storage_key.c_str());
        return ERROR;
    }
    // 持有此连接的Redis对象即为redis客户机
    acl::redis redis;
    redis.set_client(rconn);
    // 借助redis客户机删除指定的键值对
    if(!redis.del_one(storage_key.c_str())){
        logger_warn("delete cache fail, key: %s",storage_key.c_str());
        g_rconns->put(rconn,false);
        return ERROR;
    }
    logger("delete cache ok, key: %s",storage_key.c_str());
    g_rconns->put(rconn,true);
    return OK;
}