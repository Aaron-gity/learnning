#include "ftp_session.h"
#include "../../net/net.h"
#include "../../event/event.h"
#include "../../command/command.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <unistd.h>

// 实现服务器的命令处理
void handle_list_command(Command_t *cmd_p); // 发送指定文件夹的所有文件名称
void handle_get_command(Command_t *cmd_p);  // 根据文件名称发送文件到客户端
void handle_put_command(Command_t *cmd_p);  // 将上传的文件保存到指定的文件夹中
void handle_quit_command(Command_t *cmd_p); // 终止当前连接

COMMAND_HANDLERS handlers =
    {
        default_help_command,
        handle_list_command,
        handle_get_command,
        handle_put_command,
        handle_quit_command};


// 发送响应
static int send_response(int socket_fd, char *format, ...);

// 接收请求
static int recv_request(int socket_fd, char *buffer, int buffer_size);

// 处理请求
void ftp_process(void *event_p)
{
    Event_t *ev = (Event_t *)event_p;
    int status = 0;
    Command_t cmd = {};
    cmd.socket_fd = ev->fd;

    status = recv_request(cmd.socket_fd, cmd.buffer, sizeof(cmd.buffer) - 1);

    if (status <= 0)
    {
        event_free(ev);
        return;
    }

    printf("fpt server> %s\n", cmd.buffer);

    handle_command(cmd, handlers);
}

// 发送响应
static int send_response(int socket_fd, char *format, ...)
{
    char buffer[BUFFER_SIZE + 1] = {};
    va_list args;
    int status = 0;

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    uint32_t buffer_size = strlen(buffer);

    // 先发送四个字节的长度
    uint32_t send_head = htonl(buffer_size);
    status = net_send(socket_fd, &send_head, sizeof(uint32_t), 0);
    // 再发送数据
    status = net_send(socket_fd, buffer, buffer_size, 0);

    printf("send response length: %u\n", buffer_size);

    return status;
}

// 接收请求
static int recv_request(int socket_fd, char *buffer, int buffer_size)
{
    int status = 0;

    // 先接收四个字节的长度
    uint32_t data_len = 0;
    status = net_recv(socket_fd, &data_len, 4, MSG_WAITALL);
    data_len = ntohl(data_len);
    if (data_len > buffer_size)
    {
        printf("recv request length: %u > %d\n", data_len, buffer_size);
        return -1;
    }
    // 再接收数据
    status = net_recv(socket_fd, buffer, data_len, MSG_WAITALL);

    if (status > 0)
    {
        buffer[status] = '\0';
    }
    else
    {
        printf("Client disconnected.\n");
    }

    return status;
}

// 发送指定文件夹的所有文件名称
void handle_list_command(Command_t *cmd_p)
{
    DIR *dirp = NULL;
    struct dirent *dp = NULL;

    dirp = opendir(".");
    if (dirp == NULL)
    {
        perror("opendir");
        exit(1);
    }

    int len = 0;
    while ((dp = readdir(dirp)) != NULL)
    {
        // printf("%s\n", dp->d_name);
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            // 把数据传入buffer
            snprintf(cmd_p->buffer + len, sizeof(cmd_p->buffer) - 1 - len, "%s\n", dp->d_name);
            len += strlen(dp->d_name) + 1;
            if (len >= sizeof(cmd_p->buffer) - 1)
            {
                printf("%s\n", "Error: Buffer overflow occurred.");
                break;
            }
        }
    }

    closedir(dirp);

    send_response(cmd_p->socket_fd, "%s", cmd_p->buffer);
}

// 根据文件名称发送文件到客户端
void handle_get_command(Command_t *cmd_p)
{
    const char *file_name = cmd_p->arg;

    if (net_file_exist(file_name) > 0)
    {
        printf("File '%s' exists.\n", file_name);
        // 发送成功消息
        send_response(cmd_p->socket_fd, "%s", status_codes[cmd_p->type].success);
    }
    else
    {
        printf("File '%s' does not exist.\n", file_name);
        // 发送失败消息
        send_response(cmd_p->socket_fd, "%s", status_codes[cmd_p->type].fail);
        return;
    }

    // 发送文件
    net_send_file(file_name, cmd_p->socket_fd);
}

// 将上传的文件保存到指定的文件夹中
void handle_put_command(Command_t *cmd_p)
{
}

// 终止当前连接
void handle_quit_command(Command_t *cmd_p)
{
    send_response(cmd_p->socket_fd, "%s", status_codes[cmd_p->type].success);
}
