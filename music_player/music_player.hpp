#pragma once 

#include <stdio.h>
#include <iostream>
#include <string>

#include <jsoncpp/json/json.h>

#include "httplib.h"
#include "database.hpp"
#include "session.hpp"

using namespace std;
using namespace httplib;

#define MUSIC_SVR_IP "0.0.0.0"
#define MUSIC_SVR_PORT 18989

class MusicServer
{
    public:
        MusicServer()
        {
            svr_ip_ = MUSIC_SVR_IP;
            svr_port_ = MUSIC_SVR_PORT;
            db_svr_ = NULL;
            all_sess_ = NULL;
        }

        ~MusicServer()
        {}

        // 1. 初始化类接口
        int InitMusicServer(string db_ip, uint16_t db_port, string db_user, string db_passwd, string db_db, string ip = MUSIC_SVR_IP, uint16_t port = MUSIC_SVR_PORT)
        {
            svr_ip_ = ip;
            svr_port_ = port;

            // ip， 用户名， 密码， 数据库名， 端口号
            db_svr_ = new DataBaseSvr(db_ip, db_user, db_passwd, db_db, db_port);
            if(db_svr_ == NULL)
                return -1;

            if(db_svr_->ConnectToMysql() < 0)
                return -2;

            all_sess_ = new AllSessionInfo();
            if(all_sess_ == NULL)
                return -3;

            return 0;
        }

        // 2. 启动服务接口
        void StartMusicServer()
        {
            // 1. 注册若干个hhtp请求对应的回调函数
            http_svr_.Post("/register", [this](const Request& req, Response& resp){

                    // 1.需要将浏览器提交的数据继续持久化
                    // 将用户提交的数据，继续反序列化，拿到一个josn对象
                    Json::Reader r;
                    Json::Value v;
                    r.parse(req.body, v);

                    Json::Value resp_json;
            
                    // 2. 将浏览器提交的数据持久化（保存到数据库当中）
                    resp_json["status"] = db_svr_->InsertUserInfo(v);

                    Json::FastWriter w;

                    resp.body = w.write(resp_json);
                    resp.set_header("Content-Type", "application/json");

                    });
            http_svr_.Post("/login", [this](const Request& req, Response& resp){
                    // 1. 将浏览器提交的json串反序列化为json对象
                    // 2. 调用数据库模板的函数进行查找和对比
                    //          有邮箱，继续查找，用密码进行比对
                    // 3. 根据登录状态，判断是否会生成会话信息
                    // 4. 返回给浏览器一个结果
                    //      登陆失败
                    //      登陆成功：返回会话ID
                    
                    Json::Reader r;
                    Json::Value v;
                    r.parse(req.body, v);

                    Json::Value resp_json;
                    int user_id = db_svr_->QueryUserExist(v);
                    string session_id = "";

                    if(user_id > 0)
                    {
                        Session sess(v, user_id);
                        // 保存会话信息
                        session_id = sess.GetSessionID();
                        all_sess_->InsertSessionInfo(session_id, sess);
                    }

                    resp_json["login_status"] = user_id > 0 ? true : false;

                    Json::FastWriter w;
                    resp.body = w.write(resp_json);
                    resp.set_header("Set-Cookie", session_id.c_str());
                    resp.set_header("Content-Type", "application/json");

                    });
                    

            http_svr_.Get("/findMusic", [this](const Request& req, Response& resp){
                    // 1. 会话校验：从当前http请求的请求体中拿到Cookie对应的value及会话ID
                    // 2. 通过会话ID在all_sess_当中1查找是否有对应的会话
                    //      找到了，则认为该用户是登录用户
                    //      没找到，则认为该用户为非登录用户，返回status为-1
                    // 3. 通过数据库模板在数据表中查找音乐信息，将查找到的音乐信息组织为json串
                    // 4. 组织应答
                    
                    Json::Value resp_json;
                    int user_id = all_sess_->CheckSession(req);
                    resp_json["status"] = user_id;

                    if(user_id > 0)
                    {
                        // 1. 会话校验成功的逻辑
                        // 2. 访问数据库，获取到数据表music的音乐信息
                        db_svr_->GetAllMusic(resp_json);
                    }

                    Json::FastWriter w;
                    resp.body = w.write(resp_json);
                    resp.set_header("Content-Type", "application/json");

                    });

            http_svr_.Post("/loveMusic", [this](const Request& req, Response& resp){
                    Json::Value resp_json;
                    int user_id = all_sess_->CheckSession(req);
                    resp_json["status"] = user_id;

                    if(user_id > 0)
                    {
                        cout << "user_id:" << user_id << ",req正文信息:" << req.body << endl;
                        resp_json["status"] = db_svr_->InsertLoveMusic(req, user_id);
                    }

                    Json::FastWriter w;
                    resp.body = w.write(resp_json);
                    resp.set_header("Content-Type", "application/json");
                    });

            http_svr_.Get("/findLoveMusic", [this](const Request& req, Response& resp){
                    // 1.会话校验，通过会话校验，如果获取成功，则返回当前用户ID
                    // 2. 查询数据库，查看当前用户喜欢的音乐
                    // 3. 将用户喜欢的音乐返回给浏览器
                    
                    Json::Value resp_json;
                    int user_id = all_sess_->CheckSession(req);
                    resp_json["status"] = user_id;

                    if(user_id > 0)
                    {
                        // 去数据库查询用户喜欢的音乐
                        db_svr_->GetLoveMusic(user_id, resp_json);
                    }

                    Json::FastWriter w;
                    resp.body = w.write(resp_json);
                    resp.set_header("Content-Type", "application/json");
                    });
            // 2. 设置http服务器的静态路径
            http_svr_.set_mount_point("/", "./web");

            // 3.监听
            http_svr_.listen(svr_ip_.c_str(), svr_port_);
        }

    private:
        Server http_svr_;

        string svr_ip_;
        uint16_t svr_port_;

        DataBaseSvr*  db_svr_;
        AllSessionInfo* all_sess_;
};
