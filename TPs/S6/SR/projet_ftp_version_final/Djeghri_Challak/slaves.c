#include "csapp.h"
#include "FTP.h"

#define SERVER_DIR "serveur_files/"
#define NPROC 3  

void echo(int connfd); 

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <MASTER_IP> <SLAVE_PORT>\n", argv[0]);
        exit(1);
    }

    char *master_ip = argv[1];
    int slave_port = atoi(argv[2]);

    // notifier le master
    int fd_master = Open_clientfd(master_ip, PORT_MASTER);
    
    
    //Rio_writen(fd_master, master_ip, INET_ADDRSTRLEN);
    char slave_ip[INET_ADDRSTRLEN];

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in tmp_addr;
    tmp_addr.sin_family = AF_INET;
    tmp_addr.sin_addr.s_addr = inet_addr(master_ip); // utiliser le master 
    tmp_addr.sin_port = htons(PORT_MASTER);

    connect(sock, (struct sockaddr*)&tmp_addr, sizeof(tmp_addr));

    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    getsockname(sock, (struct sockaddr*)&name, &namelen);

    inet_ntop(AF_INET, &name.sin_addr, slave_ip, INET_ADDRSTRLEN);
    close(sock);

    // envoyer au master
    Rio_writen(fd_master, slave_ip, INET_ADDRSTRLEN);
    Rio_writen(fd_master, &slave_port, sizeof(int));
    Close(fd_master);

    // attends des clients
    int listenfd = Open_listenfd(slave_port);
    printf("[SLAVE] listening on port %d\n", slave_port);

    while (1) {
        struct sockaddr_in clientaddr;
        socklen_t clientlen = sizeof(clientaddr);
        int connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        int pid = Fork();
        if (pid == 0) { // fils pour ce client
            Close(listenfd);
            echo(connfd);
            Close(connfd);
            exit(0);
        }
        Close(connfd);
    }

    return 0;
}