/**
 * (12_service)存储服务器服务类
 * wfl:2024-4-4
 * change:2024-5-9
 */
// ID服务器
// 实现业务服务类
#include <linux/limits.h>
#include "service.hpp"
#include <algorithm>
#include "proto.hpp"
#include "util.hpp"
#include "globals.hpp"
#include "db.hpp"
#include "file.hpp"
#include "id.hpp"
 
// 业务处理
bool service_c::business(acl::socket_stream *conn,char const* head) const{
    // |包体长度|命令|状态|  包体  |
    // |   8   | 1 | 1 | 包体长度|
    // 解析包头: 包体长度、命令、状态
    long long bodylen=ntoll(head);
    if(bodylen<0){
        error(conn,-1,"invalid body length: %lld < 0.",bodylen);
        return false;
    }
    int command=head[BODYLEN_SIZE];
    int status=head[BODYLEN_SIZE+COMMAND_SIZE];
    logger("bodylen: %lld, command: %d,status: %d",bodylen,command,status);
    //根据命令执行具体业务处理
    bool result;
    switch (command) {
    case CMD_STORAGE_UPLOAD:
        // 处理来自客户机的上传文件请求
        result=upload(conn,bodylen);
        break;
    case CMD_STORAGE_FILESIZE:
        // 处理来自客户机的询问文件大小请求
        result=filesize(conn,bodylen);
        break;
    case CMD_STORAGE_DOWNLOAD:
        // 处理来自客户机的下载请求
        result=download(conn,bodylen);
        break;
    case CMD_STORAGE_DELETE:
        // 处理来自客户机的删除请求
        result=del(conn,bodylen);
        break;
    default:
        error(conn,-1,"unknow command: %d.",command);
        return false;
    }
    return result;
}

// 处理来自客户机的上传文件请求
bool service_c::upload(acl::socket_stream* conn, long long bodylen) const{
    // |包体长度|命令|状态|应用ID|用户ID|文件ID|文件大小|文件内容|
    // |   8   |  1 | 1 |  16  | 256  |  128 |   8   |文件大小|
    // 检查包体长度
    long long expected=APPID_SIZE+USERID_SIZE+FILEID_SIZE+BODYLEN_SIZE;
    if(bodylen<expected){
        error(conn,-1,"invalid body length: %lld < %lld.",bodylen,expected);
        return false;
    }
    // 接收包体
    char body[expected];
    if(conn->read(body,expected)<0){    // 通信失败
        logger_error("read fail: %s, expected: %lld, from: %s.",acl::last_serror(),expected,conn->get_peer());
        return false;
    }
    // 解析包体
    char appid[APPID_SIZE];
    strcpy(appid,body);
    char userid[USERID_SIZE];
    strcpy(userid,body+APPID_SIZE);
    char fileid[FILEID_SIZE];
    strcpy(fileid,body+APPID_SIZE+USERID_SIZE);
    long long filesize=ntoll(body+APPID_SIZE+USERID_SIZE+FILEID_SIZE);
    // 检查文件大小
    if(filesize!=bodylen-expected){
        logger_error("invalid file size: %lld != %lld.",filesize,bodylen-expected);
        error(conn,-1,"invalid file size: %lld != %lld.",filesize,bodylen-expected);
        return false;
    }
    // 生成文件路径
    char filepath[PATH_MAX+1];
    if(genpath(filepath)!=OK){      // 生成文件路径失败，genpath函数在下面实现
        error(conn,-1,"get file fail.");
        return false;
    }
    logger("upload file, appid: %s, useid: %s, fileid: %s, filesize: %lld, filepath: %s.",appid,userid,fileid,filesize,filepath);
    // 接收并保存文件
    int result=save(conn,appid,userid,fileid,filesize,filepath);
    if(result==SOCKET_ERROR) return false;
    else if(result==ERROR){
        error(conn,-1,"receive and save file fail, fileid: %s.",fileid);
        return false;
    }
    return ok(conn);
}

// 处理来自客户机的询问文件大小请求
bool service_c::filesize(acl::socket_stream* conn, long long bodylen) const{
    // |包体长度|命令|状态|应用ID|用户ID|文件ID|
    // |   8   |  1 | 1 |  16  |  256 |  128 |
    // 检查包体长度
    long long expected=APPID_SIZE+USERID_SIZE+FILEID_SIZE;
    if(bodylen!=expected){
        error(conn,-1,"invalid body length: %lld != %lld.",bodylen,expected);
        return false;
    }
    // 接收包体
    char body[expected];
    if(conn->read(body,expected)<0){
        logger_error("read fail: %s, expected: %lld, from: %s.",acl::last_serror(),expected,conn->get_peer());
        return false;
    }
    // 解析包体
    char appid[APPID_SIZE];
    strcpy(appid,body);
    char userid[USERID_SIZE];
    strcpy(userid,body+APPID_SIZE);
    char fileid[FILEID_SIZE];
    strcpy(fileid,body+APPID_SIZE+USERID_SIZE);
    // 数据库访问对象
    db_c db;
    // 连接数据库
    if(db.connect()!=OK) return false;
    // 文件路径文件大小
    std::string filepath;
    long long filesize;
    // 根据文件ID获取其对应的路径及大小
    if(db.get(appid,userid,fileid,filepath,&filesize)!=OK){
        error(conn,-1,"read database fail, fileid: %s.",fileid);
        return false;
    }
    logger("filesize, appid: %s, userid: %s, fileid: %s, filepath: %s, filesize: %lld.",appid,userid,fileid,filepath.c_str(),filesize);
    // |包体长度|命令|状态|文件大小|
    // |   8   | 1 | 1 |   8   |
    // 构造响应
    bodylen=BODYLEN_SIZE;
    long long resplen=HEADLEN+bodylen;
    char resp[resplen]={};
    llton(bodylen,resp);
    resp[BODYLEN_SIZE]=CMD_STORAGE_REPLY;
    resp[BODYLEN_SIZE+COMMAND_SIZE]=0;
    llton(filesize,resp+HEADLEN);
    // 发送响应
    if(conn->write(resp,resplen)<0){
        logger_error("write fail: %s, resplen: %lld, to: %s.",acl::last_serror(),resplen,conn->get_peer());
        return false;
    }
    return true;
}

// 处理来自客户机的下载请求
bool service_c::download(acl::socket_stream* conn,long long bodylen) const{
    // |包体长度|命令|状态|应用ID|用户ID|文件ID|偏移|大小|
    // |   8   |  1 |  1 | 16  |  256 |  128 |  8 | 8 |
    // 检查包体长度
    long long expected=APPID_SIZE+USERID_SIZE+FILEID_SIZE+BODYLEN_SIZE+BODYLEN_SIZE;
    if(bodylen!=expected){
        error(conn,-1,"invalid body length: %lld != %lld.",bodylen,expected);
        return false;
    }
    // 接收包体
    char body[expected];
    if(conn->read(body,expected)<0){
        logger_error("read fail: %s, bodylen: %lld, from: %s.",acl::last_serror(),bodylen,conn->get_peer());
        return false;
    }
    // 解析包体
    char appid[APPID_SIZE];
    strcpy(appid,body);
    char userid[USERID_SIZE];
    strcpy(userid,body+APPID_SIZE);
    char fileid[FILEID_SIZE];
    strcpy(fileid,body+APPID_SIZE+USERID_SIZE);
    long long offset=ntoll(body+APPID_SIZE+FILEID_SIZE);
    long long size=ntoll(body+APPID_SIZE+USERID_SIZE+FILEID_SIZE+BODYLEN_SIZE);
    // 数据库访问对象
    db_c db;
    // 连接数据库
    if(db.connect()!=OK) return false;
    // 文件路径文件大小
    std::string filepath;
    long long filesize;
    // 根据文件ID获取其对应的路径及大小
    if(db.get(appid,userid,fileid,filepath,&filesize)!=OK){
        error(conn,-1,"read database fail, fileid: %s.",fileid);
        return false;
    }
    // 检查位置
    if(offset<0||filesize<offset){
        logger_error("invalid offset, %lld is not between 0 and %lld.",offset,filesize);
        error(conn,-1,"invalid offset, %lld is not between 0 and %lld.",offset,filesize);
        return false;
    }
    // 大小为零表示下到文件尾
    if(!size) size=filesize-offset;
    // 检查大小
    if(size<0||filesize-offset<size){
        logger_error("invalid offset, %lld is not between 0 and %lld.",size,filesize-offset);
        error(conn,-1,"invalid offset, %lld is not between 0 and %lld.",size,filesize-offset);
        return false;
    }
    logger("down file, appid: %s, userid: %s, fileid: %s, offset: %lld, size: %lld, filepath: %s, filesize: %lld.",appid,userid,fileid,offset,size,filepath.c_str(),filesize);
    // 读取并发送文件
    int result=send(conn,filepath.c_str(),offset,size);
    if(result==SOCKET_ERROR) return false;
    else if(result==ERROR) {
        error(conn,-1,"read and send file fail, fileid: %s.",fileid);
        return false;
    }
    return true;
}

// 处理来自客户机的删除请求
bool service_c::del(acl::socket_stream* conn,long long bodylen) const{
    // |包体长度|命令|状态|应用ID|用户ID|文件ID|
    // |   8   |  1 | 1  | 16  |  256 |  128 |
    // 检查包体长度
    long long expected=APPID_SIZE+USERID_SIZE+FILEID_SIZE;
    if(bodylen!=expected){
        error(conn,-1,"invalid body length: %lld != %lld.",bodylen,expected);
        return false;
    }
    // 接收包体
    char body[expected];
    if(conn->read(body,expected)<0){
        logger_error("read fail: %s, expected: %lld, from: %s.",acl::last_serror(),expected,conn->get_peer());
        return false;
    }
    // 解析包体
    char appid[APPID_SIZE];
    strcpy(appid,body);
    char userid[USERID_SIZE];
    strcpy(userid,body+APPID_SIZE);
    char fileid[FILEID_SIZE];
    strcpy(fileid,body+APPID_SIZE+USERID_SIZE);
    // 数据库访问对象
    db_c db;
    // 连接数据库
    if(db.connect()!=OK) return false;
    // 文件路径、文件大小
    std::string filepath;
    long long filesize;
    // 根据文件ID获取其对应的路径及大小
    if(db.get(appid,userid,fileid,filepath,&filesize)!=OK){
        error(conn,-1,"read database fail, fileid: %s.",fileid);
        return false;
    }
    // 删除文件ID
    if(db.del(appid,userid,fileid)!=OK){
        error(conn,-1,"delete database fail, fileid:: %s.",fileid);
        return false;
    }
    // 删除文件
    if(file_c::del(filepath.c_str())!=OK){
        error(conn,-1,"delete file fail, fileid:: %s.",fileid);
        return false;
    }
    logger("delete file success, appid: %s, userid: %s, fileid: %s, filepath: %s, filesize: %lld.",appid,userid,fileid,filepath.c_str(),filesize);

    return ok(conn);
}


// 生成文件路径
int service_c::genpath(char * filepath) const{
    // 从存储路径表中随机抽取一个存储路径
    srand(time(nullptr));
    int nspaths=g_spaths.size();
    int nrand=rand()%nspaths;
    std::string spath=g_spaths[nrand];
    // 以存储路径为键从ID服务器获取与之对应的值作为文件ID
    id_c id;
    long fileid=id.get(spath.c_str());
    if(fileid<0) return ERROR;
    // 先将文件ID转换为512进制，在根据它生成文件路径
    return id2path(spath.c_str(),id512(fileid),filepath);
}

// 将id转化为512进制
long service_c::id512(long id) const{
    long result=0;
    for(int i=1;id;i*=1000){
        result+=(id%512)*i; // 若需要改成别的进制，只需要将512改成相应的进制数即可
        id/=512;
    }
    return result;
}

 // 用文件ID生成文件路径
long service_c::id2path(char const* spath,long fileid,char* filepath) const{
    // 检查存储路径
    if(!spath||!strlen(spath)){
        logger_error("storage path is null.");
        return ERROR;
    }
    // 生成文件路径中的各个分量
    unsigned short subdir1=(fileid/1000000000)%1000;// 一级子目录
    unsigned short subdir2=(fileid/1000000)%1000;   // 二级子目录
    unsigned short subdir3=(fileid/1000)%1000;      // 三级子目录
    time_t   curtime=time(nullptr);                 // 当前时间戳
    unsigned short postfix=(fileid/1)%1000;         // 文件名后缀
    // 格式化完成的文件路径，这里要是spath中已经有了/，就不用再加了
    if(spath[strlen(spath)-1]=='/')
        snprintf(filepath,PATH_MAX+1,"%s%03x/%03x/%03x/%lx_%03x",spath,subdir1,subdir2,subdir3,curtime,postfix);
    else snprintf(filepath,PATH_MAX+1,"%s/%03x/%03x/%03x/%lx_%03x",spath,subdir1,subdir2,subdir3,curtime,postfix);
    return OK;
}


// 接收并保存文件
int service_c::save(acl::socket_stream* conn,char const* appid,char const* userid,char const* fileid,long long filesize,char const* filepath) const{
    // 文件操作对象
    file_c file;
    // 打开文件
    if(file.open(filepath,file_c::O_WRITE)!=OK) return ERROR;
    // 依次将接收到的数据块写入文件
    // 未接收字节数
    long long remain=filesize;  // remain表示剩余字节数
    // 接收写入缓冲区
    char rcvmr[STORAGE_RCVWR_SIZE];
    // 还有未接收数据
    while(remain){
        // 接收数据
        long long bytes=std::min(remain,(long long)sizeof(rcvmr));
        long long count=conn->read(rcvmr,bytes);
        if(count<0){
            logger_error("read fail: %s, bytes: %lld, from: %s.",acl::last_serror(),bytes,conn->get_peer());
            file.close();
            return SOCKET_ERROR;
        }
        // 写入交件
        if(file.write(rcvmr,count)!=OK){
            file.close();
            return ERROR;
        }
        // 未收递减
        remain-=count;
    }
    // 关闭文件
    file.close();
    // 数据库访问对象
    db_c db;
    // 连接数据库
    if(db.connect()!=OK) return ERROR;
    // 设置文件ID和路径及大小的对应关系
    if(db.set(appid,userid,fileid,filepath,filesize)!=OK){
        error(conn,-1,"insert database fail, fileid:: %s.",fileid);
        file.del(filepath);
        return ERROR;
    }
    return OK;
}

// 读取并发送
int service_c::send(acl::socket_stream* conn,char const* filepath,long long offset,long long size) const{
    // 文件操作对象
    file_c file;
    // 打开文件
    if(file.open(filepath,file_c::O_READ)!=OK) return ERROR;
    // 设置偏移
    if(offset&&file.seek(offset)!=OK){
        file.close();
        return ERROR;
    }
    // |包体长度|命令|状态|文件内容|
    // |   8  |  1 | 1 |内容大小|
    // 构造响应头
    long long bodylen=size;
    long long headlen=HEADLEN;
    char head[headlen]={};
    llton(bodylen,head);
    head[BODYLEN_SIZE]=CMD_STORAGE_REPLY;
    head[BODYLEN_SIZE+COMMAND_SIZE]=0;
    // 发送响应头
    if(conn->write(head,headlen)<0){
        logger_error("write fail: %s, headlen: %lld to: %s.",acl::last_serror(),headlen,conn->get_peer());
        file.close();
        return SOCKET_ERROR;
    }
    // 依次将从文件中读取到的数据块作为响应包体的一部分发送出去
    // 未读取字节数
    long long remain=size;
    // 读取发送缓冲区
    char rdsend[STORAGE_RDSND_SIZE];
    // 还有未读取的数据
    while(remain){
        // 读取文件
        long long count=std::min(remain,(long long)sizeof(rdsend));
        if(file.read(rdsend,count)!=OK){
            file.close();
            return ERROR;
        }
        // 发送数据
        if(conn->write(rdsend,count)<0){
            logger_error("write fail: %s, count: %lld to: %s.",acl::last_serror(),count,conn->get_peer());
            file.close();
            return SOCKET_ERROR;
        }
        // 未读递减
        remain-=count;
    }
    // 关闭文件
    file.close();
    return OK;
}


// 应答成功
bool service_c::ok(acl::socket_stream* conn) const{
    // |包体长度|命令|状态|
    // |   8   | 1 | 1 |
    // 构造响应
    long long bodylen=0;
    long long resplen=HEADLEN+bodylen;
    char resp[resplen]={};
    llton(bodylen,resp);
    resp[BODYLEN_SIZE]=CMD_STORAGE_REPLY;
    resp[BODYLEN_SIZE+COMMAND_SIZE]=0;
    // 发送响应
    if(conn->write(resp,resplen)<0){
        logger_error("write fail: %s.resplen: %lld, to: %s.",acl::last_serror(),resplen,conn->get_peer());
        return false;
    }
    return true;
}
// 应答错误
bool service_c::error(acl::socket_stream* conn,short errnumb,char const* format,...) const{
    // 错误描述
    char errdesc[ERROR_DESC_SIZE];
    va_list ap;
    va_start(ap,format);
    vsnprintf(errdesc,ERROR_DESC_SIZE,format,ap);
    va_end(ap);
    logger_error("%s",errdesc);
    acl::string desc;
    desc.format("[%s] %s",g_hostname.c_str(),errdesc);
    memset(errdesc,0,sizeof(errdesc));
    strncpy(errdesc,desc.c_str(),ERROR_DESC_SIZE-1);
    size_t desc_len=strlen(errdesc);
    desc_len+=desc_len!=0;
    // |包体长度|命令|状态|错误号|错误描述|
    // |   8   | 1 | 1 |  2  | <=1024|
    // 构造响应
    long long bodylen=ERROR_NUMB_SIZE+desc_len;
    long long resplen=HEADLEN+bodylen;
    char resp[resplen]={};
    llton(bodylen,resp);
    resp[BODYLEN_SIZE]=CMD_STORAGE_REPLY;
    resp[BODYLEN_SIZE+COMMAND_SIZE]=STATUS_ERROR;
    ston(errnumb,resp+HEADLEN);
    if(desc_len) strcpy(resp+HEADLEN+ERROR_NUMB_SIZE,errdesc);
    // 发送响应
    if(conn->write(resp,resplen)<0){
        logger_error("write fail: %s.resplen: %lld, to: %s.",acl::last_serror(),resplen,conn->get_peer());
        return false;
    }
    return true;
}