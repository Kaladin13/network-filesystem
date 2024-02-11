#include "http_client.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/net.h>
#include <net/sock.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include <asm/uaccess.h>
#include <linux/socket.h>
#include <linux/slab.h>

#define PORT 8239

struct socket *conn_socket = NULL;

u32 create_address(u8 *ip)
{
    u32 addr = 0;
    int i;

    for (i = 0; i < 4; i++)
    {
        addr += ip[i];
        if (i == 3)
            break;
        addr <<= 8;
    }
    return addr;
}

int tcp_client_send(struct socket *sock, const char *buf, const size_t length,
                    unsigned long flags)
{
    struct msghdr msg;
    // struct iovec iov;
    struct kvec vec;
    int len, written = 0, left = length;

    msg.msg_name = 0;
    msg.msg_namelen = 0;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = flags;

repeat_send:
    vec.iov_len = left;
    vec.iov_base = (char *)buf + written;

    len = kernel_sendmsg(sock, &msg, &vec, left, left);
    if ((len == -ERESTARTSYS) || (!(flags & MSG_DONTWAIT) &&
                                  (len == -EAGAIN)))
        goto repeat_send;
    if (len > 0)
    {
        written += len;
        left -= len;
        if (left)
            goto repeat_send;
    }

    return written ? written : len;
}

int tcp_client_receive(struct socket *sock, char *str,
                       unsigned long flags)
{
    struct msghdr msg;

    struct kvec vec;
    int len;
    int max_size = 50;

    msg.msg_name = 0;
    msg.msg_namelen = 0;

    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = flags;

    vec.iov_len = max_size;
    vec.iov_base = str;

read_again:
    len = kernel_recvmsg(sock, &msg, &vec, max_size, max_size, flags);

    if (len == -EAGAIN || len == -ERESTARTSYS)
    {
        pr_info(" *** mtp | error while reading: %d | "
                "tcp_client_receive *** \n",
                len);

        goto read_again;
    }

    pr_info(" *** mtp | the server says: %s | tcp_client_receive *** \n", str);
    return len;
}

int tcp_client_connect(void)
{
    struct sockaddr_in saddr;
    unsigned char destip[5] = {192, 168, 1, 68, '\0'};
    int ret;

    ret = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &conn_socket);
    if (ret < 0)
    {
        pr_info(" *** mtp | Error: %d while creating first socket. | "
                "setup_connection *** \n",
                ret);
        goto err;
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(PORT);
    saddr.sin_addr.s_addr = htonl(create_address(destip));

    ret = conn_socket->ops->connect(conn_socket, (struct sockaddr *)&saddr, sizeof(saddr), O_RDWR);

    if (ret && (ret != -EINPROGRESS))
    {
        pr_info(" *** mtp | Error: %d while connecting using conn "
                "socket. | setup_connection *** \n",
                ret);
        goto err;
    }
err:
    return -1;
}

void tcp_client_disconnect(void)
{
    if (conn_socket != NULL)
    {
        sock_release(conn_socket);
        pr_info("releasing\n");
    }
}

void make_request(char *request, char *response)
{
    tcp_client_send(conn_socket, request, strlen(request), MSG_DONTWAIT);

    DECLARE_WAIT_QUEUE_HEAD(recv_wait);

    wait_event_timeout(recv_wait,
                       !skb_queue_empty(&conn_socket->sk->sk_receive_queue),
                       5 * HZ);

    if (!skb_queue_empty(&conn_socket->sk->sk_receive_queue))
    {
        tcp_client_receive(conn_socket, response, MSG_DONTWAIT);
    }
}

int create(char *name)
{
    char name_f = name[0];
    int inode_num = 1099 - name_f;

    return inode_num;
}

int lookup(const char *name)
{
    char name_f = name[0];
    int inode_num = 1099 - name_f;

    return inode_num;
}

void list(char *response)
{
    char request[50];
    memset(&request, 0, 40);

    strcat(request, "list_call\n");

    make_request(request, response);

    pr_info("gottedsss: %s\n", response);
    pr_info("ptrrs: %p\n", response);
}

void send_holla()
{
    char request[50];
    char response[4096];

    strcat(request, "xv6");

    make_request(request, response);
    make_request(response, response);

    // int rrr = tcp_client_connect();
    // pr_info("code %d\n", rrr);

    // int len = 49;
    // char response[len + 1];
    // char reply[len + 1];
    // int ret = -1;

    // memset(&reply, 0, len + 1);
    // strcat(reply, "HOLA");

    // tcp_client_send(conn_socket, reply, strlen(reply), MSG_DONTWAIT);

    // DECLARE_WAIT_QUEUE_HEAD(recv_wait);

    // wait_event_timeout(recv_wait,
    //                    !skb_queue_empty(&conn_socket->sk->sk_receive_queue),
    //                    5 * HZ);
    // /*
    // add_wait_queue(&conn_socket->sk->sk_wq->wait, &recv_wait);
    // while(1)
    // {
    //         __set_current_status(TASK_INTERRUPTIBLE);
    //         schedule_timeout(HZ);
    // */
    // if (!skb_queue_empty(&conn_socket->sk->sk_receive_queue))
    // {
    //     /*
    //     __set_current_status(TASK_RUNNING);
    //     remove_wait_queue(&conn_socket->sk->sk_wq->wait,\
    //                                           &recv_wait);
    //     */
    //     memset(&response, 0, len + 1);
    //     tcp_client_receive(conn_socket, response, MSG_DONTWAIT);
    //     // break;
    // }

    // pr_info("%s\n", response);

    // if (conn_socket != NULL)
    // {
    //     sock_release(conn_socket);
    //     pr_info("releasing\n");
    // }
}

void print_hell_msg()
{
    printk(KERN_INFO "Hello world driver loaded.\n");
}