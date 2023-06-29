/**
 * @file app.h
 * @brief
 *
 * @author Lux
 */

#pragma once

#include <LuteBase.h>
#include <LuteMySQL.h>
#include <LuteRedis.h>
#include <http/HttpRequest.h>
#include <http/HttpResponse.h>
#include <http/HttpServer.h>
#include <mysql/mysql.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fstream>
#include <map>
#include <regex>

namespace Lute {
namespace http {

    class Application {
    public:
        static const int FILENAME_LEN = 200;

    private:
        Lute::EventLoop* loop_;
        HttpServer server_;
        int numThreads_;

        char realFile_[FILENAME_LEN];
        std::string serverPath_;
        struct stat fileStat_;
        char* fileAddr_;

        std::string dbIpAddr_;
        uint16_t dbPort_;
        // 数据用户名
        std::string dbUser_;
        // 密码
        std::string dbPasswd_;
        // 数据库名
        std::string dbName_;
        // 数据库连接池
        mysql::MySQLConnPool* connPool_;

        // Redis
        redis::RedisConn redisConn_;

    public:
        Application(Lute::EventLoop* loop, const Lute::InetAddress& listenAddr,
                    const std::string& name, const std::string& root,
                    const std::string& dbIpAddr, uint16_t dbPort,
                    const std::string& dbUser, const std::string& dbPasswd,
                    const std::string& dbName);

        void onRequest(const HttpRequest& req, HttpResponse* resp);

        void setNumThreads(int num) { server_.setThreadNum(num); }
        void start() { server_.start(); }

    private:
        std::string getHtml();
    };
}  // namespace http
}  // namespace Lute