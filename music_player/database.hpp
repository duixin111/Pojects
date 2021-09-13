#pragma once 
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string>

#include <jsoncpp/json/json.h>
#include <mysql/mysql.h>

#include "httplib.h"

using namespace std;
using namespace httplib;

class DataBaseSvr
{
    public:
        DataBaseSvr(const string& db_host, const string& db_user, const string& db_password, const string& db_name, unsigned int db_port = 3306)
        {
            mysql_init(&mysql_);
            db_host_ = db_host;
            db_user_ = db_user;
            db_password_ = db_password;
            db_name_ = db_name;
            db_port_ = db_port;
        }

        ~DataBaseSvr()
        {}

        int ConnectToMysql()
        {
           if(!mysql_real_connect(&mysql_, db_host_.c_str(), db_user_.c_str(), db_password_.c_str(), db_name_.c_str(), db_port_, NULL, CLIENT_FOUND_ROWS))
           {
               cout << mysql_error(&mysql_) << endl;
               return -1;
           }
           return 0;
        }

        bool InsertUserInfo(const Json::Value& v)
        {
            
            // 1.连接数据库
            // 2. 组织sql语句
            // {"name":"xxx","password":"xxx","email":"xxx","phonenumber":"xxx"}
            string name = v["name"].asString();
            string passwd = v["passwd"].asString();
            string email = v["email"].asString();
            string phonenum = v["phonenum"].asString();
#define INSERT_USER_INFO "insert into sys_user(user_name, password, email, phone_num) values('%s','%s','%s','%s');"
            char sql[1024] = {0};
            snprintf(sql, sizeof(sql) - 1, INSERT_USER_INFO, name.c_str(), passwd.c_str(), email.c_str(), phonenum.c_str());
                        
            // 3.继续执行sql
            // 4.返回插入结果给调用
            return ExecuteSql(sql);
        }

        bool ExecuteSql(const  string& sql)
        {
            // 1.设置mysql客户端字符集为utf8
            mysql_query(&mysql_, "set names utf8");
            // 执行sql
            if(mysql_query(&mysql_, sql.c_str()) != 0)
            {
                cout << mysql_error(&mysql_) << endl;
                return false;
            }

            return true;
        }
        bool ExecuteSql(const string& sql, MYSQL_RES** res)
        {
            // 1. 设置mysql客户端字符集为utf8
            mysql_query(&mysql_, "set names utf8");
            
            // 2. 执行sql
            if(mysql_query(&mysql_,sql.c_str()) != 0)
            {
                cout << mysql_error(&mysql_) << endl;
                return false;
            }
            // 3. 将mysql执行的结果放入到结果集中
            *res = mysql_store_result(&mysql_);
            if(res == NULL)
                return false;
        
            return true;
        }

        int QueryUserExist(const Json::Value& v)
        {
            // 1. 从json对象中解析出email和password
            string email = v["email"].asString();
            string password = v["password"].asString();
            // 2. 使用email作为查询条件，在sys_user表中进行查询
#define QUERY_USER "select * from sys_user where email='%s';"
            char sql[1024] = {0};
            snprintf(sql, sizeof(sql) - 1, QUERY_USER, email.c_str());
            cout << sql << endl;
            
            // 3. 对密码进行比对
            MYSQL_RES* res = NULL;
            if(ExecuteSql(sql, &res) == false)
                return -2;
            // 针对结果集进行操作，判断结果集中行数是否为1
            // 若为1，继续执行，否则退出
            if(mysql_num_rows(res) != 1)
            {
                cout << "No data for sql! sql is " << sql << endl;
                mysql_free_result(res);
                return -3;
            }
            // 在结果集中获取一行数据
            MYSQL_ROW row = mysql_fetch_row(res);

            // 比对密码
            string db_password = row[2];
            if(db_password != password)
                return -4;

            mysql_free_result(res);

            // 返回查询结果
            return atoi(row[0]);
        }

        int GetMusic(string sql, Json::Value& resp_json)
        {
            MYSQL_RES* res = NULL;
            if(ExecuteSql(sql, &res) == false)
                return -2;
            
            int row_nums = mysql_num_rows(res);
            if(row_nums <= 0)
            {
                printf("No data: sql is \"%s\"", sql.c_str());
                mysql_free_result(res);
                return -3;
            }

            MYSQL_ROW row = mysql_fetch_row(res);
            Json::Value music_value;
            while(row != NULL)
            {
                // 将数据保存到json中
                Json::Value tmp;
                tmp["id"] = row[0];
                tmp["title"] = row[1];
                tmp["singer"] = row[2];
                tmp["url"] = row[3];

                music_value.append(tmp);
                row = mysql_fetch_row(res);
            }

            resp_json["music"] = music_value;
            mysql_free_result(res);

            return 0;
        }

        int GetAllMusic(Json::Value& resp_json)
        {
            const char* sql = "select * from music;";
            return GetMusic(sql,resp_json);
        }

        int InsertLoveMusic(const Request& req, int user_id)
        {
            // 1. 通过req当中的正文信息获取music_id
            // 2. 使用music_id, user_id组织sql语句
            // 3. 执行sql语句
            Json::Reader r;
            Json::Value v;
            r.parse(req.body, v);

            int music_id = v["music_id"].asInt();
#define INSERT_LOVE_MUSIC "insert into love_music(user_id, music_id) values(%d, %d);"
            char sql[1024] = {0};
            snprintf(sql, sizeof(sql) - 1, INSERT_LOVE_MUSIC, user_id, music_id);
            if(ExecuteSql(sql) == false)
                return -1;
            return 1;
        }

        int GetLoveMusic(int user_id, Json::Value& resp_json)
        {
            // 1. 组织查询sql语句
            // 2. 调用GetMusic函数
#define GET_LOVE_MUSIC "select * from music where music_id in(select music_id from love_music where user_id=%d);"
            char sql[1024] = {0};
            snprintf(sql, sizeof(sql) - 1, GET_LOVE_MUSIC, user_id);
           
            return GetMusic(sql, resp_json);
        }



    private:
        MYSQL mysql_;

        string db_host_;
        string db_user_;
        string db_password_;
        string db_name_;
        unsigned int db_port_;
};

