#include "event.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

// 创建事件上下文
Event_base_t* event_base_new()
{
    Event_base_t* event_base = (Event_base_t*)calloc(sizeof(Event_base_t), 1);
    event_base->epoll_fd = epoll_create(1);
    if (event_base->epoll_fd == -1)
    {
        perror("epoll_create");
        free(event_base);
        return NULL;
    }
    event_base->is_run = true;
    return event_base;
}

// 销毁事件上下文
void event_base_free(Event_base_t* event_base)
{
    free(event_base);
}

// 创建事件
Event_t* event_new(int epoll_fd, const int fd, const int flags, event_callback_fn event_callback, void* arg)
{
    Event_t* event = (Event_t*)calloc(sizeof(Event_t), 1);
    event->epoll_fd = epoll_fd;
    event->fd = fd;
    event->ev.events |= flags;
    event->ev.data.ptr = event;
    event->handle_callback = event_callback;
    event->arg = arg;
    return event;
}

// 设置事件自身为回调参数
void event_set_self_callback_arg(Event_t *event)
{
    event->arg = event;
}

// 销毁事件
void event_free(Event_t* event)
{
    free(event);
}

// 添加事件
void event_add(Event_t* event)
{
    epoll_ctl(event->epoll_fd, EPOLL_CTL_ADD, event->fd, &event->ev);
}

// 分发处理事件
int event_base_dispatch(Event_base_t* event_base)
{
    int epoll_fd = event_base->epoll_fd;
    struct epoll_event *evs = event_base->evs;

    // 定义一个循环监视事件集合
    while (event_base->is_run)
    {
        int n = epoll_wait(epoll_fd, evs, MAX_EVENTS, -1);
        if (n < 0 && errno != EINTR) // 这个错误已经处理了
        {
            perror("epoll_wait");
            break;
        }
        for (int i = 0; i < n; i++)
        {
            // 有数据读
            if (evs[i].events & EPOLLIN)
            {
                // 收到客户端发送的数据
                Event_t* e = (Event_t *)evs[i].data.ptr;
                e->handle_callback(e->arg);
            }
            // 有数据写
            else if (evs[i].events & (EPOLLOUT | EPOLLHUP | EPOLLERR))
            {
                // 发送请求数据到客户端
                Event_t* e = (Event_t *)evs[i].data.ptr;
                e->handle_callback(e->arg);
            }
        }
    }
    return 0;
}