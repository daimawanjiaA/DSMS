/**
 * (05_db)跟踪服务器数据库类
 * wfl:2024-3-30
*/
// 跟踪服务器
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
    // 根据用户ID获取其对应的组名
    int get(char const* userid,std::string &groupname) const;
    // 设置用户ID和组名的对应关系
    int set(char const* appid,char const*userid,char const* groupname) const;
    // 获取全部组名
    int get(std::vector<std::string>& groupnames) const;
private:
    MYSQL* m_mysql; // MySQL对象
};

