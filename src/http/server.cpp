/**
 * (12_server)http服务器服务类
 * wfl:2024-4-12
*/
// 声明服务器类
#include "types.hpp"
#include "client.hpp" 
#include "globals.hpp"
#include "service.hpp"
#include "server.hpp"

// 进程启动是被调用
void server_c::proc_on_init(void){
    // 初始化客户机
    client_c::init(cfg_taddrs);
    // 创建并初始化redis集群
    m_redis=new acl::redis_client_cluster;
    m_redis->init(nullptr,cfg_raddrs,cfg_maxthrds,cfg_ctimeout,cfg_rtimeout);
    // 打印配置信息
    logger("cfg_taddrs: %s, cfg_raddrs: %s, cfg_maxthrds: %d, cfg_ctimeout: %d cfg_rtimeout: %d, cfg_rsession: %d.",cfg_taddrs,cfg_raddrs,cfg_maxthrds,cfg_ctimeout,cfg_rtimeout,cfg_rsession);
}
// 子进程意图退出时被调用
// 返回true，进程立即退出，否则，若配置ioctl_quick_abort非0，进程立即退出，否则待所有客户机连接都关闭后，进程再退出
bool server_c::proc_exit_timer(size_t nclients,size_t nthreads){
    // 终止存储服务器状态检查线程
    if(!nclients||!nthreads){
        logger("nclient: %lu, nthread: %lu.",nclients,nthreads);
        return true;
    }
    return false;
}
// 进程临退出被调用
void server_c::proc_on_exit(void){
    delete m_redis;
    // 终结化客户机
    client_c::deinit();
}

// 线程获得连接时被调用
bool server_c::thread_on_accept(acl::socket_stream* conn){
    logger("connect, from: %s.",conn->get_peer());
    // 设置读写超时
    conn->set_rw_timeout(cfg_rtimeout);
    // 创建会话，根据配置选择redis集群或memcache本地的缓存会话
    acl::session* session=cfg_rsession?(acl::session*)new acl::redis_session(*m_redis,cfg_maxthrds):(acl::session*)new acl::memcache_session("127.0.0.1:11211");
    // 创建并设置业务服务对象
    conn->set_ctx(new service_c(conn,session));
    return true;
}
// 与线程绑定的连接可读时被调用
bool server_c::thread_on_read(acl::socket_stream* conn){
    service_c* service=(service_c*) conn->get_ctx();    // 获得连接节点所绑定的对象地址
    if(!service) logger_fatal("sevice is null.");
    return service->doRun();    // 调用业务服务对象的处理函数，内部是调用的doGet、doPost等函数
}
// 线程读写连接超时时被调用
bool server_c::thread_on_timeout(acl::socket_stream* conn){
    logger("read timeout, from: %s.",conn->get_peer());
    return false;
}
// 以上三个函数返回true连接将被保持，否则断开连接
// 与线程绑定的连接即将关闭时被调用
void server_c::thread_on_close(acl::socket_stream* conn){
    logger("client disconnect, from: %s.",conn->get_peer());
    service_c* service=(service_c*)conn->get_ctx(); // 获得连接节点所绑定的对象地址
    acl::session* session=&service->getSession();   // 获得业务服务对象的会话对象
    delete session;
    delete service;
}