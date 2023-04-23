#include "net.h"

#include <sys/socket.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

// 初始化网络,创建套接字
int net_init()
{
    // 创建socket
    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        net_error_handler("Error opening socket");
    }
    return socket_fd;
}

// 绑定ip和port到一个套接字
int net_bind(int socket_fd, const char *ip, const int port)
{
    // 绑定IP和端口
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    return bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
}

// 连接到一个套接字
int net_connect(int socket_fd, const char *ip, const int port)
{
    // 绑定IP和端口
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    // 连接服务器
    return connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
}

// 监听
int net_listen(int socket_fd, const int max_conn)
{
    // 监听
    return listen(socket_fd, max_conn);
}

// 接收一个连接
int net_accept(int socket_fd, struct sockaddr_in *client_addr)
{
    // 等待连接
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(socket_fd, (struct sockaddr *)client_addr, &addr_len);
    return client_fd;
}

// 接收数据
int net_recv(const int socket_fd, void *buffer, size_t len, const int flag)
{
    ssize_t bytes = recv(socket_fd, buffer, len, flag);
    if (bytes <= 0)
    {
        close(socket_fd);
    }
    return bytes;
}

// 发送数据
int net_send(const int socket_fd, void *buffer, size_t len, const int flag)
{
    ssize_t bytes = send(socket_fd, buffer, len, flag);
    if (bytes <= 0)
    {
        close(socket_fd);
    }
    return bytes;
}

// 将套接字设置为非阻塞模式
int net_set_nonblocking(const int socket_fd)
{
    return fcntl(socket_fd, F_SETFL, O_NONBLOCK);
}

// 将套接字设置为端口重用
int net_set_reuseaddr(const int socket_fd)
{
    int reuse = 1;
    return setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)); // 设置端口重用
}

// 错误处理
void net_error_handler(const char *msg)
{
    perror(msg);
    exit(1);
}

// 判断文件是否存在
int net_file_exist(const char *filename)
{
    struct stat buffer;
    if (lstat(filename, &buffer) < 0)
    {
        return -1;
    }
    if (S_ISDIR(buffer.st_mode))   // 排除目录文件
    {
        return -1;
    }
    return (stat(filename, &buffer) == 0);
}

// 发送文件到套接字
int net_send_file(const char *file_name, const int socket_fd)
{
    // 打开文件
    int fd = open(file_name, O_RDONLY);
    if (fd == -1)
    {
        perror("open file failed");
        return -1;
    }

    // 获取文件大小
    struct stat stat_buf;
    if (fstat(fd, &stat_buf) != 0)
    {
        perror("get file size failed");
        close(fd);
        return -1;
    }
    printf("File size: %lu\n", stat_buf.st_size);
    uint32_t file_size = htonl(stat_buf.st_size);

    // 发送文件大小
    if (net_send(socket_fd, &file_size, sizeof(file_size), 0) != sizeof(file_size))
    {
        perror("send file size failed");
        close(fd);
        return -1;
    }

    // 发送文件内容
    char buffer[NET_BUFFER_SIZE + 1] = {};
    int bytes_read;
    int total_bytes_read = 0;
    while ((bytes_read = read(fd, buffer, NET_BUFFER_SIZE)) > 0)
    {
        printf("send buffer: %d bytes\n", bytes_read);
        if (net_send(socket_fd, buffer, bytes_read, 0) != bytes_read)
        {
            perror("send file content failed");
            close(fd);
            return -1;
        }
        total_bytes_read += bytes_read;
    }
    printf("send total bytes: %d\n", total_bytes_read);
    if (bytes_read == -1)
    {
        perror("read file failed");
        close(fd);
        return -1;
    }

    // 关闭文件描述符
    close(fd);

    return 0;
}

// 从套接字中接收文件
int net_receive_file(const char *file_name, const int socket_fd)
{
    // 打开文件
    int file_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (file_fd == -1)
    {
        perror("open file failed");
        return -1;
    }

    // 接收文件大小
    uint32_t file_size;
    if (net_recv(socket_fd, &file_size, sizeof(file_size), 0) != sizeof(file_size))
    {
        perror("receive file size failed");
        close(file_fd);
        return -1;
    }
    file_size = ntohl(file_size);
    printf("File size: %u\n", file_size);

    // 接收文件内容并写入到文件描述符中
    char buffer[NET_BUFFER_SIZE + 1] = {};
    int bytes_read;
    int total_bytes_read = 0;
    while (total_bytes_read < file_size && (bytes_read = net_recv(socket_fd, buffer, NET_BUFFER_SIZE, 0)) > 0)
    {
        printf("receive buffer: %d bytes\n", bytes_read);
        if (write(file_fd, buffer, bytes_read) != bytes_read)
        {
            perror("write file failed");
            close(file_fd);
            return -1;
        }
        total_bytes_read += bytes_read;
    }
    printf("receive total bytes: %d\n", total_bytes_read);
    if (bytes_read == -1)
    {
        perror("receive file content failed");
        close(file_fd);
        return -1;
    }

    // 关闭文件描述符
    close(file_fd);

    return 0;
}
