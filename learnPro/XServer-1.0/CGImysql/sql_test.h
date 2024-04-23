#ifndef _SQL_TEST_H
#define _SQL_TEST_H
#include "sql_connection_pool.h"

const char *url = "192.168.59.133";
const char *user = "root";
const char *passwd = "123";
const char *dbname = "yourdb";
const unsigned int port = 3306;

static void test_fun_sql()
{
    Log::get_instance()->init("xupan");
    connection_pool* pool =  connection_pool::GetInstance();
    pool->init(url, user, passwd, dbname, port, 5);

    Sqlcon *con =  pool->GetConnection();
    con->querySql("select * from user", 2);

    vector< vector<string> > datas = con->getData();
    for (auto arow: datas)
    {
        for (auto str : arow)
        {
            std::cout << str << std::endl;
        }
        
    }
    

}


#endif