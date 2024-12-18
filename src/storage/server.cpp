/**
 * (16_server)存储服务器服务类
 * wfl:2024-4-5
*/
// 声明服务器类
#include <unistd.h>
#include "proto.hpp"
#include "util.hpp"
#include "globals.hpp"
#include "service.hpp"
#include "server.hpp"

// 进程启动是被调用 
void server_c::proc_on_init(void){
    // 隶属组名
    if(strlen(cfg_gpname)>STORAGE_GROUPNAME_MAX) logger_fatal("groupname too big %lu > %d.",strlen(cfg_gpname),STORAGE_GROUPNAME_MAX);
    // 绑定端口号
    if(cfg_bindport<=0) logger_fatal("invalid bind port %d <= 0.",cfg_bindport);
    // 存储路径表
    if(!cfg_spaths||!strlen(cfg_gpname)) logger_fatal("storage paths is null.");
    split(cfg_spaths,g_spaths);
    if(g_spaths.empty()) logger_fatal("storage paths is null.");
    // 跟踪服务器地址表
    if(!cfg_taddrs||!strlen(cfg_taddrs)) logger_fatal("tracker address is null.");
    split(cfg_taddrs,g_taddrs);
    if(g_taddrs.empty()) logger_fatal("tracker address is null.");
    // ID服务器地址表
    if(!cfg_iaddrs||!strlen(cfg_iaddrs)) logger_fatal("id address is null.");
    split(cfg_iaddrs,g_iaddrs);
    if(g_iaddrs.empty()) logger_fatal("id address is null.");
 
    // MySQL地址表
    if(!cfg_maddrs||!strlen(cfg_maddrs))  logger_fatal("mysql addresses is null.");
    split(cfg_maddrs,g_maddrs);
    if(g_maddrs.empty())  logger_fatal("mysql addresses is empty.");
    // Redis地址表
    if(!cfg_raddrs||!strlen(cfg_raddrs))  logger_fatal("redis addresses is null.");
    else{
        split(cfg_raddrs,g_raddrs);
        if(g_raddrs.empty()) logger_error("redis addresses is empty.");
        // 遍历Redis地址表，尝试创建连接池
        for(std::vector<std::string>::const_iterator raddr=g_raddrs.begin();raddr!=g_raddrs.end();++raddr){
            if((g_rconns=new acl::redis_client_pool(raddr->c_str(),cfg_maxconns))){
                // 设置Redis连接超时和读写超时
                g_rconns->set_timeout(cfg_ctimeout,cfg_rtimeout);
                break;
            }
        }
        if(!g_rconns){
            logger_error("create redis connection pool fail, cfg_raddrs: %s",cfg_raddrs);
        }
    }
    // 主机名
    char hostname[256+1]={};
    if(gethostname(hostname,sizeof(hostname)-1)) logger_error("call gethostname fail: %s",strerror(errno));
    g_hostname=hostname;
    // 启动时间
    g_stime=time(nullptr);
    // 创建并启动连接每台跟踪服务器的客户机线程
    for(auto taddr=g_taddrs.begin();taddr!=g_taddrs.end();++taddr){
        tracker_c* tracker=new tracker_c(taddr->c_str());
        tracker->set_detachable(false); // 将tracker线程设置为非分离状态，以便等待线程结束（可汇合线程）
        tracker->start();
        m_trackers.push_back(tracker);
    }
    // 打印配置信息
    logger("cfg_gpname: %s, cfg_spaths: %s, cfg_taddrs: %s, cfg_iaddrs: %s, cfg_maddrs: %s, cfg_raddrs: %s, cfg_bindport: %d, cfg_interval: %d, cfg_mtimeout: %d, cfg_maxconns: %d, cfg_ctimeout: %d, cfg_rtimeout: %d, cfg_ktimeout: %d.",cfg_gpname,cfg_spaths,cfg_taddrs,cfg_iaddrs,cfg_maddrs,cfg_raddrs,cfg_bindport,cfg_interval,cfg_mtimeout,cfg_maxconns,cfg_ctimeout,cfg_rtimeout,cfg_ktimeout);
}
// 子进程意图退出时被调用
// 返回true，进程立即退出，否则，若配置ioctl_quick_abort非0，进程立即退出，否则待所有客户机连接都关闭后，进程再退出
bool server_c::proc_exit_timer(size_t nclients,size_t nthreads){
    // 终止跟踪客户机线程
    for(auto tracker=m_trackers.begin();tracker!=m_trackers.end();++tracker){
        (*tracker)->stop();
    }
    if(!nclients||!nthreads){
        logger("nclient: %lu, nthread: %lu.",nclients,nthreads);
        return true;
    }
    return false;
}
// 进程临退出被调用
void server_c::proc_on_exit(void){
    // 回收跟踪客户机线程
    for(auto tracker=m_trackers.begin();tracker!=m_trackers.end();++tracker){
        if((*tracker)->wait(nullptr))
            logger_error("wait thread #%lu fail.",(*tracker)->thread_id());
        // 销毁跟踪客户机线程
        delete *tracker;
    }
    m_trackers.clear();
    
    // 销毁Redis连接池
    if(g_rconns){
        delete g_rconns;
        g_rconns=nullptr;
    }
}

// 线程获得连接时被调用
bool server_c::thread_on_accept(acl::socket_stream* conn){
    logger("connect, from: %s.",conn->get_peer());
    return true;
}
// 与线程绑定的连接可读时被调用
bool server_c::thread_on_read(acl::socket_stream* conn){
    // 接收包头
    char head[HEADLEN];
    if(conn->read(head,HEADLEN)<0){ //  conn->eof()表示连接已关闭
        if(conn->eof()) logger("connection has been closed, from: %s.",conn->get_peer());
        else logger_error("read fail: %s, from: %s.",acl::last_serror(),conn->get_peer());
        return false;
    }
    // 业务处理
    service_c service;
    return service.business(conn,head);
}
// 线程读写连接超时时被调用
bool server_c::thread_on_timeout(acl::socket_stream* conn){
    logger("read timeout, from: %s.",conn->get_peer());
    return true;
}
// 以上三个函数返回true连接将被保持，否则断开连接
// 与线程绑定的连接即将关闭时被调用
void server_c::thread_on_close(acl::socket_stream* conn){
    logger("client disconnect, from: %s.",conn->get_peer());
}