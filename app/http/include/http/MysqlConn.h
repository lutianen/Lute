/**
 * @file MysqlConn.h
 * @brief MySQL 连接的 RAII 实现
 *
 * @author Lux
 */

#pragma once

#include <LuxLog/Logger.h>
#include <LuxUtils/Types.h>
#include <mysql/mysql.h>

namespace Lux {
namespace mysql {

class MySQLConn {
private:
    // TODO use std::unique_ptr
    MYSQL* connPtr_;

    std::string url_;
    unsigned int port_;
    std::string user_;
    std::string passwd_;
    std::string db_;

public:
    MySQLConn(MYSQL* mysql, const std::string& url, unsigned int port,
              const std::string user, const std::string& passwd,
              const std::string& db)
        : connPtr_(mysql),
          url_(url),
          port_(port),
          user_(user),
          passwd_(passwd),
          db_(db) {
        connPtr_ = mysql_init(connPtr_);
        if (nullptr == connPtr_) {
            LOG_ERROR << "MySQL init Error";
            exit(1);
        }

        connPtr_ =
            mysql_real_connect(connPtr_, url.c_str(), user_.c_str(),
                               passwd_.c_str(), db_.c_str(), port, nullptr, 0);
        if (nullptr == connPtr_) {
            LOG_ERROR << "MySQL init Error";
            exit(1);
        }
    }

    ~MySQLConn() { mysql_close(connPtr_); }
};
}  // namespace mysql
}  // namespace Lux