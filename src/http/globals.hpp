/**
 * (02_globals)声明全局变量
 * wfl:2024-4-6
*/
// HTTP服务器
// 声明全局变量
#pragma once

#include <lib_acl.hpp>

// 配置信息
extern char *cfg_taddrs;                // 跟踪服务器地址列表
extern char *cfg_raddrs;                // Redis地址表
extern acl::master_str_tbl cfg_str[];   // 字符串配置表
 
extern int cfg_maxthrds;                // Redis最大线程数
extern int cfg_ctimeout;                // Redis连接超时
extern int cfg_rtimeout;                // Redis读写超时
extern acl::master_int_tbl cfg_int[];   // 整形配置表

extern int cfg_rsession;                // 是否使用Redis会话
extern acl::master_bool_tbl cfg_bool[]; // 布尔类型配置表
