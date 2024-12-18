/**
 * (15_server)存储服务器服务类
 * wfl:2024-4-5
*/
#pragma once
// 声明服务器类
#include <list>
#include <lib_acl.hpp>
#include "tracker.hpp"
 
// 服务器类
class server_c: public acl::master_threads {
protected:
    // 进程启动是被调用
    void proc_on_init(void) override;
    // 子进程意图退出时被调用
    // 返回true，进程立即退出，否则，若配置ioctl_quick_abort非0，进程立即退出，否则待所有客户机连接都关闭后，进程再退出
    bool proc_exit_timer(size_t nclients,size_t nthreads) override;
    // 进程临退出被调用
    void proc_on_exit(void) override;

    // 线程获得连接时被调用
    bool thread_on_accept(acl::socket_stream* conn) override;
    // 与线程绑定的连接可读时被调用
    bool thread_on_read(acl::socket_stream* conn) override;
    // 线程读写连接超时时被调用
    bool thread_on_timeout(acl::socket_stream* conn) override;
    // 以上三个函数返回true连接将被保持，否则断开连接
    // 与线程绑定的连接即将关闭时被调用
    void thread_on_close(acl::socket_stream* conn) override;
private:
    // 跟踪客户机存储线程集
    std::list<tracker_c*> m_trackers;
};
