#include "csapp.h"
#define BLOCK_SIZE 1024
#define PORT_MASTER 2125//en debut le port 2121 a marche mais on sait pas pourquoi maintenant ne marche pas pour cela on a modifier le port

typedef enum{
    GET ,
    PUT,
    LS,
    BYE,
    RM,  
}typereq_t;

typedef struct{
    typereq_t type;
    char filename[100];
    size_t offset; // pour la reprise de transfert (Q10)
}request_t;

typedef struct{
    int success; // 1 si la requete a été traitée avec succès, 0 sinon
    char message[256]; // message d'erreur ou de succès 
}response_t;

typedef struct{
    char ip[100];
    int port;
}slaveInfo_t;