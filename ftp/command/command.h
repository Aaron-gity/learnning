#ifndef ___COMMAND_H__
#define ___COMMAND_H__

#define COMMAND_MAX_SIZE 320     // 最大命令长度
#define COMMAND_TYPE_MAX_SIZE 20 // 最大命令类型长度
#define COMMAND_ARG_MAX_SIZE 300 // 最大命令参数长度
#define BUFFER_SIZE 1024            // 缓冲区大小

// 命令类型结构体
typedef enum
{
    CMD_HELP,
    CMD_LIST,
    CMD_GET,
    CMD_PUT,
    CMD_QUIT,
    CMD_INVALID
} CommandType;

// 命令结构体
typedef struct
{
    CommandType type;
    char command_type[COMMAND_TYPE_MAX_SIZE + 1];
    char arg[COMMAND_ARG_MAX_SIZE + 1];
    char buffer[BUFFER_SIZE + 1];
    int socket_fd;
} Command_t;

// 实现自己处理命令的回调函数
typedef void (*CommandHandler)(Command_t *);

// 实现自己处理命令的回调函数数组
typedef CommandHandler COMMAND_HANDLERS[CMD_INVALID];

// 处理命令
// 通过传递给COMMAND_HANDLERS参数来实现自己的命令处理
void handle_command(Command_t cmd, COMMAND_HANDLERS handlers);

// 定义帮助命令信息的结构体
typedef struct
{
    const char *command; // 命令名称
    const char *help;    // 帮助信息
    const char *desc;    // 命令描述
} CommandInfo;

typedef CommandInfo COMMANDINFOS[CMD_INVALID];

// 输出帮助命令信息
void default_help_command(Command_t *cmd);

// 定义列出状态信息的结构体
typedef struct
{
    char success[100];
    char fail[100];
} Status_t;

// 声明状态信息数组
extern const Status_t status_codes[CMD_INVALID];

#endif // !___COMMAND_H__
