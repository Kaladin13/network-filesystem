#ifndef HTTP_CLIENT
#define HTTP_CLIENT

#include <linux/fs.h>

#define MAX_NUM 32

void print_hell_msg(void);
void send_holla(void);
int tcp_client_connect(void);
void tcp_client_disconnect(void);

void list(char *);
int lookup(const char *);
int create(char *);

#endif