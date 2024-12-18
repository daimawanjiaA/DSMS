/**
 * (03_util)实用工具
 * wfl:2024-3-29
 * change:2024-5-11
*/
// 公共模块
// 定义几个实用函数 
// #pragma once

#include "util.hpp"
#include "types.hpp"
#include <string.h>

// long long类型整数主机序转为网络序
void llton(long long ll,char* n){
    // n[0]=ll>>56; n[1]=ll>>48; n[2]=ll>>40; n[3]=ll>>32;
    // n[4]=ll>>24; n[5]=ll>>16; n[6]=ll>> 8; n[7]=ll>> 0;
    for(size_t i=0;i<sizeof(ll);++i){
        n[i]=ll>>(sizeof(ll)-i-1)*8;
    }
}
// long long类型整数网络序转为主机序
long long ntoll(char const* n){
    long long ll=0;
    // ll|=(long long)(unsigned char)n[0]<<56;
    // ll|=(long long)(unsigned char)n[1]<<48;
    // ll|=(long long)(unsigned char)n[2]<<40;
    for(size_t i=0;i<sizeof(ll);++i){
        ll|=(long long)(unsigned char)n[i]<<(sizeof(ll)-i-1)*8;
    }
    return ll;
}

// long类型整数主机序转为网络序
void lton(long l,char* n){
    for(size_t i=0;i<sizeof(l);++i){
        n[i]=l>>(sizeof(l)-i-1)*8;
    }
}
// long类型整数网络序转为主机序
long ntol(char const* n){
    long l=0;
    for(size_t i=0;i<sizeof(l);++i){
        l|=(long)(unsigned char)n[i]<<(sizeof(l)-i-1)*8;
    }
    return l;
}

// short类型整数主机序转为网络序
void ston(short s,char* n){
    for(size_t i=0;i<sizeof(s);++i){
        n[i]=s>>(sizeof(s)-i-1)*8;
    }
}
// short类型整数网络序转为主机序
short ntos(char const* n){
    short s;
    for(size_t i=0;i<sizeof(s);++i){
        s|=(short)(unsigned char)n[i]<<(sizeof(s)-i-1)*8;
    }
    return s;
}

// 字符串是否合法，即是否只包含26个因为字母大小写和0-9数字
int valid(char const* str){
    if(!str) return ERROR;
    size_t len=strlen(str);
    if(!len) return ERROR;
    for(size_t i=0;i<len;++i){
        if(!(('a'<=str[i]&&str[i]<='z')||('A'<=str[i]&&str[i]<='Z')||('0'<=str[i]&&str[i]<='9')))
            return ERROR;
    }
    return OK;
}

// 以分号为分割符，将一个字符串拆分为多个子字符串
int split(char const* str,std::vector<std::string>& substrs){
    // 类似strtok函数
    if(!str) return ERROR;
    size_t len=strlen(str);
    if(!len) return ERROR;
    char *buf=new char[len+1];
    strcpy(buf,str);
    char const* sep=";";
    for(char *substr=strtok(buf,sep);substr;substr=strtok(nullptr,sep))
        substrs.push_back(substr);
    delete[] buf;
    return OK;
}

