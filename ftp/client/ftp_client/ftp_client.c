#include "ftp_client.h"
#include "../../net/net.h"
#include "../../command/command.h"
// #include "../prompt/prompt.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

static const char *ip = "127.0.0.1";
static const int port = 20021;

// 实现客户端的命令处理
void handle_list_command(Command_t *cmd_p);   // 显示服务器当前文件夹的内容
void handle_get_command(Command_t *cmd_p);    // 从服务器获取指定文件
void handle_put_command(Command_t *cmd_p);    // 上传指定文件到服务器
void handle_quit_command(Command_t *cmd_p);   // 退出程序

COMMAND_HANDLERS handlers =
    {
        default_help_command,
        handle_list_command,
        handle_get_command,
        handle_put_command,
        handle_quit_command};

// 发送消息
static int send_message(int socket_fd, char *message);

// 接收消息
static int receive_message(int socket_fd, char *message);

// 循环处理
static int loopProcessor(int socket_fd);

// 启动客户端
int start_ftp_client()
{
    // 连接服务器
    int socket_fd = net_init();
    if (socket_fd < 0)
    {
        perror("net_init failed.");
        return -1;
    }
    if (net_connect(socket_fd, ip, port) == -1)
    {
        perror("Failed to connect to server.");
        return -1;
    }

    // 循环处理
    loopProcessor(socket_fd);

    return 0;
}

// 发送消息
static int send_message(int socket_fd, char *message)
{
    uint32_t message_len = strlen(message);
    uint32_t send_head = htonl(message_len);
    int len_send = net_send(socket_fd, &send_head, sizeof(send_head), MSG_WAITALL);
    if (len_send == -1)
    {
        perror("Error sending message length.\n");
        return -1;
    }

    int message_send = net_send(socket_fd, message, message_len, MSG_WAITALL);
    if (message_send == -1)
    {
        perror("Error sending message.\n");
        return -1;
    }

    return 0;
}

// 接收消息
static int receive_message(int socket_fd, char *message)
{
    uint32_t message_len = 0;

    int len_recv = net_recv(socket_fd, &message_len, sizeof(message_len), MSG_WAITALL);
    message_len = ntohl(message_len);
    if (len_recv == -1)
    {
        perror("Error receiving message length.\n");
        return -1;
    }

    int message_recv = net_recv(socket_fd, message, message_len, MSG_WAITALL);
    if (message_recv == -1)
    {
        perror("Error receiving message.\n");
        return -1;
    }
    message[message_recv] = '\0';

    printf("Message length: %u\n", message_len);
    // printf("Message: %s\n", message);

    return message_len;
}

// 循环处理
int loopProcessor(int socket_fd)
{
    Command_t cmd = {};
    cmd.socket_fd = socket_fd;
    while (1)
    {
        // 读取输入的命令
        printf("ftp> ");
        fgets(cmd.buffer, BUFFER_SIZE, stdin);
        // printf("%s\n", op);
        // 处理命令
        handle_command(cmd, handlers);

        // print_prompt();
        // // 刷新屏幕
        // refresh();
    }
    return 0;
}

// 显示服务器当前文件夹的内容
void handle_list_command(Command_t *cmd_p)
{
    // 发送命令
    send_message(cmd_p->socket_fd, cmd_p->buffer);
    receive_message(cmd_p->socket_fd, cmd_p->buffer);
    printf("%s\n", cmd_p->buffer);
}

// 从服务器获取指定文件
void handle_get_command(Command_t *cmd_p)
{
    // 发送命令
    printf("handle %s\n", cmd_p->buffer);
    send_message(cmd_p->socket_fd, cmd_p->buffer);

    // 判断是否成功
    receive_message(cmd_p->socket_fd, cmd_p->buffer);
    char *file_name = cmd_p->arg;
    if (strncmp(cmd_p->buffer, status_codes[cmd_p->type].success, strlen(status_codes[cmd_p->type].success)) == 0)
    {
        // 接收文件
        char *path = "./download";
        char path_name[COMMAND_ARG_MAX_SIZE + 15] = {};
        sprintf(path_name, "%s/%s", path, file_name);
        net_receive_file(path_name, cmd_p->socket_fd);
    }
    else
    {
        printf("Failed to get file: %s\n", file_name);
    }
}

// 上传指定文件到服务器
void handle_put_command(Command_t *cmd_p)
{
    printf("Command not handle\n");
}

// 退出程序
void handle_quit_command(Command_t *cmd_p)
{
    // 发送命令
    send_message(cmd_p->socket_fd, cmd_p->buffer);
    receive_message(cmd_p->socket_fd, cmd_p->buffer);
    printf("%s\n", cmd_p->buffer);
    close(cmd_p->socket_fd);
    exit(EXIT_SUCCESS);
}
