#include "ftp_server.h"
#include "../../event/event.h"
#include "../../net/net.h"
#include "../ftp_session/ftp_session.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

static const char *ip = "127.0.0.1";
static const int port = 20021;
static const int max_connections = 1000;
static int pipe_fd[2]; // 信号处理的管道

// 处理连接到客户端的事件
static void handle_client_connect(void *arg);

// 处理信号发生的事件
static void handle_signal(void *arg);

// 信号处理函数
static void sig_handler(int signum);

// 清理注册的信号
static void cleanup();

// 设置信号函数
static void add_sig(int sig, void(handler)(int), bool restart);

// 启动服务器
void start_ftp_server()
{
    // 开启网络连接
    int listen_fd = net_init();
    if (listen_fd < 0)
    {
        fprintf(stderr, "net_init failed\n");
        return;
    }
    net_set_nonblocking(listen_fd);
    net_set_reuseaddr(listen_fd);
    if (net_bind(listen_fd, ip, port) < 0)
    {
        fprintf(stderr, "net_bind failed\n");
        close(listen_fd);
        return;
    }
    if (net_listen(listen_fd, max_connections) < 0)
    {
        fprintf(stderr, "net_listen failed\n");
        close(listen_fd);
        return;
    }

    printf("Starting...\n");

    // 创建事件处理上下文
    Event_base_t *base = event_base_new();

    // 创建监听事件
    Event_t *listen_ev = event_new(base->epoll_fd, listen_fd, EPOLLIN, handle_client_connect, NULL);
    event_set_self_callback_arg(listen_ev);
    event_add(listen_ev);

    // 注册信号处理程序
    add_sig(SIGPIPE, SIG_IGN, true);
    // 创建通信管道
    if (socketpair(PF_UNIX, SOCK_STREAM, 0, pipe_fd) < 0)
    {
        perror("socketpair");
        exit(EXIT_FAILURE);
    }
    net_set_nonblocking(pipe_fd[1]);
    // if (signal(SIGINT, sig_handler) == SIG_ERR)
    // {
    //     perror("signal");
    //     exit(EXIT_FAILURE);
    // }
    add_sig(SIGINT, sig_handler, false);

    // 注册清理函数，确保所有资源都被释放
    atexit(cleanup);

    // 创建信号处理事件
    Event_t *sig_ev = event_new(base->epoll_fd, pipe_fd[0], EPOLLIN, handle_signal, base);
    event_add(sig_ev);

    // 进行事件分发
    event_base_dispatch(base);

    // 关闭通信管道
    close(pipe_fd[1]);
    close(pipe_fd[0]);

    // 关闭事件处理
    event_free(listen_ev);
    event_free(sig_ev);
    event_base_free(base);
}

// 处理连接到客户端的事件
static void handle_client_connect(void *arg)
{
    Event_t *eva = (Event_t *)arg;
    struct sockaddr_in addr;
    int client_fd = net_accept(eva->fd, &addr);
    Event_t *ev = event_new(eva->epoll_fd, client_fd, EPOLLIN, ftp_process, NULL);
    event_set_self_callback_arg(ev);
    event_add(ev);
    printf("Client connected\n");
}

// 处理连接到客户端的事件
static void handle_signal(void *arg)
{
    Event_base_t *base = (Event_base_t *)arg;
    char signals[NET_BUFFER_SIZE];
    int ret = recv(pipe_fd[0], signals, sizeof(signals), 0);
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
            case SIGINT:
            {
                // 停止事件处理循环
                base->is_run = false;
                printf("Quit\n");
            }
            }
        }
    }
}

// 信号处理函数
static void sig_handler(int signum)
{
    int msg = signum;
    printf("signum: %d\n", msg);
    int ret = send(pipe_fd[1], &msg, 1, 0);
    if (ret == -1)
    {
        perror("send");
    }
}

// 清理注册的信号
static void cleanup()
{
    // 注销所有已注册的信号处理程序
    signal(SIGTERM, SIG_DFL);
}

// 设置信号函数
static void add_sig(int sig, void(handler)(int), bool restart)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
    {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}
