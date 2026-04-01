/*
 * Copyright (C) 2002, Simon Nieuviarts
 */

#include <stdio.h>
#include <stdlib.h>
#include "readcmd.h"
#include "csapp.h"
#include <fcntl.h>
#include <unistd.h>

#define MAX_LINE 1024

int main(int argc , char *argv[])
{
    if (argc!=2)
    {
        printf("usage : shell fich.tx\n");
        exit(0);
    }

    int fich = open(argv[1], O_RDONLY);
    if (fich == -1)
    {
        perror("Erreur ouverture");
        exit(0);
    }
    // rediriger le fichier vers stdin
    dup2(fich, STDIN_FILENO);
    close(fich);

    while (1) {
		struct cmdline *l;
		int i, j;
		//printf("shell> ");
		l = readcmd();

		/* If input stream closed, normal termination */
		if (!l) {
			printf("exit\n");
			exit(0);
		}

		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}

		if (l->in) printf("in: %s\n", l->in);
		if (l->out) printf("out: %s\n", l->out);

		/* Display each command of the pipe */
		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];
            if (strcmp(cmd[0], "#") == 0)
            {
                continue;
            }
            
			printf("seq[%d]: ", i);
			for (j=0; cmd[j]!=0; j++) {
				printf("%s ", cmd[j]);
			}}
            /////////////////////////////////////////////////////////////////////////////////
            // pour chaque commande doit executer comme un fils pour le processus shell
            // et rediger la sortie de chaque commande vers l'autre commande .
            // ls | echo : on doit pas creer un fichier intermediaire il faut rediger directemanet
            /*
            int k = i; // i =  nb totale des commandes
            while ( k < i)
            {
                int fd[2];
                pipe(fd);
                pid_t fils = fork();
                if (fils == -1)
                {
                    printf("erreur de fils");
                    exit(0);
                }
                if (fils == 0)
                {   
                    k++;
                    close(fd[0]);/// fermeture de lire 
                    dup2(fd[1], STDOUT_FILENO);
                    char **commande =  l->seq[k];
                    char **arguments;
                    for (size_t r = 0; commande[r]!=0 ; r++)
                    {
                        arguments[r] = commande[r+1];
                    }
                    
                    execl(commande[0],arguments,NULL);
                }
                pid_t fils2  = fork();
                if (fils2 == -1)
                {
                    printf("erreur de fils");
                    exit(0);
                }
                
                if (fils2 == 0)
                {
                    k++;
                    close(fd[1]);
                    dup2(fd[0], STDIN_FILENO);
                    char **commande =  l->seq[k];
                    char **arguments;
                    for (size_t r = 0; commande[r]!=0 ; r++)
                    {
                        arguments[r] = commande[r+1];
                    }
                    execl(commande[0],arguments,NULL);
                }
            }*/
            
            /////////////////////////////////////////////////////////////////////////////////
            /*
			if (strcmp(cmd[0], "quit") == 0)
			{
                printf("\n");
				printf("exit\n");
				exit(0);
			}*/
    int in_fd = STDIN_FILENO;
    int fd[2];
    pid_t pid;

    for (i = 0; l->seq[i] != NULL; i++) {

        // creer un pipe sauf pour la derniere commande
        if (l->seq[i+1] != NULL) {
            pipe(fd);
    }

        pid = fork();

        if (pid == 0) { 

        // entrée depuis le pipe précédent
            if (in_fd != STDIN_FILENO) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
        }

        // sortie vers pipe suivant
        if (l->seq[i+1] != NULL) {
            dup2(fd[1], STDOUT_FILENO);
            close(fd[0]);
            close(fd[1]);
        }

        execv(l->seq[i][0], l->seq[i]);

        perror("exec");
        exit(1);
    }

    // parent
    if (in_fd != STDIN_FILENO)
        close(in_fd);

    if (l->seq[i+1] != NULL) {
        close(fd[1]);
        in_fd = fd[0];
    }
}
    while (wait(NULL) > 0);

			
	printf("\n");
    exit(0);
		}
	}
