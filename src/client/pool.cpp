/**
 * (04_pool)客户机
 * wfl:2024-4-6
*/
// 客户机
// 定义连接池类
#include "conn.hpp"
#include "pool.hpp"
 
pool_c::pool_c(char const* destaddr,int count,size_t index):connect_pool(destaddr,count,index),m_ctimeout(30),m_rtimeout(60),m_itimeout(90){

}
// 设置超时
void pool_c::timeouts(int ctimeout,int rtimeout,int itimeout){
    m_ctimeout=ctimeout;
    m_rtimeout=rtimeout;
    m_itimeout=itimeout;
}
// 获取连接
acl::connect_client* pool_c::peek(void){
    connect_pool::check_idle(m_itimeout);   // 检查空闲连接，释放空闲连接
    return connect_pool::peek();
}
// 创建连接
acl::connect_client* pool_c::create_connect(void){
    return new conn_c(addr_,m_ctimeout,m_rtimeout); // addr_是父类的成员变量，表示连接地址
}