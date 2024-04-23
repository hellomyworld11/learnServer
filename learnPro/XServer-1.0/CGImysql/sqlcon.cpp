#include "sqlcon.h"

Sqlcon::Sqlcon(std::string url, std::string User, std::string PassWord, std::string DataBaseName, int Port)
    :_con(NULL)
{
    if(!initConnect(url, User, PassWord, DataBaseName, Port))
    {    
        LOG_ERROR("initConnect[%s,%s,%s] failed", url.c_str(), User.c_str(), DataBaseName.c_str());
        exit(1);
    }
    LOG_INFO("initConnect[%s,%s,%s] success", url.c_str(), User.c_str(), DataBaseName.c_str());
}

Sqlcon::~Sqlcon()
{
    destroy();
}


bool Sqlcon::initConnect(std::string url, std::string User, std::string PassWord, std::string DataBaseName, int Port)
{
    _con = mysql_init(_con);
    if (_con == NULL)
    {
        return false;
    }
    
    _con = mysql_real_connect(_con, url.c_str(), User.c_str(), PassWord.c_str(), DataBaseName.c_str(), Port, NULL, 0);
    if (_con == NULL)
    {
        return false;
    }
    
    return true;
}

bool Sqlcon::querySql(std::string strsql, int cols)
{
    if(mysql_query(_con, strsql.c_str()))
    {
        LOG_ERROR("mysql_query error:%s \n", mysql_error(_con));
        return false;
    }

    MYSQL_RES* result = mysql_store_result(_con);
    if (result == NULL)
    {
        return true;
    }
    
    int num_fields = mysql_num_fields(result);
    
    int rows = mysql_num_rows(result);

    MYSQL_FIELD *field =  mysql_fetch_fields(result);

     //从结果集中获取下一行，将对应的用户名和密码，存入map中
    while (MYSQL_ROW row = mysql_fetch_row(result))
    {
        vector<string> aRow;
        for (int i = 0; i < cols; i++)
        {
            aRow.push_back(row[i]);
        }
        _datas.push_back(aRow);  
    }
    mysql_free_result(result);
    return true;
}


void Sqlcon::close()
{
    mysql_close(_con);
}

void Sqlcon::destroy()
{
    close();
    _datas.clear();
    _con = NULL;
}


