/**
 * (13_service)存储服务器线程类
 * wfl:2024-4-5
*/
// 存储服务器
// 声明跟踪客户机线程类
#pragma once

#include <lib_acl.hpp>

// 跟踪客户机线程类
class tracker_c: public acl::thread
{
public:
    tracker_c(char const* taddr);
    // 终止线程
    void stop(void); 
protected:
    // 线程过程
    void *run(void);
private:
    // 向跟踪服务器发送加入包
    int join(acl::socket_stream* conn) const;
    // 向跟踪服务器发送心跳包
    int beat(acl::socket_stream* conn) const;
    
private:
    // 是否终止
    bool m_stop;
    // 跟踪服务器地址
    acl::string m_taddr;
};
