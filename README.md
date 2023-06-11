### learnServer



#### process：

```
1.simple webserver
2.libevent   libevent-httpserver
3.others webserver 
4.own webserver
```

### 1.基础知识

分层模型：

```
+ 应用层		 特定协议实现特定功能
+ 运输层		 解决进程之间基于网络的通信问题 重传机制 有序
+ 网络层         解决分组在多个网络上传输的问题  
+ 数据链路层     解决分组在一个网络上传输的问题，比如从一个硬件地址传到另一个硬件地址
+ 物理层  		 何种信号传输比特
```

#### linux网络有关命令

```
host :查询DNS 域名获取ip
route: 查看路由表
etc/services  这个文件是知名服务端口号定义文件
```

#### 传输层协议

```

```



