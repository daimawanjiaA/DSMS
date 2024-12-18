/**
 * (03_db)ID服务器数据库类
 * wfl:2024-4-1
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
    // 获取ID当前值，同时产生下一个值
    int get(char const* key,int inc,long *value) const;
private:
    MYSQL* m_mysql; // MySQL对象
};

