
#include<assert.h>
#include<stdio.h>
#include<unistd.h>
#include<bluetooth/bluetooth.h>
#include<bluetooth/hci.h>
#include<bluetooth/rfcomm.h>
#include<sys/socket.h>
#include<signal.h>
#include<time.h>
#include<stdlib.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MTW 127
int rev_size = 0;
int rcv_B = 0, rcv_KB = 0, rcv_MB = 0;

int begin_time, end_time, stop_srv = 0;

static void error_deal(const char *where)
{
    char *print_message = (char *)malloc(sizeof("():") + sizeof(*where) + 1);
    strcpy(print_message, where);
    strcat(print_message,"():");
    perror(print_message);
    free(print_message);
    return;
}

void sig_int(int signo)
{
    float speed = 0;
    
    printf("The last time is : %d s\n",end_time - begin_time);
    stop_srv = 1;
    return;
}

int main(int argc, char **argv)
{
    struct sigaction sigact_int, old_sigact_int;
    struct sockaddr_rc loc_addr = {0}, cli_addr;
    char buf[MTW];
    int ser_sock, cli_sock;
    
    //打开文件，获取文件句柄
    char *file_name = "1200M.txt";
    int filefd = open( file_name, O_RDONLY );
    assert( filefd > 0 );
    struct stat stat_buf;
    fstat( filefd, &stat_buf );

    sigact_int.sa_handler = sig_int;
    sigemptyset(&sigact_int.sa_mask);
    sigact_int.sa_flags = 0;
    sigaction(SIGINT, &sigact_int,&old_sigact_int);

    if(ser_sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM))
        error_deal("socket");

    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = *BDADDR_ANY;
    loc_addr.rc_channel = (uint8_t) 1;
    if(bind(ser_sock,(struct sockaddr *)&loc_addr, sizeof(loc_addr)) < 0)
        perror("bind():");

    if(listen(ser_sock,1) < 0)
        perror("listen()");
    int addr_len = sizeof(struct sockaddr);
    cli_sock = accept(ser_sock, (struct sockaddr *) &cli_addr,&addr_len);
    if(cli_sock < 0)
        perror("accept():");
    else {
        ba2str(&cli_addr.rc_bdaddr,buf);
        printf("accepted connection from %s\n",buf);
    }
    memset(buf,0, MTW);
    int read_bytes = 0;

    begin_time = time(NULL);
    int i = 0;
    int send_bytes = 0;

    while(!stop_srv)
    {
        send_bytes = sendfile( cli_sock, filefd, NULL, stat_buf.st_size );
        if(send_bytes >= 0)
            printf("send success!\n");
    }
    
    end_time = time(NULL);
    printf("The last time is : %d s\n",end_time - begin_time);
    rcv_MB = send_bytes / (1024 * 1024);
    rcv_KB = (send_bytes - rcv_MB * 1024 * 1024) / 1024;
    rcv_B = send_bytes - rcv_MB * 1024 * 1024 - rcv_KB * 1024;
    printf("have recv %d MB %d KB %d B\n",rcv_MB, rcv_KB, rcv_B);
    close(cli_sock);
    close(ser_sock);

}



