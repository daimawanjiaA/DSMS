/**
 * (05_service)ID服务器服务类
 * wfl:2024-4-2
*/
// ID服务器
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
    // 处理来自存储服务器的获取ID请求
    bool get(acl::socket_stream* conn, long long bodylen) const;

    // 根据ID的键获取其值
    long get(char const* key) const;
    // 从数据库中获取ID值
    long fromdb(char const* key) const;

    // 应答成功
    bool id(acl::socket_stream* conn,long value) const;
    // 应答错误
    bool error(acl::socket_stream* conn,short errnumb,char const* format,...) const;
};
