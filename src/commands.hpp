#ifndef COMMANDS_H
#define COMMANDS_H

#ifdef __cplusplus
extern "C" {
#endif

void parse_command();

#ifdef __cplusplus
}
#endif

void cmd_help();
void cmd_clear();
void cmd_reboot();
void cmd_echo();

#endif

