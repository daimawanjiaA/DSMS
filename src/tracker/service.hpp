/**
 * (07_service)跟踪服务器服务类
 * wfl:2024-3-30
*/
// 跟踪服务器
// 定义业务服务类
#pragma once

#include <lib_acl.hpp>
#include "types.hpp"

// 业务服务类
class service_c
{
public:
    // 业务处理
    bool business(acl::socket_stream *conn,char const* head) const;
private: 
    // 第一层函数start
    // 处理来自存储服务器的加入包
    bool join(acl::socket_stream* conn, long long bodylen) const;
    // 处理来自存储服务器的心跳包
    bool beat(acl::socket_stream* conn, long long bodylen) const;
    // 处理来自客户机的获取存储服务器地址列表请求
    bool saddrs(acl::socket_stream* conn,long long bodylen) const;
    // 处理来自客户机的获取组列表请求
    bool groups(acl::socket_stream* conn) const;
    // 第一层函数end

    // 第二层函数start（是对第一层函数的辅助）
    // 将存储服务器加入组表
    int join(storage_join_t const* sj, char const* saddr) const;
    // 将存储服务器标为活动
    int beat(char const* groupname,char const* hostname,char const* saddr) const;
    // 第二层函数end

    // 第三层函数start（是对第二层函数的辅助）
    // 响应客户机存储服务器地址列表
    int saddrs(acl::socket_stream* conn,char const* appid,char const* userid) const;
    // 根据用户ID获取对应的组名
    int group_of_user(char const* appid,char const* userid,std::string& groupname) const;
    // 根据组名获取存储服务器地址列表
    int saddrs_of_group(char const* groupname,std::string& saddrs) const;
    // 第三层函数end

    // 应答成功
    bool ok(acl::socket_stream* conn) const;
    // 应答错误
    bool error(acl::socket_stream* conn,short errnumb,char const* format,...) const;
};
