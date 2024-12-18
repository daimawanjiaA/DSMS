/**
 * (05_pool)客户机
 * wfl:2024-4-6
*/
// 客户机
// 声明连接池管理器类
#pragma once
#include <lib_acl.hpp>
 
class mngr_c:public acl::connect_manager
{
protected:
    // 创建连接池 
    acl::connect_pool* create_pool(char const* destaddr,size_t count,size_t index);
};
