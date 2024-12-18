/**
 * (02_globals)声明全局变量
 * wfl:2024-4-6
 * change：2024-4-27
*/
// 跟踪服务器
// 声明全局变量
#pragma once

#include <vector>
#include <string>
#include <map>
#include <list>
#include <lib_acl.hpp>
#include "types.hpp"
 
// 配置信息
extern char *cfg_appids;                // 应用ID表
extern char *cfg_maddrs;                // MySQL地址表
extern char *cfg_raddrs;                // Redis地址表
extern acl::master_str_tbl cfg_str[];   // 字符串配置表

extern int cfg_interval;                // 存储服务器状态检查间隔秒数
extern int cfg_mtimeout;                // MySQL读写超时
extern int cfg_maxconns;                // Redis连接池最大连接数
extern int cfg_ctimeout;                // Redis连接超时
extern int cfg_rtimeout;                // Redis读写超时
extern int cfg_ktimeout;                // Redis连接超时（键超时是用来表示在redis构建键值对的是否所花费的时间不能过长）
extern acl::master_int_tbl cfg_int[];   // 整形配置表

extern std::vector<std::string> g_appids;   // 应用ID表
extern std::vector<std::string> g_maddrs;   // MySQL地址表
extern std::vector<std::string> g_raddrs;   // Redis地址表
extern acl::redis_client_pool*  g_rconns;   // Redis连接池
extern std::string            g_hostname;   // 主机名
// string <-> list<storage_info_t>的映射组表 多线程 map非线程安全
// 上面是只读，下面是可读可写
extern std::map<std::string,std::list<storage_info_t>> g_groups;    // 组表
extern pthread_mutex_t          g_mutex;    // 互斥锁
