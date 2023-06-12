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
tcpdump 抓包工具 : sudo tcpdump -i lo port 9999  检测回环地址  端口 9999 的tcp数据
nc   可以测试服务器  
```

#### 传输层协议

tcp 连接状态图:

![image-20230612211833556](README.assets/image-20230612211833556.png)

注解:TIME_WAIT : 客户端等待服务器重新发送FIN报文的时间，防止服务器没有收到最后一个ACK而重向客户端发消息时客户端可以收到，保证正常结束这个TCP连接。  一般为2MSL  所以这段时间，端口是被占用着的。

+ 1.复位报文段  一般表示关闭连接，或者重新建立连接，异常情况。