/*
 * client.c - Fonctions communes pour les clients FTP
 */

#include "csapp.h"
#include "FTP.h"
#include <time.h>
#include <sys/time.h>
#define CLIENT_DIR "clients_files/"
#define PROGRESS_EXT ".progress"

/* -------------------------------------------------------------------
 * do_bye - envoie la commande BYE pour fermer proprement (Q9)
 * ------------------------------------------------------------------- */
void do_bye(int clientfd)
{
    request_t req;
    req.type = BYE;
    req.filename[0] = '\0';
    Rio_writen(clientfd, &req, sizeof(req));
    printf("Deconnexion.\n");
}

/* -------------------------------------------------------------------
 * progress_read - lit l'offset sauvegardé pour un fichier donné.
 * Retourne 0 si pas de fichier de progression (= départ à zéro).
 * ------------------------------------------------------------------- */
long progress_read(const char *filename)
{
    char progress_path[MAXLINE];
    snprintf(progress_path, sizeof(progress_path),
             CLIENT_DIR "%s" PROGRESS_EXT, filename);

    FILE *f = fopen(progress_path, "r");
    if (f == NULL) return 0; /* pas de reprise -> partir de 0 */

    long offset = 0;
    fscanf(f, "%ld", &offset);
    fclose(f);
    return offset;
}

/* -------------------------------------------------------------------
 * progress_write - sauvegarde l'offset courant pour un fichier.
 * Appelé régulièrement pendant le téléchargement.
 * ------------------------------------------------------------------- */
void progress_write(const char *filename, long offset)
{
    char progress_path[MAXLINE];
    snprintf(progress_path, sizeof(progress_path),
             CLIENT_DIR "%s" PROGRESS_EXT, filename);

    FILE *f = fopen(progress_path, "w");
    if (f == NULL) return;
    fprintf(f, "%ld", offset);
    fclose(f);
}

/* -------------------------------------------------------------------
 * progress_delete - supprime le fichier de progression une fois le
 * transfert terminé avec succès.
 * ------------------------------------------------------------------- */
void progress_delete(const char *filename)
{
    char progress_path[MAXLINE];
    snprintf(progress_path, sizeof(progress_path),
             CLIENT_DIR "%s" PROGRESS_EXT, filename);
    remove(progress_path);
}

/* -------------------------------------------------------------------
 * GET : envoie la requete GET au serveur, lit la reponse, puis lit le fichier envoyé
 * par le serveur et l'enregistre localement.
 * ------------------------------------------------------------------- */
void get(int clientfd, rio_t *rio, request_t *req, response_t *res)
{
    // d'abord verifier si le transmission de ce fichier a ete stopped
    size_t offset = progress_read(req->filename);
    //printf("DEBUG: offset = %zu\n", offset);
    if (offset != 0) {
        req->offset = offset;
        printf("Reprise du transfert de '%s' a partir de %ld octets.\n", req->filename, req->offset);
    }

    size_t total_bytes = 0;

    // envoyer la requete au serveur
    Rio_writen(clientfd, req, sizeof(*req)); // envoyer la requete au serveur

    // lire réponse serveur
    if (Rio_readnb(rio, res, sizeof(*res)) <= 0) {
        printf("Erreur communication serveur\n");
        return;
    }

    if (!res->success) {
        printf("Erreur serveur: %s\n", res->message);
        return;
    }

    req->filename[strcspn(req->filename, "\n")] = '\0';

    char path[MAXLINE];
    snprintf(path, sizeof(path), CLIENT_DIR "%.255s", req->filename);
    FILE *f = fopen(path, offset > 0 ? "ab" : "wb");

    if (f == NULL) {
        printf("Erreur ouverture fichier\n");
        return;
    }

    size_t size;
    Rio_readnb(rio, &size, sizeof(size));
    size_t n;
    struct timeval start, end;

    gettimeofday(&start, NULL);

    // LIRE PAR DES BLOCS
    size_t received = req->offset; // pour la reprise de transfert
    char buffer[BLOCK_SIZE];

    while (received < size) {
        size_t to_read = BLOCK_SIZE;
        if (size - received < BLOCK_SIZE)
            to_read = size - received;

        n = Rio_readnb(rio, buffer, to_read);
        fwrite(buffer, 1, n, f);
        received += n;
        total_bytes += n;
        // Q10 : sauvegarder l'offset courant pour permettre la reprise en cas d'interruption
        progress_write(req->filename, offset + received);
    }
    gettimeofday(&end, NULL);

    fclose(f);
    double seconds = (end.tv_sec - start.tv_sec)
                     + (end.tv_usec - start.tv_usec) / 1e6;

    if (seconds == 0) seconds = 0.000001;

    double rate = (total_bytes / 1024.0) / seconds;
    printf("Transfer successfully complete.\n");
    printf("%zu bytes received in %.2f seconds (%.2f Kbytes/s)\n",
           total_bytes, seconds, rate);
    progress_delete(req->filename);
}

/* -------------------------------------------------------------------
 * LS: lister les fichiers qui se trouvent dans le repertoie serveur_files
 * ------------------------------------------------------------------- */
void ls(int clientfd, rio_t *rio, request_t *req, response_t *res){

    Rio_writen(clientfd, req, sizeof(*req)); // envoyer la requete au serveur

    // lire reponse serveur
    if (Rio_readnb(rio, res, sizeof(*res)) <= 0) {
        printf("Erreur communication serveur\n");
        return;
    }

    if (!res->success) {
        printf("Erreur serveur: %s\n", res->message);
        return;
    }

    size_t size ;
    Rio_readnb(rio,&size,sizeof(size));

    char buffer[BLOCK_SIZE];
    size_t received = 0;
    size_t n ;
    while (received < size) {
        size_t to_read = BLOCK_SIZE;
        if (size - received < BLOCK_SIZE)
            to_read = size - received;

        n = Rio_readnb(rio, buffer, to_read);
        fwrite(buffer, 1, n, stdout);
        received += n;
}}

/* -------------------------------------------------------------------
 * RM: supprimer un fichier precis si il existe dans le repertoir serveur_Files
 * ------------------------------------------------------------------- */
void Rm(int clientfd, rio_t *rio, request_t *req, response_t *res){
    Rio_writen(clientfd, req, sizeof(*req)); // envoyer la requete au serveur

    // lire réponse serveur
    if (Rio_readnb(rio, res, sizeof(*res)) <= 0) {
        printf("Erreur communication serveur\n");
        return;
    }

    if (!res->success) {
        printf("Erreur serveur: %s\n", res->message);
        return;
    }
    printf("operation de supression du %s : DONE \n" , req->filename);
}


/* -------------------------------------------------------------------
 * Put: permet de téléverser un fichier sur le serveur FTP.
 * ------------------------------------------------------------------- */
void put(int clientfd, rio_t *rio, request_t *req, response_t *res){

    Rio_writen(clientfd, req, sizeof(*req)); // envoyer la requete au serveur

    req->filename[strcspn(req->filename, "\n")] = '\0';

    char path[MAXLINE];
    snprintf(path, sizeof(path), CLIENT_DIR "%.255s", req->filename);
    FILE *f = fopen(path, "rb");

    if (f == NULL) {
        printf("Erreur ouverture fichier\n");
        return;
    }

    struct stat st;//verifier que le fichier existe
    if (stat(path, &st) < 0) {
        printf("le fichier %s non trouve" , req->filename);
        return;
    }

    size_t size = st.st_size;
    Rio_writen(clientfd, &size, sizeof(size)); // envoi le size du fichier au serveur

    size_t n;
    char buffer[BLOCK_SIZE];
    while ((n = fread(buffer, 1, BLOCK_SIZE, f)) > 0) {
        Rio_writen(clientfd, buffer, n);// envoyer le nombre d'octets lu
    }
    fclose(f);

    if (Rio_readnb(rio, res, sizeof(*res)) <= 0) {
        printf("Erreur communication serveur apres PUT\n");
        return;
    }
    if (!res->success) {
        printf("Erreur serveur PUT: %s\n", res->message);
        return;
    }
    printf("PUT OK: %s\n", res->message);
    
    return;


}

