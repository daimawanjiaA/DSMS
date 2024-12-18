/**
 * (06_db)跟踪服务器数据库类
 * wfl:2024-3-30
 * change: 2024-4-27
*/
// 跟踪服务器
// 实现数据库访问类
#include "db.hpp"
#include "cache.hpp"
#include "globals.hpp"

// 构造函数，创建MySQL对象
db_c::db_c() : m_mysql(mysql_init(nullptr)){
    if(!m_mysql) logger_error("create dao fail: %s.",mysql_error(m_mysql));
}
// 析构函数，销毁MySQL对象
db_c::~db_c(){ 
    if(m_mysql){
        mysql_close(m_mysql);
        m_mysql=nullptr;
    }
}
// 连接数据库
int db_c::connect(void){
    // 多增加一个mysql变量是为了避免m_mysql被修改
    MYSQL* mysql=m_mysql;
    for(std::vector<std::string>::const_iterator maddr=g_maddrs.begin();maddr!=g_maddrs.end();++maddr){
        if((m_mysql=mysql_real_connect(mysql,maddr->c_str(),"root","123456","tnv_trackerdb",0,nullptr,0))) return OK;
    }
    logger_error("connect database fail: %s",mysql_error(m_mysql=mysql));
    return ERROR;
}
// 根据用户ID获取其对应的组名
int db_c::get(char const* userid,std::string &groupname) const{
    // 先尝试从缓存中获取与用户ID对应的组名
    cache_c cache;
    acl::string key;
    // 这里采用userid:%s的格式作为key是为了避免与其他key冲突，userid在整个分布式可能不是唯一的，但是对userid这一类的key来说是唯一的
    key.format("userid:%s",userid);
    acl::string value;
    // 若缓存中想要的数据有则直接返回
    if(cache.get(key,value)==OK){
        groupname=value.c_str();
        return OK;
    }
    // 缓存中没有再查询数据库
    acl::string sql;
    sql.format("SELECT group_name FROM t_router WHERE userid='%s';",userid);
    if(mysql_query(m_mysql,sql.c_str())){
        logger_error("query database fail: %s, sql: %s.",mysql_error(m_mysql),sql.c_str());
        return ERROR;
    }
    // 获取查询结果
    MYSQL_RES* res=mysql_store_result(m_mysql);
    if(!res){
        logger_error("result is null: %s, sql: %s.",mysql_error(m_mysql),sql.c_str());
        return ERROR;
    }
    // 获取结果记录
    MYSQL_ROW row=mysql_fetch_row(res);
    if(!row){
        logger_warn("result is empty: %s, sql: %s.",mysql_error(m_mysql),sql.c_str());
    }else{
    // 将用户ID和组名的对应关系保存在缓存中
        groupname=row[0];
        cache.set(key,groupname.c_str());
    }
    return OK;
}
// 设置用户ID和组名的对应关系
int db_c::set(char const* appid,char const*userid,char const* groupname) const{
    // 插入一条记录
    acl::string sql;
    sql.format("INSERT INTO t_router SET appid='%s',userid='%s',group_name='%s';",appid,userid,groupname);
    if(mysql_query(m_mysql,sql.c_str())){
        logger_error("insert database fail: %s, sql: %s.",mysql_error(m_mysql),sql.c_str());
        return ERROR;
    }
    // 检查插入结果
    MYSQL_RES* res=mysql_store_result(m_mysql);
    // mysql_field_count表示返回结果的列数，如果返回0表示没有返回结果
    if(!res&&mysql_field_count(m_mysql)){
        logger_error("insert database fail: %s, sql: %s.",mysql_error(m_mysql),sql.c_str());
        return ERROR;
    }
    return OK;
}
// 获取全部组名
int db_c::get(std::vector<std::string>& groupnames) const{
    // 查询全部组名
    acl::string sql;
    sql.format("SELECT group_name FROM t_groups_info;");
    if(mysql_query(m_mysql,sql.c_str())){
        logger_error("query database fail: %s, sql: %s.",mysql_error(m_mysql),sql.c_str());
        return ERROR;
    }
    // 获取查询结果
    MYSQL_RES* res=mysql_store_result(m_mysql);
    if(!res){
        logger_error("result is null: %s, sql: %s",mysql_error(m_mysql),sql.c_str());
        return ERROR;
    }
    // 获取结果记录
    int nrows=mysql_num_rows(res);
    for(int i=0;i<nrows;++i){
        // mysql_fetch_row表示获取结果集的下一行
        MYSQL_ROW row=mysql_fetch_row(res);
            if(!row) break;
        groupnames.push_back(row[0]);
    }
    return OK;
}