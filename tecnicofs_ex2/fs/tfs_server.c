#include "operations.h"
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define S 1
#define CLOSE 0
#define OPEN 1

typedef struct {
    int state;
    char pipe_client[40];
} session;

session aux[S];
int server_session=0;

int main(int argc, char **argv) {
    int fr , fw ,session_received,res;
    ssize_t m,n,o;
    char op_code;
    char* pipe_client;
    pipe_client=(char*) malloc(40*1);
    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return 1;
    }

    char *pipename = argv[1];
    printf("Starting TecnicoFS server with pipe called %s\n", pipename);
    
    unlink(pipename);

    if (mkfifo(pipename, 0777) < 0) {
        return -1;
    }

    if ((fr = open(pipename, O_RDONLY)) < 0) {
        return -1;
    }
 
    
    tfs_init();

    while(1){
        m =  read(fr,&op_code,1);
        if (m < 0) {return -1;}
        if (op_code=='1') {
            o = read(fr,pipe_client,40);
            if (o < 0) {
                return -1;
            }
            if ((fw = open(pipe_client, O_WRONLY)) < 0) {
                res=-1;
                n=write(fw,&res,sizeof(int));
            }
            server_session++;
            if (server_session > S) {
                res=-1;
                n=write(fw,&res,sizeof(int));
            }
            else {
                res=0;
                n=write(fw,&res,sizeof(int));
                m=write(fw,&server_session,sizeof(int));
                aux[server_session-1].state=OPEN;
                strcpy(aux[server_session-1].pipe_client,pipe_client);
            }

            if (n < 0 || m < 0) {
                res=-1;
                n=write(fw,&res,sizeof(int));
            }
            
           
        }
        
        else if (op_code=='2')  {

            m=read(fr,&session_received,sizeof(int));
            strcpy(pipe_client,aux[session_received-1].pipe_client);
            if ((fw = open(pipe_client, O_WRONLY)) < 0) {
                
            }
            if (m < 0) {
                res=-1;
                n=write(fw,&res,sizeof(int));
            }
            aux[session_received-1].state=CLOSE;
            res=0;
            m=write(fw,&res,sizeof(int));
            unlink(aux[session_received-1].pipe_client);
            if (m<0) {
                res=-1;
                n=write(fw,&res,sizeof(int));
            }
        }

        else if (op_code=='3')  {
            
            m=read(fr,&session_received,sizeof(int));
            
            if (m<0) {
                return -1;
            }
            
            strcpy(pipe_client,aux[session_received-1].pipe_client);
            if ((fw = open(pipe_client, O_WRONLY)) < 0) {
                res=-1;
                n=write(fw,&res,sizeof(int));
            }
            char* name=malloc(1*40 );
            m=read(fr,name,1*40);
            if (m<0) {
                res=-1;
                n=write(fw,&res,sizeof(int));
            }
            
            int flags;
            m=read(fr,&flags,sizeof(int));
            
            if (m<0) {
                res=-1;
                n=write(fw,&res,sizeof(int));
            }
            

            
            res = tfs_open(name,flags);
            
            m=write(fw,&res,sizeof(int));

            if (m<0) {
                res=-1;
                n=write(fw,&res,sizeof(int));
            }

        }
        
        else if (op_code=='4') {
            m=read(fr,&session_received,sizeof(int));
            if (m<0) {
                return -1;
            }
            strcpy(pipe_client,aux[session_received-1].pipe_client);
            if ((fw = open(pipe_client, O_WRONLY)) < 0) {
                res=-1;
                n=write(fw,&res,sizeof(int));
            }
            int file;
            m=read(fr,&file,sizeof(int));
            if (m<0) {
                res=-1;
                n=write(fw,&res,sizeof(int));
            }
            res = tfs_close(file);
            m=write(fw,&res,sizeof(int));
            if (m<0) {
                res=-1;
                n=write(fw,&res,sizeof(int));
            }
        }

        else if(op_code=='5') {
            
            m = read( fr, &session_received , sizeof(int));
            if ( m < 0 ){return -1;}
            strcpy(pipe_client,aux[session_received-1].pipe_client);
            if ((fw = open(pipe_client, O_WRONLY)) < 0) {
                res=-1;
                n=write(fw,&res,sizeof(int));
            }
            int file;
            m=read(fr,&file,sizeof(int));
            if (m<0) {
                res=-1;
                n=write(fw,&res,sizeof(int));
            }
            size_t len;
            m=read(fr,&len , sizeof(size_t));
            if ( m < 0 ){
                res = -1 ;
                n = write(fw,&res , sizeof(int));
            }
            char *buffer =  malloc(1 * len);
            m=read(fr,buffer,1*len);
            if( m< 0 ){
                res = -1 ;
                n = write(fw,&res,sizeof(int));
            }
            res = (int) tfs_write( file, buffer, len);
            m=write(fw,&res,sizeof(int));
            if (m<0) {
                res=-1;
                n=write(fw,&res,sizeof(int));

            }
        }

        else if(op_code=='6') { 

            m = read( fr, &session_received , sizeof(int));
            if ( m < 0 ){return -1;}
            strcpy(pipe_client,aux[session_received-1].pipe_client);
            if ((fw = open(pipe_client, O_WRONLY)) < 0) {
                res=-1;
                n=write(fw,&res,sizeof(int));
            }
            int file;
            m=read(fr,&file,sizeof(int));
            if (m<0) {
                res=-1;
                n=write(fw,&res,sizeof(int));
            }
            size_t len;
            m=read(fr,&len , sizeof(int));
            if ( m < 0 ){
                res = -1 ;
                n = write(fw,&res , sizeof(int));
            }
            char *buffer =  malloc(1 * len);
            res =(int) tfs_read( file , buffer, len);
            m = write(fw , &res , sizeof(int));
            if (m<0) {
                res=-1;
                n=write(fw,&res,sizeof(int));
            }
            m= write(fw, buffer ,(size_t) res );
            if (m<0) {
                res=-1;
                n=write(fw,&res,sizeof(int));
            }
        }

        else if(op_code=='7')  {
            m = read(fr , &session_received , sizeof(int));
            if ( m < 0 ){ return -1 ;}
            strcpy(pipe_client , aux[session_received -1 ].pipe_client);
            if ((fw = open(pipe_client, O_WRONLY)) < 0) {
                res=-1;
                n=write(fw,&res,sizeof(int));
            }
            res = tfs_destroy_after_all_closed();
            m = write(fw,&res,sizeof(int));
            if(m<0){return -1;}
            tfs_destroy();
            break;
        }

    }
    
    return 0;
}