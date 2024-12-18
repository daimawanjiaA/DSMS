/**
 * (09_main)客户机
 * wfl:2024-4-6
*/
// 客户机
// 主函数
#include <unistd.h>
#include <lib_acl.h> 
#include <lib_acl.hpp>
#include "types.hpp"
#include "client.hpp"

void usage(char const* cmd){
    // ./client 127.0.0.1:21000 groups
    fprintf(stderr,"Group:    %s <taddrs> groups\n",cmd);
    // ./client 127.0.0.1:21000 upload tnvideo tnv001 ~/Videos/001.mp4 -> 00a...eb8
    fprintf(stderr,"Upload:   %s <taddrs> upload <appid> <userid> <filepath>\n",cmd);
    // ./client 127.0.0.1:21000 filesize tnvideo tnv001 00a...eb8
    fprintf(stderr,"Filesize: %s <taddrs> filesize <appid> <userid> <fileid>\n",cmd);
    // ./client 127.0.0.1:21000 download tnvideo tnv001 00a...eb8 1024 2048
    fprintf(stderr,"Download: %s <taddrs> download <appid> <userid> <fileid> <offset> <size>\n",cmd);
    // ./client 127.0.0.1:21000 delete tnvideo tnv001 00a...eb8 
    fprintf(stderr,"Delete:   %s <taddrs> delete <appid> <userid> <fileid>\n",cmd);
}

// 根据用户ID生成文件ID
std::string genfileid(char const* userid){
    srand(time(nullptr));
    struct timeval now;
    gettimeofday(&now,nullptr);
    acl::string str;
    str.format("%s@%d%lx@%d",userid,getpid(),acl_pthread_self(),rand());

    acl::md5 md5;
    md5.update(str.c_str(),str.size()); // 将str给md5
    md5.finish();                       // 计算str的MD5值
    char buf[33]={};
    strncpy(buf,md5.get_string(),32);   // 32位的MD5字符值赋给buf
    memmove(buf,buf+8,16);              // 将buf的后16位移到前16位，表示不要前面的8位
    memset(buf+16,0,16);                // 将buf的后16位清零，这两句表示只要中间的8位
    static int count=0;
    if(count>=8000) count=0;            // 限制count的范围：0-7999
    acl::string fileid;
    fileid.format("%08lx%06lx%s%04d%02d",now.tv_sec,now.tv_usec,buf,++count,rand()%100);
    return fileid.c_str();
}

int main(int argc,char *argv[]){
    char const* cmd=argv[0];
    if(argc<3){
        usage(cmd);
        return -1;
    }
    char const* taddrs=argv[1];
    char const* subcmd=argv[2];
    // 初始化ACL库
    acl::acl_cpp_init();
    // 日志打印到标准输出
    acl::log::stdout_open(true);
    // 初始化客户机
    if(client_c::init(taddrs)!=OK) return -1;
    // 客户机对象
    client_c client;
    // 从跟踪服务器获取组列表
    if(!strcmp(subcmd,"groups")){
        std::string groups;
        if(client.groups(groups)!=OK){
            client_c::deinit();
            return -1;
        }
        printf("%s\n",groups.c_str());
    }
    // 向存储服务器上传文件
    else if(!strcmp(subcmd,"upload")){
        if(argc<6){
            client_c::deinit();
            usage(cmd);
            return -1;
        }
        char const* appid   =argv[3];
        char const* userid  =argv[4];
        std::string fileid  =genfileid(userid);
        char const* filepath=argv[5];
        if(client.upload(appid,userid,fileid.c_str(),filepath)!=OK){
            client_c::deinit();
            return -1;
        }
        printf("Upload success: %s.\n",fileid.c_str());
    }
    // 向存储服务器询问文件大小
    else if(!strcmp(subcmd,"filesize")){
        if(argc<6){
            client_c::deinit();
            usage(cmd);
            return -1;
        }
        char const* appid   =argv[3];
        char const* userid  =argv[4];
        char const* fileid  =argv[5];
        long long   filesize=0;
        if(client.filesize(appid,userid,fileid,&filesize)!=OK){
            client_c::deinit();
            return -1;
        }
        printf("Get filesize success: %lld.\n",filesize);
    }
    // 从存储服务器下载文件
    else if(!strcmp(subcmd,"download")){
        if(argc<8){
            client_c::deinit();
            usage(cmd);
            return -1;
        }
        char const* appid   =argv[3];
        char const* userid  =argv[4];
        char const* fileid  =argv[5];
        long long   offset  =atoll(argv[6]);
        long long   size    =atoll(argv[7]);
        char*       filedata=nullptr;
        long long   filesize=0;
        if(client.download(appid,userid,fileid,offset,size,&filedata,&filesize)!=OK){
            client_c::deinit();
            return -1;
        }
        printf("Download success: %lld.\n",filesize);
        free(filedata);
    }
    // 删除存储服务器上的文件
    else if(!strcmp(subcmd,"delete")){
        if(argc<6){
            client_c::deinit();
            usage(cmd);
            return -1;
        }
        char const* appid   =argv[3];
        char const* userid  =argv[4];
        char const* fileid  =argv[5];
        if(client.del(appid,userid,fileid)!=OK){
            client_c::deinit();
            return -1;
        }
        printf("Delete success: %s.\n",fileid);
    }
    else{
        client_c::deinit();
        usage(cmd);
        return -1;
    }
    // 终结化客户机
    client_c::deinit();
    return 0;
}