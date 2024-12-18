/**
 * (13_main)主函数
 * wfl:2024-4-1
*/
// 定义主函数
#include "globals.hpp"
#include "server.hpp"

int main(void){
    // 初始化ACL库
    acl::acl_cpp_init();
    // 设置日志输出到标准输出
    acl::log::stdout_open(true);
 
    // 创建并运行服务器
    server_c& server=acl::singleton2<server_c>::get_instance();
    // 设置配置信息，将配置信息保存到全局变量中
    server.set_cfg_str(cfg_str);
    server.set_cfg_int(cfg_int);
    // 启动服务器，参数为监听地址和配置文件路径
    server.run_alone("127.0.0.1:21000","../etc/tracker.cfg");
    return 0;
}