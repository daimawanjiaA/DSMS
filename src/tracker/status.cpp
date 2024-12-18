/**
 * (10_status)跟踪服务器服务类
 * wfl:2024-3-31
*/
// 实现存储服务器状态检查线程类
#include <unistd.h>
#include "globals.hpp"
#include "status.hpp"

 // 构造函数
status_c::status_c(void): m_stop(false){
}
// 终止线程 
void status_c::stop(void){
    m_stop=true;
}

// 线程过程
void* status_c::run(void){
    for(time_t last=time(nullptr);!m_stop;sleep(1)){
        // 现在
        time_t now=time(nullptr);
        // 若现在距离最近一次检查存储服务器状态已足够久
        if(now-last>=cfg_interval){
            // 检查存储服务器状态
            check();
            // 更新最近一次检查时间
            last=now;
        }
    }
    return nullptr;
}

// 检查存储服务器状态
int status_c::check(void) const{
    // 现在
    time_t now=time(nullptr);
    // 互斥锁加锁
    if((errno=pthread_mutex_lock(&g_mutex))){
        logger_error("call pthread_mutex_lock fail: %s.",strerror(errno));
        return ERROR;
    }
    // 遍历组表中的每一组
    for (std::map<std::string,std::list<storage_info_t>>::iterator group = g_groups.begin(); group!=g_groups.end(); ++group){
        // 遍历该组中的每一台存储服务器
        for(std::list<storage_info_t>::iterator si=group->second.begin();si!=group->second.end();++si){
            //若该存储服务器心跳停止太久
            if(now-si->si_btime>=cfg_interval)
                //则将其状态标记为离线
                si->si_status=STORAGE_STATUS_OFFLINE;
        }
    }
    //互斥锁解锁
    if((errno=pthread_mutex_unlock(&g_mutex))){
        logger_error("call pthread_mutex_unlock fail: %s.",strerror(errno));
        return ERROR;
    }
    return OK;
}
