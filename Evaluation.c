#include "Shell.h"
#include "Evaluation.h"
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>



int evaluer_expr(Expression *e){

struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset (&sa.sa_mask);
    while( ( waitpid(-1, &status, WNOHANG)) > 0){}
    sigaction (SIGCHLD, &sa, NULL);

   if (e->type == VIDE){
     fprintf(stderr, "not yet implemented \n");
    return 0;
}

   int status;
   while( (waitpid(-1, &status, WNOHANG)) > 0){
            
        }
    
     if (e == NULL) return 0 ;
   

     if (e->type == SIMPLE){
        return executer_simple(e);
     } 
     if (e->type == SEQUENCE){
           evaluer_expr(e->gauche);
         return   evaluer_expr(e->droite );
     }
     if (e->type == SEQUENCE_ET ){
         if (!(status=  evaluer_expr(e->gauche ))){
             status =   evaluer_expr(e->droite );
             return status;
         }
     }
     if (e->type == SEQUENCE_OU){
           if ((status =   evaluer_expr(e->gauche ))){
                status =   evaluer_expr(e->droite );
                return status;
           }
     }

     if (e->type == BG){
         pid_t pid = fork();
         int status = 0;
    
        if(pid == 0){
        execvp( e->gauche->arguments[0], e->gauche->arguments);
        perror( e->gauche->arguments[0]);
        exit(1);
       
    }
    
    return status;
     }
     int oldfd=dup(STDOUT_FILENO);
     int oldfd1=dup(STDIN_FILENO);
     int oldfd2=dup(STDERR_FILENO);


     if (e->type == REDIRECTION_O){
  
        int fd = open(e->arguments[0], O_CREAT|O_TRUNC|O_WRONLY, 0666);
        dup2(fd,STDOUT_FILENO);
        
        status =   evaluer_expr(e->gauche);
        wait(&status);
       dup2(oldfd,1);
    return status;
     }
     if (e->type == REDIRECTION_I){
        int fd = open(e->arguments[0],O_RDONLY, 0666);
        dup2(fd,STDIN_FILENO);
        
        status = evaluer_expr(e->gauche);
       dup2(oldfd1,0);
       
    return status;
     }
     if (e->type == REDIRECTION_A){
         int fd = open(e->arguments[0],O_CREAT|O_APPEND|O_WRONLY, 0666);
         dup2(fd,STDOUT_FILENO);
        
        status = evaluer_expr(e->gauche);
        dup2(oldfd,1);
        return status;
     }
     if (e->type == REDIRECTION_E){
         int fd = open(e->arguments[0],O_CREAT|O_TRUNC|O_WRONLY, 0666);
         dup2(fd,STDERR_FILENO);
        
        status = evaluer_expr(e->gauche);
        dup2(oldfd2,2);
        return status;
     }
     if (e->type == REDIRECTION_EO){
         int fd = open(e->arguments[0],O_CREAT|O_TRUNC|O_WRONLY, 0666);
         dup2(fd,STDERR_FILENO);
         dup2(fd,STDOUT_FILENO);
         status = evaluer_expr(e->gauche);
         dup2(oldfd2,2);
         dup2(oldfd,1);
         return status;
     }

     if (e->type == PIPE){
         int tube[2];
         enum{ R, W};
         int n = pipe(tube);
         if(fork()){
            close (tube[R]);
            dup2 (tube[W], STDOUT_FILENO);
            status = evaluer_expr(e->gauche);
           
         }else{
            close (tube[W]); 
            dup2 (tube[R], STDIN_FILENO); 
            status = evaluer_expr(e->droite);
         }
        return status;
     }
     return 0;
}

int executer_simple(Expression *e){

    if (strcmp(e->arguments[0],"echo")==0){ // echo
        int i=1;
        while(e->arguments[i]!=NULL){
            if(e->arguments[i+1]==NULL){
                printf("%s",e->arguments[i]);
              
            }else{
        printf("%s ",e->arguments[i]);
            }
            i++;
        }
        putchar('\n');
        return 0;
     }
     else{
    pid_t pid = fork();
    int status = 0;

    if(pid == 0 && strcmp(e->arguments[0],"echo")!=0){
        execvp( e->arguments[0], e->arguments);
        perror( e->arguments[0]);
        exit(1);
    }

    waitpid(pid,&status,0);
    
    return status;
     }

    
}
