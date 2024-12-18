/**
 * (03_globals)HTTP全局变量
 * wfl:2024-4-6
*/
// HTTP服务器
// 定义全局变量
#include "globals.hpp"

// 配置信息
char *cfg_taddrs;                // 跟踪服务器地址列表
char *cfg_raddrs;                // Redis地址表
acl::master_str_tbl cfg_str[]={  // 字符串配置表
    {"tnv_tracker_addrs",   "127.0.0.1:21000", &cfg_taddrs},
    {"redis_addrs",         "127.0.0.1:6379",  &cfg_raddrs},
    {0,0,0}
}; 

int cfg_maxthrds;                // Redis最大线程数
int cfg_ctimeout;                // Redis连接超时
int cfg_rtimeout;                // Redis读写超时
acl::master_int_tbl cfg_int[]={  // 整形配置表
    {"ioctl_max_threads",   250, &cfg_maxthrds, 0, 0},
    {"redis_conn_timeout",  10,  &cfg_ctimeout, 0, 0},
    {"redis_rw_timeout",    10,  &cfg_rtimeout, 0, 0},
    {0,0,0,0,0}
};

int cfg_rsession;                // 是否使用Redis会话
acl::master_bool_tbl cfg_bool[]={ // bool形配置表
    {"use_redis_session", 1, &cfg_rsession},
    {0,0,0}
};
