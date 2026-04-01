/*
 * echoclient.c - An echo client
 */

#include "csapp.h"
#include "client.h"
#include <time.h>
#include <sys/time.h>


int main(int argc, char **argv)
{
    int clientfd, port;
    char *host, buf[MAXLINE];
    rio_t rio;
    request_t req;
    response_t res;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host> \n", argv[0]);
        exit(0);}

    host = argv[1];
    port = 2125; // fixer le port
    
    // Se connecter au serveur Master
    clientfd = Open_clientfd(host, port);
    printf("Connected to master %s:%d\n", host, port);

    Rio_readinitb(&rio, clientfd);

    // Lire le port du slave depuis le maître
    //int port_slave;
    //Rio_readnb(&rio, &port_slave, sizeof(int));
    //printf("Redirected to slave port %d\n", port_slave);
    char slave_ip[INET_ADDRSTRLEN];
    int slave_port;

    Rio_readnb(&rio, slave_ip, INET_ADDRSTRLEN);
    Rio_readnb(&rio, &slave_port, sizeof(int));
    printf("Redirected to slave %s:%d\n", slave_ip, slave_port);

    Close(clientfd);

    /* connexion au slave */
    //clientfd = Open_clientfd(slave_ip, slave_port);
    //Rio_readinitb(&rio, clientfd);
    //Close(clientfd);

    // Se connecter au slave
    int clientfd_slave = Open_clientfd(slave_ip, slave_port);
    printf("Connected to slave %s:%d\n", slave_ip, slave_port);
    printf("ftp> ");

    
    Rio_readinitb(&rio, clientfd_slave);

    while (Fgets(buf, MAXLINE, stdin) != NULL) {

        char command[MAXLINE];
        char filename[MAXLINE];
        if (sscanf(buf, "%s", command) != 1) {
            printf("Erreur de lecture commande.\n");
            printf("ftp> ");
            continue;
        }
        if (strcmp(command, "get") == 0) {
            req.type = GET;
            if (sscanf(buf, "%*s %255s", filename) != 1) {
                printf("Nom de fichier manquant.\n");
                printf("ftp> ");
                continue;
            }
            strcpy(req.filename, filename);
            req.offset = 0; //initialise a zero
            get(clientfd_slave, &rio, &req, &res);

        } else if (strcmp(command, "put") == 0) {
            req.type = PUT;
            if (sscanf(buf, "%*s %255s", filename) != 1) {
                printf("Nom de fichier manquant.\n");
                printf("ftp> ");
                continue;
            }
            strcpy(req.filename, filename);
            // A IMPLEMENTER
            put(clientfd_slave, &rio, &req, &res);
         
        } else if (strcmp(command, "ls") == 0) {
            req.type = LS;
            // A IMPLEMENTER
            ls(clientfd_slave,&rio,&req,&res);
        
        } else if (strcmp(command, "bye") == 0) {
            /* Q9 : fermeture propre de la connexion */
            do_bye(clientfd_slave);
            exit(0);
        }else if(strcmp(command, "rm") == 0){
            req.type= RM;
            if (sscanf(buf, "%*s %255s", filename) != 1) {
                printf("Nom de fichier manquant.\n");
                printf("ftp> ");
                continue;
            }
            strcpy(req.filename, filename);
            Rm(clientfd_slave, &rio, &req, &res);
        } else {
            printf("Commande non reconnue. Utilisez get, rm, put, bye ou ls.\n");
            printf("ftp> ");
            continue;}
        printf("ftp> ");
}
    Close(clientfd_slave);
    exit(0);

}
