#include "../LuxMySQL.h"

#include <iostream>
#include <string>

/**
 * docker run -p 3306:3306 --name LuxMysql -e MYSQL_ROOT_PASSWORD=lutianen -d mysql
 * 
 */

int 
main(int argc, char const *argv[])
{
    connection conn;

    if (conn.connectToDb(
            const_cast<char*>(std::string("127.0.0.1,root,lutianen,mysql,3306").c_str()), 
            const_cast<char*>(std::string("gbk").c_str())) != 0)
    {
        std::cout << "connect database failed." << conn.cda_.message << std::endl;
        return -1;
    }

    sqlstatement stmt(&conn);

    //  超女表girls，超女编号id，超女姓名name，体重weight，报名时间btime，超女说明memo，超女图片pic。
    stmt.prepare("\
        create table grils(id   bigint(10),\
                           name varchar(30),\
                            weihti  decimal(8, 2),\
                            btime   datetime,\
                            memo    longtext,\
                            pic     longblob,\
                            primary key (id))");
    
    if (stmt.execute() != 0)
    {
        std::cout << "stmt.execute() failed." 
            << stmt.sql_ << std::endl 
            << stmt.cda_.message << std::endl;

        return -1;
    }
    return 0;
}

