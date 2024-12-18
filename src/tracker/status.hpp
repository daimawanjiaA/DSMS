/**
 * (09_status)跟踪服务器服务类
 * wfl:2024-3-31
*/
// 声明存储服务器状态检查线程类
#pragma once

#include <lib_acl.hpp>

// 存储服务器状态检查线程类
class status_c: public acl::thread
{
public:
    // 构造函数 
    status_c(void);
    // 终止线程
    void stop(void);
protected:
    // 线程过程
    void * run(void);
private:
    // 检查存储服务器状态
    int check(void) const;
    // 是否终止
    bool m_stop;
};

