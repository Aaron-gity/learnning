#ifndef __PROMPT_H__
#define __PROMPT_H__

#include <ncurses.h>

// 设置命令提示
void set_prompt(const char *newprompt);
// 输出命令提示
void print_prompt();

// 恢复命令提示
void restore_prompt();

#endif // !__PROMPT_
