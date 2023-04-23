// #include "./prompt/prompt.h"
#include "./ftp_client/ftp_client.h"

int main()
{
    // // 改变命令提示符
    // const char *newprompt = "ftp> ";
    // set_prompt(newprompt);

    // 启动客户端
    start_ftp_client();

    // // 暂停程序，直到用户按下任意键
    // getch();

    // // 恢复命令提示符
    // restore_prompt();

    return 0;
}