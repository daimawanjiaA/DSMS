/**
 * (05_db)储存服务器数据库类
 * wfl:2024-4-3
*/
// 存储服务器
// 声明数据库访问类
#pragma once

#include <string>
#include <vector>
#include <mysql.h>

// 数据库访问类
class db_c
{
public: 
    db_c();
    ~db_c();
    // 连接数据库
    int connect(void);
    // 根据文件ID获取其路径及大小
    int get(char const* appid,char const* userid,char const* fileid,std::string &filepath,long long *filesize) const;
    // 设置文件ID和路径及大小的对应关系
    int set(char const* appid,char const*userid,char const* fileid,char const *filepath,long long filesize) const;
    // 删除文件ID
    int del(char const* appid,char const* userid,char const* fileid) const;
private:
    // 根据用户名来获取其对应的表名
    std::string table_of_user(char const* userid) const;
    // 计算哈希值
    unsigned int hash(char const* buf,size_t len) const;
    MYSQL* m_mysql; // MySQL对象
};

