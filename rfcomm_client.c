#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>

#define MTU 127

int send_B = 0, send_KB = 0, send_MB = 0;
long long begin_time, end_time;
int stop_srv = 0;
static void sig_int(int signo)
{
    end_time = time(NULL);
    printf("Has read %d MB %d KB %d B\n",send_MB,send_KB,send_B);
    printf("Has last %lld s",end_time - begin_time);
    stop_srv = 1;
    return;
}

static void print_returnval(int ret_val, char *function_name)
{
    printf("the %s()'s return value is %d\n",function_name,ret_val);
}
int main(int argc, char **argv)
{
    if(argc != 2)
    {
        printf("Usage:%s <SEND FREQUENCY(us)>\n",argv[0]);
        return 0;
    }
    int send_frequency = atoi(argv[1]);
    struct sockaddr_rc addr = { 0 };
    struct sigaction sigact_int, old_sigact_int;

    sigact_int.sa_handler = sig_int;
    sigemptyset(&sigact_int.sa_mask);
    sigact_int.sa_flags = 0;
    sigaction(SIGINT,&sigact_int, &old_sigact_int);

    int s, status;
    char dest[19] = "D4:CA:6E:51:C1:64";
    char buf[MTU + 1] = {0};

    // allocate a socket
    s = socket(AF_BLUETOOTH , SOCK_STREAM, BTPROTO_RFCOMM);

    // set the connection parameters (who to connect to)
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t) 1;
    str2ba( dest, &addr.rc_bdaddr );

    // connect to server
    status = connect(s, (struct sockaddr *)&addr, sizeof(addr));
    print_returnval(status,"connect");
    int write_bytes = 0;
    memset(buf,1,MTU - 1);
    begin_time = time(NULL);
    if( status == 0 ) {
        while(!stop_srv)
        {
            write_bytes = recv(s,buf,MTU,MSG_WAITALL);
            send_B += write_bytes;
            if(send_B >= 1024)
            {
                send_B -= 1024;
                send_KB += 1;
            }
            if(send_KB >= 1024)
            {
                send_KB -= 1024;
                send_MB += 1;
            }
            printf("Has read %d MB %d KB %d B\n",send_MB,send_KB,send_B);
        }
    }
    //if( status < 0 ) perror("uh oh");
    print_returnval(status,"write");
    printf("The client procedure has quit!\n");
    close(s);
    return 0;
}