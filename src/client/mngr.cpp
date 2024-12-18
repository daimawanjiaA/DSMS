/**
 * (06_pool)客户机
 * wfl:2024-4-6
*/
// 客户机
// 声明连接池管理器类
#include "pool.hpp"
#include "mngr.hpp"
 
// 创建连接池 
acl::connect_pool* mngr_c::create_pool(char const* destaddr,size_t count,size_t index){
    return new pool_c(destaddr,count,index);
}
