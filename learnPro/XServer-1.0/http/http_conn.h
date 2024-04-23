#ifndef _HTTP_CONN_H
#define _HTTP_CONN_H

#include <sys/epoll.h>
#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include "../log/log.h"
#include "../CGImysql/sql_connection_pool.h"


class http_conn
{
public:
    enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };
    enum HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,  
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    enum LINE_STATUS
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;
public:
    http_conn()=default;
    ~http_conn()=default;
public:
    static void setepollfd(int epollfd);
    void initmysql_result(connection_pool *connPool);
    //主要
    void init(int sockfd, const sockaddr_in &addr);
    void process();

    void close_conn(bool real_close = true);
    bool read_once();
    bool write();

    sockaddr_in *get_address()
    {
        return &_address;
    }
private:
    void init();
    void unmap();
    HTTP_CODE process_read();
    bool process_write(HTTP_CODE ret); 
    char *get_line() { return _read_buf + _start_line; };
    LINE_STATUS parse_line();
    //解析请求行
    HTTP_CODE parse_request_line(char *text);
    //解析请求头
    HTTP_CODE parse_headers(char *text);
    //解析请求内容
    HTTP_CODE parse_content(char *text);
    HTTP_CODE do_request();

    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content(const char *content);

    bool add_response(const char *format, ...);
    bool add_content_type();
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();
public:
    static int m_epollfd;
    static int m_user_count;
    Sqlcon *mysql;
private:
    int _sockfd;
    sockaddr_in _address;

    char _read_buf[READ_BUFFER_SIZE];           //读数据缓存
    int _read_idx;                              //读数据缓存位置

    int _checked_idx;                           
    int _start_line;
    char _write_buf[WRITE_BUFFER_SIZE];
    int _write_idx;
    CHECK_STATE _check_state;
    METHOD _method;
    char _real_file[FILENAME_LEN];
    char *_url;
    char *_version;
    char *_host;
    int _content_length;
    bool _linger;
    char *_file_address;
    struct stat _file_stat;
    struct iovec _iv[2];
    int _iv_count;
    int cgi;        //是否启用的POST
    char *_string; //存储请求头数据
    int bytes_to_send;
    int bytes_have_send;
};





#endif