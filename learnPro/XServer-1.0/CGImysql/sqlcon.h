#ifndef _SQL_CON_H
#define _SQL_CON_H 
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <list>
#include <vector>
#include <mysql/mysql.h>
#include "../log/log.h"
using namespace std;

class Sqlcon
{
public:
    Sqlcon(std::string url, std::string User, std::string PassWord, std::string DataBaseName, int Port);
    ~Sqlcon();

    bool initConnect(std::string url, std::string User, std::string PassWord, std::string DataBaseName, int Port);
    bool querySql(std::string strsql, int cols);
    void close();
    void destroy();

    vector< vector<string> > getData(){ return _datas; }
public:
    MYSQL *_con;
    vector< vector<string> > _datas;
};


#endif