#include "command.h"

#include <stdio.h>
#include <string.h>

// 定义帮助命令信息数组
static const COMMANDINFOS command_infos = {
    {"help", "help", "Show available commands"},
    {"list", "list", "List files in current directory"},
    {"get", "get \"filename\"", "Download specified file"},
    {"put", "put \"filename\"", "Upload specified file"},
    {"quit", "quit", "Quit the program"}};

// 定义状态信息数组
const Status_t status_codes[CMD_INVALID] = {
    {"OK", "fail"},
    {"OK", "fail"},
    {"OK", "fail"},
    {"OK", "fail"},
    {"Quit success", "Quit fail"}};

// 输出帮助命令信息
void default_help_command(Command_t *cmd)
{
    printf("Available commands:\n");
    for (int i = 0; i < CMD_INVALID; ++i)
    {
        printf("\t%s\t%s\n", command_infos[i].help, command_infos[i].desc);
    }
}

// 解析命令
int parse_command(Command_t *cmd_p)
{
    char *command = cmd_p->buffer;
    const char *p = strtok(command, " \n");
    if (p == NULL)
    {
        return -1;
    }
    if (strncmp(p, command_infos[CMD_HELP].command, 4) == 0)
    {
        cmd_p->type = CMD_HELP;
        strncpy(cmd_p->command_type, command_infos[CMD_HELP].command, sizeof(cmd_p->command_type) - 1);
        return 0;
    }
    else if (strncmp(p, command_infos[CMD_LIST].command, 4) == 0)
    {
        cmd_p->type = CMD_LIST;
        strncpy(cmd_p->command_type, command_infos[CMD_LIST].command, sizeof(cmd_p->command_type) - 1);
        return 0;
    }
    else if (strncmp(p, command_infos[CMD_GET].command, 3) == 0)
    {
        cmd_p->type = CMD_GET;
        strncpy(cmd_p->command_type, command_infos[CMD_GET].command, sizeof(cmd_p->command_type) - 1);
        // 解析 get 命令的参数
        const char *arg = strtok(NULL, " \n");
        if (arg != NULL)
        {
            strncpy(cmd_p->arg, arg, sizeof(cmd_p->arg) - 1);
            sprintf(command, "%s %s", cmd_p->command_type, cmd_p->arg);
            return 0;
        }
    }
    else if (strncmp(p, command_infos[CMD_PUT].command, 3) == 0)
    {
        cmd_p->type = CMD_PUT;
        strncpy(cmd_p->command_type, command_infos[CMD_PUT].command, sizeof(cmd_p->command_type) - 1);
        // 解析 put 命令的参数
        const char *arg = strtok(NULL, " \n");
        if (arg != NULL)
        {
            strncpy(cmd_p->arg, arg, sizeof(cmd_p->arg) - 1);
            sprintf(command, "%s %s", cmd_p->command_type, cmd_p->arg);
            return 0;
        }
    }
    else if (strncmp(p, command_infos[CMD_QUIT].command, 4) == 0)
    {
        cmd_p->type = CMD_QUIT;
        strncpy(cmd_p->command_type, command_infos[CMD_QUIT].command, sizeof(cmd_p->command_type) - 1);
        return 0;
    }
    else
    {
        cmd_p->type = CMD_INVALID; // 表示无效命令
    }
    return -1;
}

// 处理命令
void handle_command(Command_t cmd, COMMAND_HANDLERS handlers)
{
    if (parse_command(&cmd) < 0)
    {
        printf("Invalid command\n");
        return;
    }
    handlers[cmd.type](&cmd);
}
