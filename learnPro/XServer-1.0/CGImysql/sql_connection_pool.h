#ifndef _SQL_CONNECTION_POOL_H
#define _SQL_CONNECTION_POOL_H

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <list>
#include <mysql/mysql.h>
#include "../lock/locker.h"
#include "sqlcon.h"

using namespace std;

class connection_pool
{
public:
    Sqlcon *GetConnection();
    bool ReleaseConnection(Sqlcon *conn);
    int GetFreeConn();
    void DestroyPool();
    
    static connection_pool* GetInstance();

    void init(string url, string User, string PassWord, string DataBaseName, int Port, unsigned int MaxConn);

    connection_pool();
    ~connection_pool();
private:
    unsigned int MaxConn;
    unsigned int CurConn;
    unsigned int FreeConn;

private:
    locker lock;
    list<Sqlcon *> connList;
    sem reserve;
private:
    string url;
    string Port;
    string User;
    string PassWord;
    string DatabaseName;
};


class connectionRAII
{
public:
    connectionRAII(Sqlcon **con, connection_pool *connPool);
    ~connectionRAII();
private:
    Sqlcon *conRAII;
    connection_pool *poolRAII;
};


#endif