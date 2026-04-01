#include "csapp.h"
#include "FTP.h"

#define MAX_NAME_LEN 256
#define NB_SLAVES 2


int main() {
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_hostname[256];  
    char client_ip[INET_ADDRSTRLEN];
    //rio_t rio;

    // Infos sur les slaves
    slaveInfo_t slaves[NB_SLAVES];

    // Master écoute pour les notifications des slaves
    listenfd = Open_listenfd(PORT_MASTER);
    printf("[MASTER] listening on port %d\n", PORT_MASTER);

    int nb_slaves_connectes = 0;
    while (nb_slaves_connectes < NB_SLAVES) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        // lire les infos envoyees par le slave
        rio_t rio_client;
        Rio_readinitb(&rio_client, connfd);
        char slave_ip[INET_ADDRSTRLEN];
        int slave_port;
        Rio_readnb(&rio_client, slave_ip, INET_ADDRSTRLEN);
        Rio_readnb(&rio_client, &slave_port, sizeof(int));

        // stocker les infos du slave
        strcpy(slaves[nb_slaves_connectes].ip, slave_ip);
        slaves[nb_slaves_connectes].port = slave_port;
        nb_slaves_connectes++;

        printf("[MASTER] Slave connected: %s:%d\n", slave_ip, slave_port);
        Close(connfd);
    }

    printf("[MASTER] Tous les slaves sont connectés. Prêt à accepter les clients.\n");

    int nb_requetes = 0;
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, 256, 0, 0, 0);
        Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip, INET_ADDRSTRLEN);// convertir l'adresse IP en chaîne de caractères
        printf("[MASTER] Client connected: %s (%s)\n", client_hostname, client_ip);


        int slave_index = nb_requetes % NB_SLAVES;
        nb_requetes++;

        // envoyer IP et port du slave au client
        Rio_writen(connfd, slaves[slave_index].ip, INET_ADDRSTRLEN);
        Rio_writen(connfd, &slaves[slave_index].port, sizeof(int));
        Close(connfd);
    }

    return 0;
}