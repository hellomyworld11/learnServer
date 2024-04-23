#ifndef __XSERVER_H
#define __XSERVER_H
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/epoll.h>
#include <string.h>
#include <cassert>
#include "./log/log.h"
#include "./http/http_conn.h"
#include "./CGImysql/sql_connection_pool.h"
#include "./threadpool/threadpool.h"
#include "./timer/lst_timer.h"

//#define connfdET //边缘触发非阻塞
#define connfdLT //水平触发阻塞
//#define listenfdET //边缘触发非阻塞
#define listenfdLT //水平触发阻塞

#define SYNLOG                  //同步写日志
#define MAX_FD 65536            //最大文件描述符
#define TIMESLOT 5              //最小超时单位
#define MAX_EVENT_NUMBER 10000  //最大事件数

class XServer{
public:
    XServer(char *ip,int port); 
    void start();
    static void addfd(int epollfd, int fd, bool one_shot);
    static void modfd(int epollfd, int fd, int ev);
    static void removefd(int epollfd, int fd);
    static int  setnonblocking(int fd);
private:
    bool init();
    bool bindinit(); 
    void loginit();
    void conpoolinit();
    void threadpoolinit();
    void addsig(int sig, void(handler)(int), bool restart = true);
    void pipeinit();
    void newconnection();
    void closeconnection(int connfd);
    void signalprocess(bool& bstopserver, bool& btimeout);
    void readprocess(int sockfd);
    void writeprocess(int sockfd);
    void show_error(int connfd, const char* info);
    void timer_handler();
public:
    static int          epollfd;
private:
    char *       _ip;
    unsigned int _thread_num;
    int          _port; 
    int          _listenfd;
    connection_pool *_connPool;
    threadpool<http_conn> *_pool;
    client_data *_users_timer;
    http_conn        *_users;
};

#endif