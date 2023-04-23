#ifndef __EVENT_H__
#define __EVENT_H__

#include <sys/epoll.h>
#include <stdbool.h>

#define MAX_EVENTS 256

// 事件回调函数指针
typedef void(*event_callback_fn)(void *);

// 事件上下文结构体
typedef struct Event_base
{
    int epoll_fd;
    struct epoll_event evs[MAX_EVENTS];
    bool is_run;
} Event_base_t;

// 创建事件上下文
Event_base_t* event_base_new();

// 销毁事件上下文
void event_base_free(Event_base_t *event_base);

// 事件结构体
typedef struct Event
{
    int epoll_fd;
    int fd;
    struct epoll_event ev;
    event_callback_fn handle_callback;
    void *arg;
} Event_t;

// 创建事件
Event_t *event_new(int epoll_fd, const int fd, const int flags, event_callback_fn event_callback, void *arg);

// 设置事件自身为回调参数
void event_set_self_callback_arg(Event_t *event);

// 销毁事件
void event_free(Event_t *event);

// 添加事件
void event_add(Event_t *event);

// 分发处理事件
int event_base_dispatch(Event_base_t *event_base);

#endif // !__EVENT_H__
