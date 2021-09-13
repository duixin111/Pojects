#pragma once 
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <openssl/md5.h>
#include <iostream>
#include <string>
#include <unordered_map>

#include <jsoncpp/json/json.h>
#include "httplib.h"

using namespace std;
using namespace httplib;

typedef enum UserStatus
{
    // 不在线
    OFFLINE = 0,
    // 在线
    ONLINE 
}USER_STATUS;

// 针对 登录的用户创建会话信息
class Session
{
    public:
        Session()
        {}

        Session(const Json::Value& v, int user_id)
        {
            user_id_ = user_id;
            real_str_ += v["email"].asString();
            real_str_ += v["password"].asString();
            us_status_ = ONLINE;
        }

        ~Session()
        {}

        bool CalcMd5()
        {
            MD5_CTX ctx;
            MD5_Init(&ctx);

            if(MD5_Update(&ctx, real_str_.c_str(), real_str_.size()) != 1)
                return false;

            // md ->16 字节
            unsigned char md5[16] = {0};
            if(MD5_Final(md5, &ctx) != 1)
                return false;

            char tmp[2] = {0};
            char buf[32] = {0};

            for(int i = 0; i < 16; i++)
            {
                sprintf(tmp, "%02x", md5[i]);
                strncat(buf, tmp, 2);
            }

            session_id_ = buf;
            cout << "session_id_" << session_id_ << endl;

            return true;
        }
        string& GetSessionID()
        {
            // 1. 计算sessionID
            CalcMd5();

            // 返回session_id
            return session_id_;
        }

        int GetUserId()
        {
            return user_id_;
        }

    private:

        string session_id_;

        // 生成会话id的源字符串
        string real_str_;

        int user_id_;

        UserStatus us_status_;
};

// 保存多个用户的对应的会话信息
class AllSessionInfo 
{
    public:
        AllSessionInfo()
        {
            all_sess_map_.clear();
            pthread_mutex_init(&map_lock_, NULL);
        }

        ~AllSessionInfo()
        {
            pthread_mutex_destroy(&map_lock_);
        }

        void InsertSessionInfo(const string session_id, const Session sess)
        {
            pthread_mutex_lock(&map_lock_);
            all_sess_map_.insert(make_pair(session_id, sess));
            pthread_mutex_unlock(&map_lock_);
        }

        int CheckSession(const Request& req)
        {
            // 1. 通过http请求头部当中的Cookie字段获取对应的会话ID
            string session_id = req.get_header_value("Cookie");

            // 2. 通过会话ID在all_map_当中查找是否有对应的会话信息
            pthread_mutex_lock(&map_lock_);
            auto iter = all_sess_map_.find(session_id);
            if(iter == all_sess_map_.end())
            {
                pthread_mutex_unlock(&map_lock_);
                return -1;
            }
            int user_id = iter->second.GetUserId();
            pthread_mutex_unlock(&map_lock_);

            return user_id;
        }

    private:
        unordered_map<string, Session> all_sess_map_;
        pthread_mutex_t map_lock_;
};
