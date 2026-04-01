/*
 * client.h - Déclarations des fonctions communes pour les clients FTP
 */

#ifndef CLIENT_H
#define CLIENT_H

#include "csapp.h"
#include "FTP.h"
#include <time.h>
#include <sys/time.h>

#define CLIENT_DIR "clients_files/"
#define PROGRESS_EXT ".progress"

/* -------------------------------------------------------------------
 * do_bye - envoie la commande BYE pour fermer proprement (Q9)
 * ------------------------------------------------------------------- */
void do_bye(int clientfd);

/* -------------------------------------------------------------------
 * progress_read - lit l'offset sauvegardé pour un fichier donné.
 * Retourne 0 si pas de fichier de progression (= départ à zéro).
 * ------------------------------------------------------------------- */
long progress_read(const char *filename);

/* -------------------------------------------------------------------
 * progress_write - sauvegarde l'offset courant pour un fichier.
 * Appelé régulièrement pendant le téléchargement.
 * ------------------------------------------------------------------- */
void progress_write(const char *filename, long offset);

/* -------------------------------------------------------------------
 * progress_delete - supprime le fichier de progression une fois le
 * transfert terminé avec succès.
 * ------------------------------------------------------------------- */
void progress_delete(const char *filename);

/* -------------------------------------------------------------------
 * GET : envoie la requete GET au serveur, lit la reponse, puis lit le fichier envoyé
 * par le serveur et l'enregistre localement.
 * ------------------------------------------------------------------- */
void get(int clientfd, rio_t *rio, request_t *req, response_t *res);

void ls(int clientfd, rio_t *rio, request_t *req, response_t *res);

void Rm(int clientfd, rio_t *rio, request_t *req, response_t *res);

void put(int clientfd, rio_t *rio, request_t *req, response_t *res);

#endif 