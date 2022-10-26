/**
 * @file LuxMySQL.cc
 * @author Tianen Lu (tianenlu@stu.xidian.edu.cn)
 * @brief 
 * @version 0.1
 * @date 2022-10-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "LuxMySQL.h"

#include "LuxUtils.h"

// #include <memory>
#include <cstring>  /// memset strstr strncpy strcpy
#include <cstdarg>  /// va_list
#include <cstdio>   /// vsnsprintf snprintf
#include <ctype.h>  /// isdigit

#include <iostream>

/**********************************************************************/
connection::connection()
{
    conn_ = nullptr;
    state_ = 0;
    ::memset(&env_, 0, sizeof(LOGINENV));
    ::memset(&cda_, 0, sizeof(cda_));

    cda_.rc = -1;
    ::strncpy(cda_.message, "database not open.", 128);

    // 
    ::memset(dbType_, 0, sizeof(dbType_));
    ::strcpy(dbType_, "mysql");
}

connection::~connection()
{
    disconnect();
}

/**
 * @brief Parse ip, username, password, dbname, port
 * 
 * @param connstr "127.0.0.1","lutianen","lutianen","mysql",3306
 */
void 
connection::setDbOpt(char *connstr)
{
    ::memset(&env_, 0, sizeof(LOGINENV));

    // begin position
    // end position
    char *bpos, *epos;
    bpos = epos = 0;

    // ip
    bpos = connstr;
    epos = ::strstr(bpos, ",");
    if (epos != nullptr) 
    {
        ::strncpy(env_.ip, bpos, static_cast<size_t>(epos - bpos));
    } else return;

    // user
    bpos = epos + 1;
    epos = 0;
    epos = ::strstr(bpos, ",");
    if (epos != nullptr) {
        ::strncpy(env_.user, bpos, static_cast<size_t>(epos - bpos));
    } else return;

    // pass
    bpos = epos + 1;
    epos = 0;
    epos = ::strstr(bpos, ",");
    if (epos != nullptr) {
        ::strncpy(env_.pass, bpos, static_cast<size_t>(epos - bpos));
    } else return;


    // dbname
    bpos = epos + 1;
    epos = 0;
    epos = ::strstr(bpos, ",");
    if (epos != nullptr) {
        ::strncpy(env_.dbname, bpos, static_cast<size_t>(epos - bpos));
    } else return;

    // port
    env_.port = static_cast<unsigned int>(atoi(epos + 1));
}

int connection::connectToDb(
    char *connstr,
    char *charset,
    unsigned int autoCommitOpt)
{
    // If the state is 1 (connected), It doesn't connect
    if (state_ == 1) 
        return 0;

    // Parse connstr
    setDbOpt(connstr);

    ::memset(&cda_, 0, sizeof(cda_));

    // Initial mysql
    if ((conn_ = mysql_init(nullptr)) == nullptr) {
        cda_.rc = -1;
        ::strncpy(cda_.message, "initialize mysql failed.\n", 128);
        return -1;
    }

    if (mysql_real_connect(conn_, env_.ip, env_.user, env_.pass, env_.dbname, env_.port, nullptr, 0) != conn_)
    {
        cda_.rc = static_cast<int>(mysql_errno(conn_));
        ::strncpy(cda_.message, mysql_error(conn_), 2000);
        mysql_close(conn_);
        conn_ = nullptr;
        return -1;
    }

    // Set auto commit flag
    autoCommitOpt_ = static_cast<int>(autoCommitOpt);

    if(mysql_autocommit(conn_, autoCommitOpt_) != 0)
    {
        cda_.rc = static_cast<int>(mysql_errno(conn_));
        ::strncpy(cda_.message, mysql_error(conn_), 2000);
        mysql_close(conn_);
        conn_ = nullptr;
        return -1;
    }

    character(charset);
    state_ = 1;

    return 0;
}


/**
 * @brief Set the character set to be samilar to database,
 * or random character will be displayed
 * @param charset 
 */
void
connection::character(char* charset)
{
    mysql_set_character_set(conn_, charset);
    return;
}


int
connection::disconnect()
{
    ::memset(&cda_, 0, sizeof(cda_));
    if (state_ == 0)
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "database not open.", 128);
        return -1;
    }

    rollback();
    mysql_close(conn_);
    conn_ = nullptr;
    state_ = 0;
    return 0;
}


int
connection::rollback()
{
    ::memset(&cda_, 0, sizeof(cda_));
    if(state_ == 0) 
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "database not open.", 128);
        return -1;
    }

    if (mysql_rollback(conn_) != 0)
    {
        cda_.rc = static_cast<int>(mysql_errno(conn_));
        ::strncpy(cda_.message, mysql_error(conn_), 2000);

        mysql_close(conn_);
        conn_ = nullptr;
        return -1;
    }

    return 0;
}

int
connection::commit()
{
    ::memset(&cda_, 0, sizeof(cda_));

    if(state_ == 0) 
    {

        cda_.rc = -1;
        ::strncpy(cda_.message, "database not open.", 128);
        return -1;
    }

    if (mysql_commit(conn_) != 0) 
    {
        cda_.rc = static_cast<int>(mysql_errno(conn_));
        ::strncpy(cda_.message, mysql_error(conn_), 2000);

        mysql_close(conn_);
        conn_ = nullptr;
        return -1;
    }

    return 0;
}


void
connection::err_report()
{
    if(state_ == 0) 
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "database not open.", 128);
        return ;
    }

    ::memset(&cda_, 0, sizeof(cda_));

    cda_.rc = -1;
    ::strncpy(cda_.message, "call err_report failed.", 128);
    
    cda_.rc = static_cast<int>(mysql_errno(conn_));
    ::strncpy(cda_.message, mysql_error(conn_), 2000);

    return;
}


int
connection::execute(const char *fmt,
                    ...)
{
    ::memset(sql_, 0, sizeof(sql_));

    /// ...
    va_list ap;
    va_start(ap, fmt);
    ::vsnprintf(sql_, 10240, fmt, ap);

    sqlstatement stmt(this);

    return stmt.execute(sql_);
}
/**********************************************************************/





sqlstatement::sqlstatement()
{
    initial();
}

void
sqlstatement::initial()
{
    state_ = 0;
    handle_ = nullptr;
    ::memset(&cda_, 0, sizeof(cda_));
    ::memset(sql_, 0, sizeof(sql_));

    cda_.rc = -1;
    ::strncpy(cda_.message, "sqlstatement not connect to connection.\n", 128);
}


sqlstatement::sqlstatement(connection *conn)
{
    initial();
    connect(conn);
}


sqlstatement::~sqlstatement() 
{
    disconnect();
}


int 
sqlstatement::connect(connection *conn) 
{
    /// If the state is 1 (connected), It doesn't need to connect.
    /// Do not allow to connect to more than one database
    if (state_ == 1)
        return 0;

    ::memset(&cda_, 0, sizeof(cda_));
    conn_ = conn;

    /// Return -1 (fail), if the pointer to database is nullptr
    if (nullptr == conn_)
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "database not open.", 128);
        return -1;
    }

    /// Return -1(fail), if database is not connected
    if (0 == conn_->state_)
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "database not open.", 128);
        return -1;
    }

    if ((handle_ = mysql_stmt_init(conn_->conn_)) == nullptr)
    {
        err_report();
        return cda_.rc;
    }

    state_ = 1;
    autoCommitOpti_ = conn_->autoCommitOpt_;
    return 0;
}


int
sqlstatement::disconnect()
{
    if (0 == state_)
        return 0;

    ::memset(&cda_, 0, sizeof(cda_));

    mysql_stmt_close(handle_);
    state_ = 0;
    handle_ = nullptr;
    ::memset(&cda_, 0, sizeof(cda_));
    ::memset(sql_, 0, sizeof(sql_));
    cda_.rc = -1;
    ::strncpy(cda_.message, "cursor not open.", 128);

    return 0;
}


/// @brief In this function, ::memset(&cda_, 0, sizeof(cda_)) will clear cda.rpc.
///     Don't use ::memset arbitrarily.
void
sqlstatement::err_report()
{
    if (0 == state_) 
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "cursor not open.\n", 128);
        return;
    }

    ::memset(&conn_->cda_, 0, sizeof(conn_->cda_));

    cda_.rc = -1;
    ::strncpy(cda_.message, "call err_report() failed.\n", 128);

    cda_.rc = static_cast<int>(mysql_stmt_errno(handle_));

    ::snprintf(cda_.message, 2000, 
        "%d, %s", cda_.rc, mysql_stmt_error(handle_));
    
    conn_->err_report();

    return;
}


int
sqlstatement::prepare(const char *fmt, 
                    ...) 
{
    ::memset(&cda_, 0, sizeof(cda_));
    if (0 == state_)
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "cursor not open.\n", 128);
        return -1;
    }

    ::memset(sql_, 0, sizeof(sql_));

    va_list ap;
    va_start(ap, fmt);
    ::vsnprintf(sql_, 10240, fmt, ap);
    va_end(ap);

    /// For compatibility with oracle, replace :1,:2,:3...etc with ?
    size_t sqlLen = ::strlen(sql_);
    for (size_t i = 0; i < sqlLen; ++i)
    {
        if( (sql_[i] == ':') && (::isdigit(sql_[i + 1]) != 0))
        {
            sql_[i] = '?';
            sql_[i + 1] = ' ';
            if (::isdigit(sql_[i + 2]) != 0)
                sql_[i + 2] = ' ';
            if (::isdigit(sql_[i + 3]) != 0)
                sql_[i + 3] = ' ';
        }
    }

    /// XXX For compatibility with oracle
    /// 1. "to_date" -> "str_to_date"
    /// 2. "to_char" -> "data_format"
    if (::strstr(sql_, "str_to_date") == nullptr)
        LuxUtils::LuxUpdateStr(sql_, "to_date", "str_to_date", false);
    LuxUtils::LuxUpdateStr(sql_, "to_char", "date_format", false);
    LuxUtils::LuxUpdateStr(sql_, "yyyy-mm-dd hh24:mi:ss", "%Y-%m-%d %h:%i:%s", false);
    LuxUtils::LuxUpdateStr(sql_, "yyyymmddhh24miss", "%Y%m%d%h%i%s", false);
    /// 一定要把格式一一列出来，不能用"yyyy"替换"%Y"，因为在SQL语句的其它地方也可能存在"yyyy"。
    /// 3. ...
    
    if (mysql_stmt_prepare(handle_, sql_, ::strlen(sql_)) != 0)
    {
        err_report();
        return cda_.rc;
    }

    /// sql_ is query statement or not, if true sqlType_ = 0, others 1
    sqlType_ = 1;
    /// 1. Get the first 30 characters from sql_.
    /// 2. It is query statement if "select" in the fist.
    char strTemp[31];
    ::memset(strTemp, 0, sizeof(strTemp));
    ::strncpy(strTemp, sql_, 30);
    LuxUtils::LuxToUpper(strTemp);
    LuxUtils::LuxDeleteLeftChar(strTemp, ' ');
    if (strncmp(strTemp, "SELECT", 6) == 0)
        sqlType_ = 0;
    
    ::memset(params_in_, 0, sizeof(params_in_));
    ::memset(params_out_, 0, sizeof(params_out_));
    maxBindIn_ = 0;
    return 0;
}



/// @brief
///  @usage:stmt.prepare("
///        insert grils (id, name, weight, btime)
///             VALUES (:1, :2, :3, str_to_date(:4, '%%Y-%%m-%%d %%h:%%i:%%s'))");
///         stmt.bindin(1, &stgrils.id);
///         stmt.bindin(2, stgrils.name, 30);
///         stmt.bindin(3, &stgrils.weight);
///         stmt.bindin(4, stgrils.btime, 19);
/// @param position 
/// @param value 
/// @return 
int
sqlstatement::bindin(unsigned int position, int *value)
{
    if ((position < 1) || (position >= MAXPARAMS) || (position > handle_->param_count))
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "array bound", 128);
    }

    params_in_[position - 1].buffer_type = MYSQL_TYPE_LONG;
    params_in_[position - 1].buffer = value;
    params_in_buffer_type_[position - 1] = MYSQL_TYPE_LONG;

    if (position > maxBindIn_)
        maxBindIn_ = position;

    return 0;
}


int 
sqlstatement::bindin(unsigned int position, long *value)
{
    if ((position < 1) || (position >= MAXPARAMS) || (position > handle_->param_count))
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "array bound", 128);
    }

    params_in_[position - 1].buffer_type = MYSQL_TYPE_LONGLONG;
    params_in_[position - 1].buffer = value;
    params_in_buffer_type_[position - 1] = MYSQL_TYPE_LONGLONG;
    
    if (position > maxBindIn_)
        maxBindIn_ = position;

    return 0;
}

int 
sqlstatement::bindin(unsigned int position, unsigned int *value)
{
    if ((position < 1) || (position >= MAXPARAMS) || (position > handle_->param_count))
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "array bound", 128);
    }

    params_in_[position - 1].buffer_type = MYSQL_TYPE_LONG;
    params_in_[position - 1].buffer = value;
    params_in_buffer_type_[position - 1] = MYSQL_TYPE_LONG;
    
    if (position > maxBindIn_)
        maxBindIn_ = position;

    return 0;
}

int
sqlstatement::bindin(unsigned int position, unsigned long int *value)
{
    if ((position < 1) || (position >= MAXPARAMS) || (position > handle_->param_count))
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "array bound", 128);
    }

    params_in_[position - 1].buffer_type = MYSQL_TYPE_LONGLONG;
    params_in_[position - 1].buffer = value;
    params_in_buffer_type_[position - 1] = MYSQL_TYPE_LONGLONG;
    
    if (position > maxBindIn_)
        maxBindIn_ = position;

    return 0;
}



int
sqlstatement::bindin(unsigned int position, char *value, unsigned int len)
{
    if ((position < 1) || (position >= MAXPARAMS) || (position > handle_->param_count))
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "array bound", 128);
    }

    params_in_[position - 1].buffer_type = MYSQL_TYPE_VAR_STRING;
    params_in_[position - 1].buffer = value;
    params_in_[position - 1].buffer_length = len;
    params_in_[position - 1].length = &params_in_length_[position - 1];
    params_in_[position - 1].is_null = &params_in_is_null_[position - 1];
    params_in_buffer_type_[position - 1] = MYSQL_TYPE_VAR_STRING;
    
    if (position > maxBindIn_)
        maxBindIn_ = position;

    return 0;
}

int 
sqlstatement::bindin(unsigned int position, float *value)
{
    if ((position < 1) || (position >= MAXPARAMS) || (position > handle_->param_count))
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "array bound.", 128);
    }

    params_in_[position - 1].buffer_type = MYSQL_TYPE_FLOAT;
    params_in_[position - 1].buffer = value;
    params_in_buffer_type_[position - 1] = MYSQL_TYPE_FLOAT;

    if (position > maxBindIn_)
        maxBindIn_ = position;

    return 0;
}

int 
sqlstatement::bindin(unsigned int position,double *value)
{
    if ((position < 1) || (position >= MAXPARAMS) || (position > handle_->param_count))
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "array bound.", 128);
    }

    params_in_[position - 1].buffer_type = MYSQL_TYPE_DOUBLE;
    params_in_[position - 1].buffer = value;
    params_in_buffer_type_[position - 1] = MYSQL_TYPE_DOUBLE;

    if (position > maxBindIn_)
        maxBindIn_ = position;

    return 0;
}

///////////////////
int 
sqlstatement::bindout(unsigned int position, int *value)
{
    if ((position < 1) || (position >= MAXPARAMS) || (position > handle_->field_count))
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "array bound.", 128);
    }

    params_out_[position - 1].buffer_type = MYSQL_TYPE_LONG;
    params_out_[position - 1].buffer = value;

    return 0;
}

int 
sqlstatement::bindout(unsigned int position, long *value)
{
    if ((position < 1) || (position >= MAXPARAMS) || (position > handle_->field_count))
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "array bound.", 128);
    }

    params_out_[position - 1].buffer_type = MYSQL_TYPE_LONGLONG;
    params_out_[position - 1].buffer = value;

    return 0;
}

int 
sqlstatement::bindout(unsigned int position, unsigned int *value)
{
    if ((position < 1) || (position >= MAXPARAMS) || (position > handle_->field_count))
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "array bound.", 128);
    }

    params_out_[position - 1].buffer_type = MYSQL_TYPE_LONG;
    params_out_[position - 1].buffer = value;

    return 0;
}

int 
sqlstatement::bindout(unsigned int position, unsigned long *value)
{
    if ((position < 1) || (position >= MAXPARAMS) || (position > handle_->field_count))
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "array bound.", 128);
    }

    params_out_[position - 1].buffer_type = MYSQL_TYPE_LONGLONG;
    params_out_[position - 1].buffer = value;

    return 0;
}

int 
sqlstatement::bindout(unsigned int position, char *value, unsigned int len)
{
    if ((position < 1) || (position >= MAXPARAMS) || (position > handle_->field_count))
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "array bound.", 128);
    }

    params_out_[position - 1].buffer_type = MYSQL_TYPE_VAR_STRING;
    params_out_[position - 1].buffer = value;
    params_out_[position - 1].buffer_length = len;

    return 0;
}

int 
sqlstatement::bindout(unsigned int position, float *value)
{
    if ((position < 1) || (position >= MAXPARAMS) || (position > handle_->field_count))
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "array bound.", 128);
    }

    params_out_[position - 1].buffer_type = MYSQL_TYPE_FLOAT;
    params_out_[position - 1].buffer = value;

    return 0;
}

int 
sqlstatement::bindout(unsigned int position, double *value)
{
    if ((position < 1) || (position >= MAXPARAMS) || (position > handle_->field_count))
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "array bound.", 128);
    }

    params_out_[position - 1].buffer_type = MYSQL_TYPE_DOUBLE;
    params_out_[position - 1].buffer = value;

    return 0;
}

int
sqlstatement::execute()
{
    ::memset(&cda_, 0, sizeof(cda_));

    if(0 == state_) 
    {
        cda_.rc = -1;
        ::strncpy(cda_.message, "cursor not open.\n", 128);
        return -1;
    }

    /// Bind in
    if ( (handle_->param_count > 0)
            && (handle_->bind_param_done == 0))
    {
        if (mysql_stmt_bind_param(handle_, params_in_) != 0)
        {
            err_report();
            return cda_.rc;
        }
    }

    /// Bind out
    if ((handle_->field_count > 0) 
        && (handle_->bind_result_done == 0))
    {
        if (mysql_stmt_bind_result(handle_, params_out_) != 0)
        {
            err_report();
            return cda_.rc;
        }
    }

    for (unsigned int i = 0; i < maxBindIn_; ++i) 
    {
        if (MYSQL_TYPE_VAR_STRING == params_in_buffer_type_[i])
        {
            if (::strlen(reinterpret_cast<char *>(params_in_[i].buffer)) == 0)
            {
                // params_in_[i].buffer_type = MYSQL_TYPE_NULL;
                params_in_is_null_[i] = true;
            }
            else
            {
                // params_in_[i].buffer_type = MYSQL_TYPE_VAR_STRING;
                params_in_is_null_[i] = false;
                params_in_length_[i] = ::strlen(reinterpret_cast<char *>(params_in_[i].buffer));
            }
        }
    }

    if (mysql_stmt_execute(handle_) != 0) 
    {
        err_report();
        return cda_.rc;
    }

    // If sql_ is not query statement, Get the affected rows
    if (sqlType_ == 1) 
    {
        cda_.rpc = handle_->affected_rows;
        conn_->cda_.rpc = cda_.rpc;
    }

    return 0;
}

int 
sqlstatement::execute(const char *fmt, ...)
{
    char strtmpsql[10241];
    ::memset(strtmpsql, 0, sizeof(strtmpsql));

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(strtmpsql, 10240, fmt, ap);
    va_end(ap);

    if (prepare(strtmpsql) != 0)
        return cda_.rc;

    return execute();
}

int 
sqlstatement::next()
{
    // 注意，在该函数中，不可随意用memset(&m_cda,0,sizeof(m_cda))，否则会清空m_cda.rpc的内容
    if (state_ == 0)
    {
        cda_.rc = -1;
        strncpy(cda_.message, "cursor not open.\n", 128);
        return -1;
    }

    // 如果语句未执行成功，直接返回失败。
    if (cda_.rc != 0)
        return cda_.rc;

    // 判断是否是查询语句，如果不是，直接返回错误
    if (sqlType_ != 0)
    {
        cda_.rc = -1;
        strncpy(cda_.message, "no recordset found.\n", 128);
        return -1;
    }

    int ret = mysql_stmt_fetch(handle_);

    if (ret == 0)
    {
        cda_.rpc++;
        return 0;
    }

    if (ret == 1)
    {
        err_report();
        return cda_.rc;
    }

    if (ret == MYSQL_NO_DATA)
        return MYSQL_NO_DATA;

    if (ret == MYSQL_DATA_TRUNCATED)
    {
        cda_.rpc++;
        return 0;
    }

    return 0;
}
