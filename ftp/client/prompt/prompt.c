#include "prompt.h"

#include <stdlib.h>

static const char *g_newprompt;

// 设置命令提示
void set_prompt(const char *newprompt)
{
    // 保存命令提示
    g_newprompt = newprompt;

    // 初始化NCurses库
    initscr();

    // 将光标设置为可见状态
    curs_set(1);

    // 更改提示符字符串
    setenv("PS1", newprompt, 1);

    // 清空屏幕并输出提示符
    clear();

    printw("%s", newprompt);

    // 刷新屏幕
    refresh();
}

// 输出命令提示
void print_prompt()
{
    printw("%s", g_newprompt);
}

// 恢复命令提示
void restore_prompt()
{
    // 恢复原始terminal设置并退出
    endwin();
}