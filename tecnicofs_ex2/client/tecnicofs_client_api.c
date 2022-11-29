#include "tecnicofs_client_api.h"
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdint.h>


int active_session;
int fw , fr ;


int tfs_mount(char const *client_pipe_path, char const *server_pipe_path) {
    ssize_t i;
    uint8_t buffer[41];
    int res;
    char opcode = '1';

    unlink(client_pipe_path);
    memcpy(buffer,&opcode,1);
    memcpy(buffer + 1,client_pipe_path,40);


    
    if (mkfifo(client_pipe_path, 0777) < 0) {return -1;}
    if ((fw = open(server_pipe_path, O_WRONLY)) < 0) {return -1;}
    
    

    i = write(fw,buffer, 40);
    if (i < 0) {return -1;}
    
    if ((fr = open(client_pipe_path, O_RDONLY)) < 0) {
        return -1;
    }
    
    i = read(fr,&res,sizeof(int));
    

    if (i < 0 || res == -1) {
        return -1;
    }

    i = read(fr,&active_session,sizeof(int));

    if (i < 0) {
        return -1;
    }

    return res;
}

int tfs_unmount() {
    ssize_t i;
    int res;
    char opcode = '2';

    uint8_t buffer[1+sizeof(int)];
    memcpy(buffer,&opcode,1);
    memcpy(buffer+1,&active_session,sizeof(int));
    i = write(fw,buffer,1+sizeof(int));
    if (i < 0) {
        return -1;
    }
    i = read(fr,&res,sizeof(int));
    if (i < 0) {
        return -1;
    }
    if (res==0) {
        active_session=0;
        close(fw);
        close(fr);
    }
    return res;
}

int tfs_open(char const *name, int flags) {

    ssize_t i;
    int res;
    char opcode = '3';
    uint8_t buffer[41 + 2*sizeof(int)];
    
    memcpy(buffer,&opcode,1);
    memcpy(buffer+1 ,&active_session, sizeof(int));
    memcpy(buffer+1+sizeof(int),name,40);
    memcpy(buffer+1+sizeof(int)+40,&flags ,sizeof(int));
    
    i = write(fw,buffer, 41 + 2*sizeof(int)  );
    
    if (i < 0) {return -1;}

    i = read(fr,&res,sizeof(int));
    
    if (i < 0) {return -1;}

    return res;
}

int tfs_close(int fhandle) {
    ssize_t i;
    int res;
    char opcode = '4';

    uint8_t buffer[sizeof(char)+2*sizeof(int)];
    memcpy(buffer,&opcode,1);
    memcpy(buffer+1,&active_session,sizeof(int));
    memcpy(buffer+1+sizeof(int),&fhandle,sizeof(int));

    i = write(fw,buffer,1+2*sizeof(int) );
    if (i < 0) {
        return -1;
    }

    i = read(fr,&res,sizeof(int));

    if (i < 0) {
        return -1;
    }
    return res;
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t len){
    ssize_t i;
    int res;
    char opcode = '5';

    uint8_t buff[1 + 2*sizeof(int ) + sizeof(size_t) * (len + 1)];
    memcpy(buff,&opcode,1);
    memcpy(buff +1 , &active_session , sizeof(int));
    memcpy(buff+1+sizeof(int),&fhandle,sizeof(int));
    memcpy(buff+1+2* sizeof(int), &len ,sizeof(size_t));
    memcpy(buff+1+2*sizeof(int) + sizeof(size_t),buffer,len);

    i = write(fw,buff,1 + 2*sizeof(int ) + sizeof(size_t) + len * 1);
    if ( i < 0 ) { return -1;}

    i = read(fr,&res,sizeof(int));
    if ( i < 0 ){return -1;}
    return (ssize_t) res;
}

ssize_t tfs_read (int fhandle, void *buffer, size_t len){
    
    ssize_t i;
    int res;
    char opcode = '6';

    uint8_t buff[1 + 2*sizeof(int ) + sizeof(size_t) ];
    memcpy(buff,&opcode,1);
    memcpy(buff +1 , &active_session , sizeof(int));
    memcpy(buff+1+sizeof(int),&fhandle,sizeof(int));
    memcpy(buff+1+2* sizeof(int), &len ,sizeof(size_t));

    i = write(fw, buff ,1 + 2*sizeof(int ) + sizeof(size_t) );
    if(i<0){return -1;}

    i = read(fr,&res,sizeof(int));
    if ( i < 0 ){return -1;}
    
    
    i=read(fr , buffer,(size_t) res);
    if ( i < 0 ){return -1;}
   
    return (ssize_t) res; 
}

int tfs_shutdown_after_all_closed (){
    ssize_t i;
    int res;
    char opcode = '7';

    uint8_t buffer[1 + sizeof(int ) ];
    memcpy(buffer,&opcode,1);
    memcpy(buffer +1 , &active_session , sizeof(int));
    
    i = write(fw, buffer ,1 + sizeof(int ));
    if(i<0){return -1;}

    i = read(fr,&res,sizeof(int));
    if ( i < 0 ){return -1;}
    
    return res;

}



