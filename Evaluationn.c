#include "Shell.h"
#include "Evaluation.h"

#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


int
verifier(int cond, char *s)
{
  if (!cond)
    {
      perror(s);
      return 0;
    }
  return 1;
}


int rediriger(expr_t mode, char *fichier, int *fds)
{
  int flags[]={O_RDONLY,
	       O_WRONLY|O_CREAT|O_TRUNC,
	       O_WRONLY|O_CREAT|O_APPEND,
	       O_WRONLY|O_CREAT|O_TRUNC,
	       O_WRONLY|O_CREAT|O_TRUNC } ;

  int oflag = flags[mode - REDIRECTION_I];
  int fd = open(fichier, oflag, 0666);

  if (verifier(fd !=-1,fichier))
    return 0;
  
  switch(mode){
  case REDIRECTION_I: fds[0]=fd; break;
  case REDIRECTION_O: dup2(fd,STDOUT_FILENO); break;
  case REDIRECTION_A:  fds[1]=fd; break;
  case REDIRECTION_EO: fds[1] = fd;
  case REDIRECTION_E:  fds[2] = fd;
  default : ; 
  }
  return 0;
}


int executer_simple(Expression *e, int *fds, int bg)
{
  int status = 0;
  pid_t pid = fork();

  if (pid == 0)
    {
      
      for (int i = 0; i < 3; i++)
	if (fds[i] != i)
	  dup2(fds[i],i);
      
      if (fds[0] != 0) close(fds[0]);  
      if (fds[1] != 1) close(fds[1]);  
      if (fds[2] != 2 && fds[1] != fds[2]) close(fds[2]); 
      execvp( e->arguments[0], e->arguments);
      perror( e->arguments[0]);
      exit(1);      
    }
  
  for (int i = 0; i < 3; i++)
    if (fds[i] != i)
      {
      if(i != 2 || fds[1] != fds[2])
	close(fds[i]);
      fds[i]=i;
      }
  
  if (!bg){
    waitpid(pid,&status,0);
    status = WIFEXITED(status) ? WEXITSTATUS(status) : 128 +  WTERMSIG(status);
  }
  return status;
}



int evaluer(Expression *e, int *fds, int bg)
{
  int status;
  int tube[2];
  
  if (e == NULL) return 0 ;

  switch(e->type){

  case VIDE :
    exit(0);
    
  case SIMPLE :
    return executer_simple(e,fds,bg);

  case REDIRECTION_I: 
  case REDIRECTION_O: 	
  case REDIRECTION_A: 	
  case REDIRECTION_E: 	
  case REDIRECTION_EO :
    if( !rediriger(e->type, e->arguments[0],fds))
      return 0;
    return evaluer(e->gauche,fds,bg);
    break;
    
  case BG:   
    return evaluer(e->gauche,fds,1);
    break;
    
  case SEQUENCE :
    evaluer(e->gauche,fds,0); // on suppose pas de () pour l'instant
    return evaluer(e->droite,fds,bg);
    break;

  case SEQUENCE_OU :
    if ((status = evaluer(e->gauche,fds,0)))
      status = evaluer(e->droite,fds,0);
    return status;
    break;

  case SEQUENCE_ET :
    if (!(status=evaluer(e->gauche,fds,0)))
      status = evaluer(e->droite,fds,bg);
    return status;
    break;

  case PIPE :
    pipe(tube);
    
    
    { // producteur
	int newfds[3] ;
	newfds[0] = fds[0];
	newfds[1] = tube[1];		
	newfds[2] = fds[2];
	
	evaluer(e->gauche,newfds,1);
      }
    
    
      {
	int newfds[3] ;
	newfds[0] = tube[0];
	newfds[1] = fds[1];		
	newfds[2] = fds[2];
	
    	return evaluer(e->droite,newfds,bg);
      }  
    
  default :   
    printf("not yet implemented \n");
    return 1;
  }
}

int evaluer_expr(Expression *e)
{
  int fds[]={0,1,2};
  int status = 0;
    if (e->type == VIDE)
      return 0;
    return evaluer(e,fds,0);
}