/*
 sendFile - read a filename from the client, then send the file's contents
 *           back to the client
 */
#include "csapp.h"
#include "FTP.h"
#define SERVER_DIR "serveur_files/"



void echo(int connfd){
        char filename[MAXLINE];
        rio_t rio;
        request_t req;
        response_t res; 

        Rio_readinitb(&rio, connfd);
        while (1) {
        //lire la requete du client
        if (Rio_readnb(&rio, &req, sizeof(req)) <= 0)
            return;

        strcpy(filename, req.filename); // supposer que le nom du fichier suit la commande
        filename[strcspn(filename, "\n")] = '\0';// enlever le \n

        if(req.type == GET){

            char path[MAXLINE];
            snprintf(path, sizeof(path), SERVER_DIR "%.255s", filename);  
            
            //utlisitaion de stat pour lire une seul fois le fichier
            struct stat st;// struct contient des iinfos sur le fichier
            if (stat(path, &st) < 0) {
                res.success = 0;
                strcpy(res.message, "Fichier introuvable.");
                Rio_writen(connfd, &res, sizeof(res));
                return;
            }
            size_t size = st.st_size;
            //Rio_writen(connfd, &size, sizeof(size));// envoi le

            
            FILE *fich = fopen(path, "rb");
            if (fich == NULL) {
                res.success = 0;
                strcpy(res.message, "Erreur dans l'ouverture du fichier.");
                Rio_writen(connfd, &res, sizeof(res));// envoyer la reponse d'erreur au client
                return;
            }
            fseek(fich,req.offset,SEEK_SET); // se positionner a l'offset pour la reprise de transfert
            char buffer[BLOCK_SIZE];

            //  envoyer reponse AVANT fichier
            res.success = 1;
            strcpy(res.message, "OK");
            Rio_writen(connfd, &res, sizeof(res));
            Rio_writen(connfd, &size, sizeof(size));// envoi le size

            size_t n;
            while ((n = fread(buffer, 1, BLOCK_SIZE, fich)) > 0) {
                Rio_writen(connfd, buffer, n);
            }   
            fclose(fich);         
            //free(buffer);
        }
        else if(req.type == PUT){
            char path[MAXLINE];
            snprintf(path, sizeof(path), SERVER_DIR "%.255s", filename);

            size_t size;
            if (Rio_readnb(&rio, &size, sizeof(size)) <= 0) {
                res.success = 0;
                strcpy(res.message, "Erreur lecture taille fichier.");
                Rio_writen(connfd, &res, sizeof(res));
                return;
            }

            FILE *fd = fopen(path, "wb");
            if (fd == NULL) {
                res.success = 0;
                strcpy(res.message, "Impossible d'ouvrir le fichier en ecriture.");
                Rio_writen(connfd, &res, sizeof(res));
                return;
            }

            size_t received = 0;
            char buffer[BLOCK_SIZE];
            while (received < size) {
                size_t to_read = size - received;
                if (to_read > BLOCK_SIZE)
                    to_read = BLOCK_SIZE;

                size_t n = Rio_readnb(&rio, buffer, to_read);
                if (n <= 0) break;

                fwrite(buffer, 1, n, fd);
                received += n;
            }
            fclose(fd);

            if (received != size) {
                res.success = 0;
                strcpy(res.message, "Reception fichier incomplet.");
                Rio_writen(connfd, &res, sizeof(res));
                return;
            }

            res.success = 1;
            strcpy(res.message, "OK");
            Rio_writen(connfd, &res, sizeof(res));

        } else if(req.type == LS){
            // à implémenter 
            //FILE *f = popen("ls serveur_files/" , "r");
            if(Fork() == 0 ){
                int fd = open("sortie_ls.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
                dup2(fd, STDOUT_FILENO);
                execl("/bin/ls", "ls", "serveur_files/", NULL);
                exit(1);
            }
            wait(NULL); 
            FILE *f = fopen("sortie_ls.txt","r");
            if (f == NULL) {
                res.success = 0;
                strcpy(res.message, "Erreur ouverture fichier");
                Rio_writen(connfd, &res, sizeof(res));
                return;
            }
            res.success= 1;
            strcpy(res.message, "OK");
            Rio_writen(connfd, &res, sizeof(res));

            struct stat st;// struct contient des iinfos sur le fichier
            if (stat("sortie_ls.txt", &st) < 0) {
                res.success = 0;
                strcpy(res.message, "Fichier introuvable.");
                Rio_writen(connfd, &res, sizeof(res));
                return;
            }
            size_t size = st.st_size;
            Rio_writen(connfd, &size, sizeof(size));

            char buffer[BLOCK_SIZE];
            size_t n;
            while ((n = fread(buffer, 1, BLOCK_SIZE, f)) > 0) {
                Rio_writen(connfd, buffer, n);
            }
            fclose(f);

        }
        else if (req.type == RM){
            char path[MAXLINE];
            snprintf(path, sizeof(path), SERVER_DIR "%.255s", filename);  
            
            //utlisitaion de stat pour lire une seul fois le fichier
            struct stat st;// struct contient des iinfos sur le fichier
            if (stat(path, &st) < 0) {//ici on a utlisee st pour verifier que le fichier est existe
                res.success = 0;
                strcpy(res.message, "Fichier introuvable.");
                Rio_writen(connfd, &res, sizeof(res));
                return;
            }
            if(Fork()== 0){
                execl("/bin/rm", "rm", path, NULL);
                exit(1);
            }
            wait(NULL);
            res.success= 1;
            strcpy(res.message, "OK");
            Rio_writen(connfd, &res, sizeof(res));
        }
        else if (req.type == BYE){
            printf("Client disconnected\n");
        }
    }
}



