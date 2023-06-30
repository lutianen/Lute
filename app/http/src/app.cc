#include <LuteBase.h>
#include <LuteMySQL.h>
#include <LutePolaris.h>
#include <fcntl.h>
#include <http/app.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <memory>
#include <thread>

using namespace Lute;
// using namespace Lute::polaris;
using namespace Lute::http;
using namespace Lute::mysql;

bool benchmark = false;

// FIXME use Redis
// username : <mail, password>
// std::map<string, std::pair<string, string>> users;
const std::string HASH_KEY{"user"};

Application::Application(EventLoop* loop, const InetAddress& listenAddr,
                         const std::string& name, const std::string& root,
                         const std::string& dbIpAddr, uint16_t dbPort,
                         const std::string& dbUser, const std::string& dbPasswd,
                         const std::string& dbName)
    : loop_(loop),
      server_(loop, listenAddr, name),
      numThreads_(0),
      realFile_(),
      serverPath_(root),
      fileStat_(),
      fileAddr_(nullptr),
      dbIpAddr_(dbIpAddr),
      dbPort_(dbPort),
      dbUser_(dbUser),
      dbPasswd_(dbPasswd),
      dbName_(dbName),
      connPool_(MySQLConnPool::getInstance()) {
    (void)loop_;
    (void)numThreads_;
    (void)fileAddr_;

    memZero(realFile_, FILENAME_LEN);

    server_.setHttpCallback(std::bind(&Application::onRequest, this,
                                      std::placeholders::_1,
                                      std::placeholders::_2));

    connPool_->init(10, dbIpAddr_, dbPort_, dbUser_, dbPasswd_, dbName_, true,
                    "utf8mb4");

    MYSQL* mysql = nullptr;
    MySQLConn conn(mysql, connPool_);
    conn.execute("SELECT username, mail, passwd FROM user;");

    redisConn_.connectSvr("127.0.0.1", 6379, 1200);

    for (auto& row : conn.stmtRes_.rows_) {
        // users[row[0]] = {row[1], row[2]};
        LOG_DEBUG << row[0] << ", " << row[1] << ", " << row[2];
        std::string passwd(row[2]);
        if (redisConn_.setHField(HASH_KEY, std::string(row[0]), passwd) != 0) {
            LOG_ERROR << "Redis set failed";
        }
    }
}

void Application::onRequest(const HttpRequest& req, HttpResponse* resp) {
    LOG_INFO << "Headers " << req.methodString() << " " << req.path();

    if (req.path() == "/" || req.path() == "/index.html") {
        resp->setStatusCode(HttpResponse::HttpStatusCode::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        resp->addHeader("Server", "Lux polaris");

        strcpy(realFile_, serverPath_.c_str());
        int len = ::strlen(realFile_);
        LOG_INFO << "realFile_ len: " << len;

        const char* index = "/index.html";
        strcat(realFile_, index);

        resp->setBody(getHtml());
    } else if (req.path() == "/register") {
        LOG_INFO << req.path();
        resp->setStatusCode(HttpResponse::HttpStatusCode::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        resp->addHeader("Server", "Lux polaris");
        std::string body = req.body();

        if (req.method() == HttpRequest::Method::kPost && !body.empty()) {
            // 处理注册信息 - user, mail, password
            std::string username, mail, password;
            std::regex pattern(
                "Username=([0-9a-zA-Z_]+)&email=([0-9a-zA-Z]+\\%40[a-zA-Z]+"
                "\\."
                "com)&password=([0-9a-zA-Z]+)");
            for (std::sregex_iterator iter(body.begin(), body.end(), pattern),
                 iterend;
                 iter != iterend; ++iter) {
                username = iter->str(1);
                mail = iter->str(2);
                password = iter->str(3);
            }
            std::string stmt =
                "INSERT INTO user(username, mail, passwd) VALUES('" + username +
                "', '" + mail + "', '" + password + "')";

            // if (users.find(username) == users.end()) {
            std::string pwd;
            // if (redisConn_.getHField(HASH_KEY, username, pwd) == 0) {
            if (redisConn_.hasHField(HASH_KEY, username) == 0) {
                LOG_INFO << "username: " << username << "pwd: " << pwd;

                MYSQL* mysql = nullptr;
                MySQLConn conn(mysql, connPool_);
                conn.execute(stmt.c_str());

                // 成功
                if (!conn.stmtRes_.rc_) {
                    // users[username] = {mail, password};
                    redisConn_.setHField(HASH_KEY, username, password);
                    strcpy(realFile_, serverPath_.c_str());
                    int len = strlen(realFile_);
                    LOG_INFO << "realFile_ len: " << len;

                    const char* index = "/welcome.html";
                    strcat(realFile_, index);
                } else {
                    LOG_ERROR << "SELECT error: " << mysql_error(mysql);
                }
            } else {
                strcpy(realFile_, serverPath_.c_str());
                int len = strlen(realFile_);
                LOG_INFO << "realFile_ len: " << len;

                const char* index = "/registerFailed.html";
                strcat(realFile_, index);
            }
        } else {
            strcpy(realFile_, serverPath_.c_str());
            int len = strlen(realFile_);
            LOG_INFO << "realFile_ len: " << len;

            const char* index = "/register.html";
            strcat(realFile_, index);
        }

        resp->setBody(getHtml());
    } else if (req.path() == "/welcome") {
        resp->setStatusCode(HttpResponse::HttpStatusCode::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        resp->addHeader("Server", "Lux polaris");

        strcpy(realFile_, serverPath_.c_str());
        int len = strlen(realFile_);
        LOG_INFO << "realFile_ len: " << len;

        const char* index = "/welcome.html";
        strcat(realFile_, index);

        resp->setBody(getHtml());

    } else if (req.path().find(".jpg") != std::string::npos) {
        resp->setStatusCode(HttpResponse::HttpStatusCode::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("image/jpg");
        resp->addHeader("Server", "Lux polaris");

        strcpy(realFile_, serverPath_.c_str());
        int len = strlen(realFile_);
        LOG_INFO << "realFile_ len: " << len;

        const char* index = req.path().c_str();
        strcat(realFile_, index);

        LOG_INFO << realFile_;

        // NO resource
        if (stat(realFile_, &fileStat_) < 0) return;
        // FORBIDDEN REQUEST
        if (!(fileStat_.st_mode & S_IROTH)) return;
        // BAD_REQUEST
        if (S_ISDIR(fileStat_.st_mode)) return;

        std::ifstream ifs(realFile_, std::ios::ios_base::binary);
        if (!ifs.is_open()) {
            LOG_ERROR << "Open " << realFile_ << " failed";
            resp->setBody("");
        } else {
            // 将文件读入到ostringstream对象buf中
            std::ostringstream buf{};
            char ch;
            while (buf && ifs.get(ch)) buf.put(ch);

            resp->setBody(buf.str());
        }
    } else if (req.method() == HttpRequest::Method::kGet &&
               req.path().find("/login") != std::string::npos) {
        LOG_INFO << req.path();
        LOG_INFO << req.query();

        std::regex pattern(
            "\\?Username=([0-9a-zA-Z]+)&password=([0-9a-zA-Z]+)");

        std::string query = req.query(), username, passwd;
        for (std::sregex_iterator iter(query.begin(), query.end(), pattern),
             iterEnd;
             iter != iterEnd; ++iter) {
            username = iter->str(1);
            passwd = iter->str(2);
        }

        //  使用 Redis 缓存
        if (redisConn_.hasHField(HASH_KEY, username)) {
            std::string pwd;
            redisConn_.getHField(HASH_KEY, username, pwd);
            LOG_INFO << "username: " << username << "pwd: " << pwd;

            if (pwd == passwd) {
                resp->setStatusCode(HttpResponse::HttpStatusCode::k200Ok);
                resp->setStatusMessage("OK");
                resp->setContentType("text/html");
                resp->addHeader("Server", "Lux polaris");

                strcpy(realFile_, serverPath_.c_str());
                int len = strlen(realFile_);
                LOG_INFO << "realFile_ len: " << len;

                const char* index = "/welcome.html";
                strcat(realFile_, index);

                resp->setBody(getHtml());
            } else {
                resp->setStatusCode(HttpResponse::HttpStatusCode::k200Ok);
                resp->setStatusMessage("OK");
                resp->setContentType("text/html");
                resp->addHeader("Server", "Lux polaris");

                strcpy(realFile_, serverPath_.c_str());
                int len = strlen(realFile_);
                LOG_INFO << "realFile_ len: " << len;

                const char* index = "/loginFailed.html";
                strcat(realFile_, index);

                resp->setBody(getHtml());
            }
        } else {
            resp->setStatusCode(HttpResponse::HttpStatusCode::k200Ok);
            resp->setStatusMessage("OK");
            resp->setContentType("text/html");
            resp->addHeader("Server", "Lux polaris");

            strcpy(realFile_, serverPath_.c_str());
            int len = strlen(realFile_);
            LOG_INFO << "realFile_ len: " << len;

            const char* index = "/loginFailed.html";
            strcat(realFile_, index);

            resp->setBody(getHtml());
        }
    } else {
        resp->setStatusCode(HttpResponse::HttpStatusCode::k404NotFound);
        resp->setStatusMessage("Not Found");

        resp->setContentType("text/html");
        resp->addHeader("Server", "Lux polaris");

        strcpy(realFile_, serverPath_.c_str());
        int len = strlen(realFile_);
        LOG_INFO << "realFile_ len: " << len;

        strcat(realFile_, "/404.html");

        resp->setBody(getHtml());

        resp->setCloseConnection(true);
    }
    // FILE REQUEST
    return;
}

std::string readFile2String(const char* filename) {
    std::ifstream ifs;
    Lute::FSUtil::openForRead(ifs, filename, std::ios_base::in | std::ios_base::binary);

    if (!ifs.is_open()) {
        LOG_ERROR << "Open " << filename << " failed";
        return "";
    } else {
        // 将文件读入到ostringstream对象buf中
        std::ostringstream buf{};
        char ch;
        while (buf && ifs.get(ch)) buf.put(ch);

        return buf.str();
    }
    // int fd = ::open(filename, O_RDONLY | O_CLOEXEC);
    // if (fd < 0) {
    //     LOG_ERROR << "Failed to open file " << filename;
    //     // exit(EXIT_FAILURE);
    // }

    // struct stat statbuf;
    // if (::fstat(fd, &statbuf) < 0) {
    //     LOG_ERROR << "Failed to get file size";
    //     // exit(EXIT_FAILURE);
    // }

    // char* file_contents = static_cast<char*>(
    //     ::mmap(0, statbuf.st_size, PROT_READ | PROT_EXEC, MAP_PRIVATE, fd, 0));
    // if (file_contents == MAP_FAILED) {
    //     LOG_ERROR << "Failed to mmap file";
    //     // exit(EXIT_FAILURE);
    // }

    // std::string str(file_contents, statbuf.st_size);

    // if (::munmap(file_contents, statbuf.st_size) < 0) {
    //     LOG_ERROR << "Failed to munmap file";
    //     // exit(EXIT_FAILURE);
    // }

    // ::close(fd);

    // return str;
}

std::string Application::getHtml() {
    std::string ret{};

    // NO resource
    if (stat(realFile_, &fileStat_) < 0) return ret;
    // FORBIDDEN REQUEST
    if (!(fileStat_.st_mode & S_IROTH)) return ret;
    // BAD_REQUEST
    if (S_ISDIR(fileStat_.st_mode)) return ret;

    // std::ifstream ifs(realFile_, std::ios::ios_base::binary);
    // if (!ifs.is_open()) {
    //     LOG_ERROR << "Open " << realFile_ << " failed";
    //     return ret;
    // } else {
    //     // 将文件读入到ostringstream对象buf中
    //     std::ostringstream buf{};
    //     char ch;
    //     while (buf && ifs.get(ch)) buf.put(ch);

    //     return buf.str();
    // }

    return readFile2String(realFile_);
}

const static std::string ICON =
RED "               ,--,                                      \r\n" CLR 
GREEN "            ,---.'|                                         \r\n " CLR 
YELLOW "           |   | :                       ___                \r\n" CLR 
BLUE "            :   : |                     ,--.'|_              \r\n" CLR 
PURPLE "            |   ' :             ,--,    |  | :,'             \r\n" CLR 
DEEPGREEN "            ;   ; '           ,'_ /|    :  : ' :             \r\n" CLR 
WHITE "            '   | |__    .--. |  | :  .;__,'  /      ,---.   \r\n" CLR 
DEEPGREEN "            |   | :.'| ,'_ /| :  . |  |  |   |      /     \\  \r\n" CLR 
WHITE "            '   :    ; |  ' | |  . .  :__,'| :     /    /  | \r\n" CLR 
DEEPGREEN "            |   |  ./  |  | ' |  | |    '  : |__  .    ' / | \r\n" CLR 
PURPLE "            ;   : ;    :  | : ;  ; |    |  | '.'| '   ;   /| \r\n" CLR 
BLUE "            |   ,/     '  :  `--'   \\   ;  :    ; '   |  / | \r\n" CLR 
YELLOW "            '---'      :  ,      .-./   |  ,   /  |   :    | \r\n" CLR 
GREEN "                        `--`----'        ---`-'    \\   \\  /  \r\n" CLR 
RED "                                                    `----'   \r\n" CLR 
;

void output(const char*, int) {}

int main(int argc, char* argv[]) {
    initLogger(Lute::Logger::LogLevel::INFO);
    Lute::Logger::setOutput(output);

    std::string dbIp = "192.168.1.108";
    uint16_t dbPort = 3306;
    std::string user = "lutianen";
    std::string passwd = "lutianen";
    std::string db = "LuxDatabase";
    std::string serverName = "LuxPolaris";
    std::string staticSrcPrefix = "/home/lux/Lux/app/HTML";
    uint16_t serverPort = 5836;
    int numThreads = 8;

    if (argc > 1) {
        benchmark = true;

        serverName = argv[1];
        staticSrcPrefix = argv[2];
        serverPort = atoi(argv[3]);
        numThreads = atoi(argv[4]);
        dbIp = argv[5];
        dbPort = atoi(argv[6]);
        user = argv[7];
        passwd = argv[8];
        db = argv[9];

        printf("%s", ICON.c_str());

        EventLoop loop;

        Application app(&loop, InetAddress(serverPort), serverName,
                        staticSrcPrefix, dbIp, dbPort, user, passwd, db);

        app.setNumThreads(numThreads);
        app.start();
        loop.loop();
    } else {
        printf(
            "Usage: %s serverName staticSrcPrefix "
            "serverPort numThreads "
            "[IPofMySQLServer[default: 127.0.0.1] PortofMySQLServer[default: "
            "3306] "
            "UsernameofMySQLServer[default: lutianen] "
            "PasswordofMySQLServer[default: lutianen] "
            "DatabaseofMySQLServer[default: user]]\n",
            argv[0]);
    }

    return 0;
}
