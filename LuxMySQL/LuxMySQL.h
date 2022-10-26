/**
 * @file LuxMySQL.h
 * @author Tianen Lu (tianenlu@stu.xidian.edu.cn)
 * @brief 
 * @version 0.1
 * @date 2022-10-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef __LUXMYSQL_H
#define __LUXMYSQL_H


/// MySQL 数据库接口函数头文件
#include <mysql/mysql.h>

// 执行SQL语句前绑定输入或输出变量个数的最大值，256是很大的了，可以根据实际情况调整。
#define MAXPARAMS  256

/**
 * @brief The environment of logging in MySQL
 */
struct LOGINENV
{
    /* The IP address of MySQL database */
    char ip[32];

    /* The socket port to connect to MySQL */
    unsigned int port;

    /* user */
    char user[32];

    /* password */
    char pass[32];

    /* Default database after logging */
    char dbname[51];
};


/**
 * @brief The result of calling the interface function 
 */
struct CDA_DEF
{
    /* 0 if successful else others */
    int rc;

    /* If interface function is "insert" or "update" or "delete",
     * it is the number of line for effecting;
     * if interface function is "select", 
     * it is the number of line for the set results. */
    unsigned long long int rpc;

    /* 执行SQL语句如果失败，存放错误描述信息。 */
    char message[2048];
};

/**
 * @brief MySQL数据库连接池类。
 */
class connection
{
private:
    /**
     * @brief Parse ip, username, password, dbname, port form connstr
     * @param connstr "ip,username,password,dbname,port"
     */
    void setDbOpt(char* connstr);

    /**
     * @brief Set character set to be samilar to database,
     * or the random character will be dispalied.
     * @param charset 
     */
    void character(char* charset);

    /// The handler of server environment
    LOGINENV env_;

    /// The types of database, default: mysql
    char dbType_[21];

public:
    /// The state of connecting to database
    /// 0 - disconnected
    /// 1 - connected
    int state_;

    /// The last result of calling interface function
    CDA_DEF cda_;

    /// SQL语句的文本，最长不能超过10240字节。
    char sql_[10241];

    /// @brief Constructor
    connection();
    /// @brief Destructor
    ~connection();

    /// @brief connectToDb - login database
    /// @param connstr The login parameter of database, 
    ///     format: "ip, username, password, dbname, port"
    /// @param charset The character set of database,
    ///     e.g. "gbk"
    /// @param autoCommitOpt Whether to enable auto-commit, 
    ///     0 - disenable(default); 
    ///     1 - enable;
    /// @return 0 - sucessful,
    ///     other - failed(The fail code in cda_.rc, The fail descriptor in cda_.message)
    int 
    connectToDb(char* connstr, 
                    char* charset,
                    unsigned int autoCommitOpt = 0);

    /// @brief Commit transaction
    /// @return 0 - successful, others - fail (Programmers do not need to care about it)
    int 
    commit();

    /// @brief Rollback transaction
    /// @return 0 - successful, others - fail (Programmers do not need to care about it)
    int
    rollback();

    /// @brief Disconnect from database
    /// @return 0 - successful, others - fail (Programmers do not need to care about it)
    int
    disconnect();

    /// @brief Execute the SQL statement
    /// @param fmt 如果SQL语句不需要绑定输入和输出变量（无绑定变量、非查询语句），可以直接用此方法执行。
    /// 参数说明：这是一个可变参数，用法与printf函数相同。
    /// 返回值：0-成功，其它失败，失败的代码在m_cda.rc中，失败的描述在m_cda.message中，
    /// 如果成功的执行了非查询语句，在m_cda.rpc中保存了本次执行SQL影响记录的行数。
    /// 程序员必须检查execute方法的返回值。
    /// 在connection类中提供了execute方法，是为了方便程序员，在该方法中，也是用sqlstatement类来完成功能。
    /// @param  ...
    /// @return 
    int
    execute(const char* fmt, ...);

    /// -------------------
    /// The connect handler of MySQL
    MYSQL *conn_;

    /// The flag of auto commit
    /// 0 - disenable
    /// 1 - enable
    int autoCommitOpt_;

    /// Get the error infomation
    void err_report();
    /// -------------------
};


/// @brief The operation class
class sqlstatement
{
private:
    /// SQL statement handler
    MYSQL_STMT *handle_;

    /// Input params
    MYSQL_BIND params_in_[MAXPARAMS];
    unsigned long int params_in_length_[MAXPARAMS];

    /// NOTE Change "my_bool" to "bool"
    bool params_in_is_null_[MAXPARAMS];
    enum enum_field_types params_in_buffer_type_[MAXPARAMS];
    /// Output params
    MYSQL_BIND params_out_[MAXPARAMS];
    unsigned int maxBindIn_;

    /// The pointer to the connection pool of database
    connection *conn_;

    /// The type of sql statement: 0 - query statement, 1 - others
    int sqlType_;

    /// The flag of auto committing: 0 - disenable, 1 - enable
    int autoCommitOpti_;

    /// Error report
    void err_report();

    /// Initial the member variable
    void initial();

public:
    /// The bind state to the connection pool: 0 - disbind, 1 - bind
    int state_;

    /// SQL语句的文本，最长不能超过10240字节
    char sql_[10241];

    /// 执行SQL语句的结果
    CDA_DEF cda_;

    /// Constructor
    sqlstatement();
    /// @brief Constructor with binding the connection pool
    /// @param conn 
    sqlstatement(connection *conn);
    /// Destructor
    ~sqlstatement();

    /// @brief 绑定数据库连接池
    /// @param conn 数据库连接池connection对象的地址
    /// @return 0-成功，其它失败，
    ///         只要conn参数是有效的，并且数据库的游标资源足够，connect方法不会返回失败
    /// 注意，每个sqlstatement只需要绑定一次，在绑定新的connection前，必须先调用disconnect方法
    int connect(connection* conn);

    /// @brief 取消与数据库连接池的绑定。
    /// @return 0-成功，其它失败，程序员一般不必关心返回值。
    int disconnect();

    // 准备SQL语句。
    // 参数说明：这是一个可变参数，用法与printf函数相同。
    // 返回值：0-成功，其它失败，程序员一般不必关心返回值。
    // 注意：如果SQL语句没有改变，只需要prepare一次就可以了。
    int prepare(const char* fmt, ...);

    // 绑定输入变量的地址。
    // position：字段的顺序，从1开始，必须与prepare方法中的SQL的序号一一对应。
    // value：输入变量的地址，如果是字符串，内存大小应该是表对应的字段长度加1。
    // len：如果输入变量的数据类型是字符串，用len指定它的最大长度，建议采用表对应的字段长度。
    // 返回值：0-成功，其它失败，程序员一般不必关心返回值。
    // 注意：1）如果SQL语句没有改变，只需要bindin一次就可以了，2）绑定输入变量的总数不能超过256个。
    int bindin(unsigned int position, int    *value);
    int bindin(unsigned int position, long   *value);
    int bindin(unsigned int position, unsigned int  *value);
    int bindin(unsigned int position, unsigned long int *value);
    int bindin(unsigned int position, float *value);
    int bindin(unsigned int position, double *value);
    int bindin(unsigned int position, char   *value, unsigned int len);

    // 绑定输出变量的地址。
    // position：字段的顺序，从1开始，与SQL的结果集一一对应。
    // value：输出变量的地址，如果是字符串，内存大小应该是表对应的字段长度加1。
    // len：如果输出变量的数据类型是字符串，用len指定它的最大长度，建议采用表对应的字段长度。
    // 返回值：0-成功，其它失败，程序员一般不必关心返回值。
    // 注意：1）如果SQL语句没有改变，只需要bindout一次就可以了，2）绑定输出变量的总数不能超过256个。
    int bindout(unsigned int position, int    *value);
    int bindout(unsigned int position, long   *value);
    int bindout(unsigned int position, unsigned int  *value);
    int bindout(unsigned int position, unsigned long *value);
    int bindout(unsigned int position, float *value);
    int bindout(unsigned int position, double *value);
    int bindout(unsigned int position, char   *value,unsigned int len);

    // 执行SQL语句。
    // 返回值：0-成功，其它失败，失败的代码在m_cda.rc中，失败的描述在m_cda.message中。
    // 如果成功的执行了非查询语句，在m_cda.rpc中保存了本次执行SQL影响记录的行数。
    // 程序员必须检查execute方法的返回值。
    int execute();

    // 执行SQL语句。
    // 如果SQL语句不需要绑定输入和输出变量（无绑定变量、非查询语句），可以直接用此方法执行。
    // 参数说明：这是一个可变参数，用法与printf函数相同。
    // 返回值：0-成功，其它失败，失败的代码在m_cda.rc中，失败的描述在m_cda.message中，
    // 如果成功的执行了非查询语句，在m_cda.rpc中保存了本次执行SQL影响记录的行数。
    // 程序员必须检查execute方法的返回值。
    int execute(const char *fmt,...);

    // 从结果集中获取一条记录。
    // 如果执行的SQL语句是查询语句，调用execute方法后，会产生一个结果集（存放在数据库的缓冲区中）。
    // next方法从结果集中获取一条记录，把字段的值放入已绑定的输出变量中。
    // 返回值：0-成功，1403-结果集已无记录，其它-失败，失败的代码在m_cda.rc中，失败的描述在m_cda.message中。
    // 返回失败的原因主要有两个：1）与数据库的连接已断开；2）绑定输出变量的内存太小。
    // 每执行一次next方法，m_cda.rpc的值加1。
    // 程序员必须检查next方法的返回值。
    int next();
};


/*
delimiter $$
drop function if exists to_null;

create function to_null(in_value varchar(10)) returns decimal
begin
if (length(in_value)=0) then
  return null;
else
  return in_value;
end if;
end;
$$
*/

#endif // __LUXMYSQL_H




