#include "sql_connection_pool.h"

using namespace std;

connection_pool::connection_pool()
{
    this->CurConn = 0;
    this->FreeConn = 0;
}

connection_pool *connection_pool::GetInstance()
{
	static connection_pool connPool;
	return &connPool;
}

void connection_pool::init(string url, string User, string PassWord, string DataBaseName, int Port, unsigned int MaxConn)
{
    this->url = url;
    this->Port = Port;
    this->User = User;
    this->PassWord = PassWord;
    this->DatabaseName = DataBaseName;

    lock.lock();
    for (int i = 0; i < MaxConn; i++)
    {
        Sqlcon *con = new Sqlcon(url.c_str(), User.c_str(), PassWord.c_str(), DataBaseName.c_str(), Port);

        connList.push_back(con);

        ++FreeConn;
    }
    
    reserve = sem(FreeConn);

    this->MaxConn = FreeConn;

    lock.unlock();
}

void connection_pool::DestroyPool()
{
    lock.lock();

    for (Sqlcon* con : connList)
    {
        con->destroy();
        delete con;
        con = NULL;
    }

    CurConn = 0;
    MaxConn = 0;
    FreeConn = 0;
    connList.clear();

    lock.unlock();
}

connection_pool::~connection_pool()
{
	DestroyPool();
}


Sqlcon *connection_pool::GetConnection()
{
    Sqlcon* con = NULL;

    if (0 == connList.size())
    {
        return NULL;
    }
    
    reserve.wait();

    lock.lock();

    con = connList.front();
    connList.pop_front();

    --FreeConn;
    ++CurConn;

    lock.unlock();
    return con;
}

//释放当前使用的连接
bool connection_pool::ReleaseConnection(Sqlcon *con)
{
	if (NULL == con)
		return false;

	lock.lock();

	connList.push_back(con);
	++FreeConn;
	--CurConn;

	lock.unlock();

	reserve.post();
	return true;
}


//当前空闲的连接数
int connection_pool::GetFreeConn()
{
	return this->FreeConn;
}



connectionRAII::connectionRAII(Sqlcon **SQL, connection_pool *connPool){
	*SQL = connPool->GetConnection();
	
	conRAII = *SQL;
	poolRAII = connPool;
}

connectionRAII::~connectionRAII(){
	poolRAII->ReleaseConnection(conRAII);
}