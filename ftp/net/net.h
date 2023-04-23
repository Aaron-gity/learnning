#ifndef __NET_H__
#define __NET_H__

#include <stddef.h>
#include <arpa/inet.h>

#define NET_BUFFER_SIZE 1024 // 网络缓冲区大小

// 初始化网络,创建套接字
int net_init();

// 绑定ip和port到一个套接字
int net_bind(int socket_fd, const char *ip, const int port);

// 连接到一个套接字
int net_connect(int socket_fd, const char *ip, const int port);

// 监听
int net_listen(int socket_fd, const int max_conn);

// 接收一个连接
int net_accept(int socket_fd, struct sockaddr_in *client_addr);

// 接收数据
int net_recv(const int socket_fd, void *buffer, size_t len, const int flag);

// 发送数据
int net_send(const int socket_fd, void *buffer, size_t len, const int flag);

// 将套接字设置为非阻塞模式
int net_set_nonblocking(const int socket_fd);

// 将套接字设置为端口重用
int net_set_reuseaddr(const int socket_fd);

// 错误处理
void net_error_handler(const char *msg);

// 判断文件是否存在
int net_file_exist(const char *filename);

// 发送文件到套接字
int net_send_file(const char *file_name, const int socket_fd);

// 从套接字中接收文件
int net_receive_file(const char *file_name, const int socket_fd);

#endif // !__NET_H__
