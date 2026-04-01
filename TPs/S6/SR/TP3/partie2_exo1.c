/////////////////////////TP 3 PARTIE 2 EXO 1/
///BY ME 
void main(){
    int nb = 0
    while(1){
        if(nb == 6){
            printf("by");
            return;}
    
    while(nb<1){
        alarm(t);
        printf("bib");
        nb++;
    }}
}
//BY CHAT 
int nb = 0;

void handler(int sig) {
    nb++;
    if (nb == 6) {
        printf("bye\n");
        exit(0);
    }
    printf("bip\n");
    alarm(1);  
}
int main() {
    signal(SIGALRM, handler);
    alarm(1);

    while (1) {
    }

    return 0;
}

/////////////////////////////TP3 PARTIE 2 EXO 2
//BY ME 
void handler2(){
    printf("sig recu");
    return;
}
void main(){
    int x =  sigemptyset(sigset_t *p_ens) ;//inisialiser 
    int x4= sigaddset(p_ens, SIGINT);
    nt x5= sigaddset(p_ens, SIGUSR1);

    int x1 =  sigprocmask(SIG_BLOCK, *p_ens, sigset_t *p_ensAncien);

    signal(SIGINT, handler2);
    signal(SIGUSR1, handler2);

    kill(getpid(), SIGINT);
    kill(getpid(), SIGUSR1);
    int sigpending(sigset_t *attends);
    if (sigismember(attends, SIGUSR1))
        printf("- SIGUSR1\n");

    int x2 =sigdelset(p_ens, SIGINT);
    int x3 =sigdelset(p_ens, SIGUSR1);



}

