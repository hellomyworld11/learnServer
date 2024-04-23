#include "xserver.h"

static int pipefd[2];
static sort_timer_lst timer_lst;
int XServer::epollfd = -1;


//信号处理函数
void sig_handler(int sig)
{
    //为保证函数的可重入性，保留原来的errno
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1], (char *)&msg, 1, 0); //pipefd[0] 被放入epoll检测
    errno = save_errno;
}

//定时器回调函数，删除非活动连接在socket上的注册事件，并关闭
void cb_func(client_data *user_data)
{
    epoll_ctl(XServer::epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);
    close(user_data->sockfd);
    http_conn::m_user_count--;
    LOG_INFO("close fd %d", user_data->sockfd);
    Log::get_instance()->flush();
}


XServer::XServer(char *ip,int port):
    _ip(ip),
    _port(port)
{
    init();
}

bool XServer::bindinit()
{
    struct sockaddr_in address;
    bzero(&address, sizeof(address));

    address.sin_port = htons(_port);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    int flag = 1;
    setsockopt(_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    int ret = bind(_listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret >= 0);

    ret = listen(_listenfd, 5);
    assert(ret >= 0);
    return true;
}

void XServer::loginit()
{
#ifdef ASYNLOG
    Log::get_instance()->init("log", 2000, 800000, 8);
#endif

#ifdef SYNLOG
    Log::get_instance()->init("log", 2000, 800000, 0);
#endif
    

}

void XServer::conpoolinit()
{
     //创建数据库连接池
    _connPool = connection_pool::GetInstance();
    _connPool->init("localhost", "root", "123", "yourdb", 3306, 8);
}

void XServer::threadpoolinit()
{
    //创建线程池
    _pool = new threadpool<http_conn>(_connPool);
}

 void XServer::addsig(int sig, void(handler)(int), bool restart)
 {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
        sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
 }

void XServer::addfd(int epollfd, int fd, bool one_shot)
{
    epoll_event event;
    event.data.fd = fd;
#ifdef connfdET
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
#endif
#ifdef connfdLT
    event.events = EPOLLIN | EPOLLRDHUP;
#endif

#ifdef listenfdET
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
#endif
#ifdef listenfdLT
    event.events = EPOLLIN | EPOLLRDHUP;
#endif

    if (one_shot)
        event.events |= EPOLLONESHOT;

    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    XServer::setnonblocking(fd);
}

void XServer::modfd(int epollfd, int fd, int ev)
{
    epoll_event event;
    event.data.fd = fd;

#ifdef connfdET
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP
#endif
#ifdef connfdLT
    event.events = ev| EPOLLONESHOT | EPOLLRDHUP; //EPOLLIN 改成EPOLLONESHOT  需要充值oneshot事件，不然多个线程处理同个sockfd乱套
#endif

    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);

}

void XServer::removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

int  XServer::setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    int cur_option = fcntl(fd, F_GETFL);
    return cur_option;
}

void XServer::pipeinit()
{
    //创建管道
    int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
    assert(ret != -1);
    setnonblocking(pipefd[1]);
    addfd(epollfd, pipefd[0], false);
}

bool XServer::init()
{
    //初始化日志配置
    loginit();
    LOG_INFO("ip:%s, port:%d", _ip, _port);
    _listenfd = socket(PF_INET,SOCK_STREAM,0);
    assert(_listenfd >= 0);
   
    addsig(SIGPIPE, SIG_IGN);
    //初始化数据连接池
    conpoolinit();
    //初始化线程池
    threadpoolinit();
  
    _users = new http_conn[MAX_FD];
    assert(_users);
    _users->initmysql_result(_connPool);
    
    //绑定网络信息
    bindinit();
    //创建内核事件表
    epollfd = epoll_create(5);
    assert(epollfd != -1);

    addfd(epollfd, _listenfd, false);
    http_conn::setepollfd(epollfd);

    //创建管道
    pipeinit();

     //定时器信号
    addsig(SIGALRM, sig_handler, false);
    //中断信号
    addsig(SIGTERM, sig_handler, false);
    
    //连接请求对应计时器
    _users_timer = new client_data[MAX_FD];

    alarm(TIMESLOT);
}

void XServer::newconnection()
{   
    struct sockaddr_in client_address;
    socklen_t client_addrlen = sizeof(client_address);
#ifdef listenfdLT
    int connfd = accept(_listenfd, (struct sockaddr *)&client_address, &client_addrlen);
    if (connfd < 0)
    {
        LOG_ERROR("%s:errno is :%d","accept error", errno);
        return;
    }
    if (http_conn::m_user_count >= MAX_FD)
    {
        show_error(connfd, "Internal server busy");
        LOG_ERROR("%s", "Internal server busy");
        return;
    }
    _users[connfd].init(connfd, client_address);

    //初始化定时器数据
    _users_timer[connfd].address = client_address;
    _users_timer[connfd].sockfd = connfd;
    util_timer *timer = new util_timer;
    timer->user_data = &_users_timer[connfd];
    timer->cb_func = cb_func;
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;
    _users_timer[connfd].timer = timer;
    timer_lst.add_timer(timer);
#endif

#ifdef listenfdET
    while (1)
    {
        int connfd = accept(_listenfd, (struct sockaddr *)&client_address, &client_addrlength);
        if (connfd < 0)
        {
            LOG_ERROR("%s:errno is:%d", "accept error", errno);
            break;
        }
        if (http_conn::m_user_count >= MAX_FD)
        {
            show_error(connfd, "Internal server busy");
            LOG_ERROR("%s", "Internal server busy");
            break;
        }
        users[connfd].init(connfd, client_address);

        //初始化client_data数据
        //创建定时器，设置回调函数和超时时间，绑定用户数据，将定时器添加到链表中
        users_timer[connfd].address = client_address;
        users_timer[connfd].sockfd = connfd;
        util_timer *timer = new util_timer;
        timer->user_data = &users_timer[connfd];
        timer->cb_func = cb_func;
        time_t cur = time(NULL);
        timer->expire = cur + 3 * TIMESLOT;
        users_timer[connfd].timer = timer;
        timer_lst.add_timer(timer);
    }          
#endif
}

void XServer::show_error(int connfd, const char* info)
{
    std::cout << info << std::endl;
    send(connfd, info, strlen(info), 0);
    close(connfd);
}

void XServer::closeconnection(int connfd)
{
    util_timer *timer = _users_timer[connfd].timer;
    timer->cb_func(&_users_timer[connfd]);
    if (timer)
    {
        timer_lst.del_timer(timer);
    }   
}

void XServer::signalprocess(bool& bstopserver, bool& btimeout)
{
     int sig;
    char signals[1024];
    int ret = recv(pipefd[0], signals, sizeof(signals), 0);
    if (ret == -1)
    {
        return;
    }
    else if (ret == 0)
    {
        return;
    }
    else
    {
        for (int i = 0; i < ret; ++i)
        {
            switch (signals[i])
            {
                case SIGALRM:
                {
                    btimeout = true;
                    break;
                }
                case SIGTERM:
                {
                    bstopserver = true;
                }
            }
        }
    }
}

void XServer::readprocess(int sockfd)
{
    util_timer *timer = _users_timer[sockfd].timer;
    if (_users[sockfd].read_once())
    {
        LOG_INFO("deal with the client(%s)", inet_ntoa(_users[sockfd].get_address()->sin_addr));
        Log::get_instance()->flush();
        //若监测到读事件，将该事件放入请求队列
        _pool->append(_users + sockfd);

        //若有数据传输，则将定时器往后延迟3个单位
        //并对新的定时器在链表上的位置进行调整
        if (timer)
        {
            time_t cur = time(NULL);
            timer->expire = cur + 3 * TIMESLOT;
            LOG_INFO("%s", "adjust timer once");
            Log::get_instance()->flush();
            timer_lst.adjust_timer(timer);
        }
    }
    else
    {
        timer->cb_func(&_users_timer[sockfd]);
        if (timer)
        {
            timer_lst.del_timer(timer);
        }
    }
}

void XServer::writeprocess(int sockfd)
{
    util_timer *timer = _users_timer[sockfd].timer;
    if (_users[sockfd].write())
    {
        LOG_INFO("send data to the client(%s)", inet_ntoa(_users[sockfd].get_address()->sin_addr));
        Log::get_instance()->flush();

        //若有数据传输，则将定时器往后延迟3个单位
        //并对新的定时器在链表上的位置进行调整
        if (timer)
        {
            time_t cur = time(NULL);
            timer->expire = cur + 3 * TIMESLOT;
            LOG_INFO("%s", "adjust timer once");
            Log::get_instance()->flush();
            timer_lst.adjust_timer(timer);
        }
    }
    else
    {
        timer->cb_func(&_users_timer[sockfd]);
        if (timer)
        {
            timer_lst.del_timer(timer);
        }
    }
}

//定时处理任务，重新定时以不断触发SIGALRM信号
void XServer::timer_handler()
{
    timer_lst.tick();
    alarm(TIMESLOT);
}

void XServer::start()
{
     //创建内核事件表
    epoll_event events[MAX_EVENT_NUMBER];
    bool timeout = false;
    bool stop_server = false;
    //epoll监控循环
    while (!stop_server)
    {
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if(number < 0 && errno != EINTR)
        {
            LOG_ERROR("%s", "epoll failure");
            break;
        }

        for (int i = 0; i < number; i++)
        {
            int sockfd = events[i].data.fd;

            if(sockfd == _listenfd)
            {
                newconnection();
            }
            else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                //服务器端关闭连接，移除对应的定时器
                closeconnection(sockfd);
            }
            else if ((sockfd == pipefd[0]) && (events[i].events & EPOLLIN))
            {   
                signalprocess(stop_server, timeout);
            }
            else if (events[i].events & EPOLLIN)
            {
                readprocess(sockfd);
            }
            else if(events[i].events & EPOLLOUT)
            {
                writeprocess(sockfd);
            }
        }
        
        if (timeout)
        {
            timer_handler();
            timeout = false;
        }
    }
    close(epollfd);
    close(_listenfd);
    close(pipefd[1]);
    close(pipefd[0]);
    delete[] _users;
    delete[] _users_timer;
    delete _pool;
    return ;
}
