
/* (C) 1997 Mike Shoyher msh@corbina.net, msh@apache.lexa.ru */

#include<netdb.h>
#include<stdio.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<string.h>
#include<stdlib.h>
#include <sys/types.h> 
#include <sys/time.h>  
#include <unistd.h>    
#include"tac_plus.h"
#include "tacplus_client.h"

int tac_sequence;
int tac_session_id;
int tac_maxtry=3;
int tac_timeout=15;
int tac_fd;                  
char tac_key[128];
struct sockaddr_in tac_port; 
struct hostent *tac_h;       
struct servent *tac_serv;    
char ourhost[128];
int ourhost_len;
char* ourtty="Virtual00";
int ourtty_len;
char* tac_err="NONE";

void fill_tac_hdr(struct tac_plus_pak_hdr* hdr)
{
hdr->version=TAC_PLUS_VER_0;   
hdr->type=TAC_PLUS_AUTHEN;     
hdr->seq_no=tac_sequence;      
hdr->encryption=TAC_PLUS_ENCRYPTED;
hdr->session_id=tac_session_id;
}
void send_auth_cont(char* msg);
int make_auth(char* username,char* password)
{
struct tac_plus_pak_hdr hdr;
struct authen_start as;
int datalength;
void* buf;
void* data;
int data_len;
int buf_len;
struct authen_reply* ar;

fill_tac_hdr(&hdr);

datalength=TAC_AUTHEN_START_FIXED_FIELDS_SIZE;


as.action= TAC_PLUS_AUTHEN_LOGIN ;
as.priv_lvl= TAC_PLUS_PRIV_LVL_MIN ;
as.authen_type = TAC_PLUS_AUTHEN_TYPE_ASCII ;
as.service= TAC_PLUS_AUTHEN_SVC_LOGIN ;
as.user_len=as.port_len=as.rem_addr_len=as.data_len=0;

buf=malloc(buf_len=TAC_PLUS_HDR_SIZE+TAC_AUTHEN_START_FIXED_FIELDS_SIZE+ourtty_len+ourhost_len);

bcopy(ourtty,buf+datalength+TAC_PLUS_HDR_SIZE,ourtty_len);
datalength+=(ourtty_len);
bcopy(ourhost,buf+datalength+TAC_PLUS_HDR_SIZE,ourhost_len);      
datalength+=(ourhost_len); 
as.port_len=ourtty_len;
as.rem_addr_len=ourhost_len;
hdr.datalength= htonl(datalength) ;
bcopy(&hdr,buf,TAC_PLUS_HDR_SIZE);
bcopy(&as,buf+TAC_PLUS_HDR_SIZE,TAC_AUTHEN_START_FIXED_FIELDS_SIZE);
md5_xor(buf, buf+TAC_PLUS_HDR_SIZE, tac_key);
send_data(buf,buf_len,tac_fd);
free(buf);
while ((data_len=read_reply(&data))!=-1){

	ar=data;
	switch (ar->status) {
		case TAC_PLUS_AUTHEN_STATUS_GETUSER:
			free(data);
			send_auth_cont(username);
			break;
		case TAC_PLUS_AUTHEN_STATUS_GETPASS:
			free(data);
			send_auth_cont(password);   
			break;                      
		case TAC_PLUS_AUTHEN_STATUS_PASS:
			return 1;
		case TAC_PLUS_AUTHEN_STATUS_FAIL: 
			tac_err="Authentification failed";
        		return 0;                 
		default :
			tac_err="Protocol error";
			return 0;
	}

}
tac_err="Unknown error";
return 0;
}

void send_auth_cont(char* msg)
{
struct tac_plus_pak_hdr hdr;
struct authen_cont ac;     
int datalength;             
void* buf;                  
int msg_len,buf_len;

msg_len=strlen(msg);
buf_len=TAC_PLUS_HDR_SIZE+TAC_AUTHEN_CONT_FIXED_FIELDS_SIZE+msg_len;
buf=malloc(buf_len);
fill_tac_hdr(&hdr);
ac.user_data_len=ac.flags=0;
ac.user_msg_len=htons(msg_len);
bcopy(msg,buf+TAC_PLUS_HDR_SIZE+TAC_AUTHEN_CONT_FIXED_FIELDS_SIZE,msg_len); 
datalength=msg_len+TAC_AUTHEN_CONT_FIXED_FIELDS_SIZE;                                  
hdr.datalength=htonl(datalength);
bcopy(&hdr,buf,TAC_PLUS_HDR_SIZE);                                    
bcopy(&ac,buf+TAC_PLUS_HDR_SIZE,TAC_AUTHEN_CONT_FIXED_FIELDS_SIZE);  
md5_xor(buf, buf+TAC_PLUS_HDR_SIZE, tac_key);                         
send_data(buf,buf_len,tac_fd);                                        
free(buf);
}

void myerror (char* s)
{
printf("%s\n",s);
exit(1);
}

int init_tac_session(char* host_name, char* port_name, char* key, int timeout)
{
gethostname(ourhost,127);
ourhost_len=strlen(ourhost);
ourtty_len=strlen(ourtty);
srand(time(NULL));
if (timeout>0) tac_timeout=timeout;
strcpy(tac_key,key);
tac_session_id=rand();
tac_sequence=1;
tac_port.sin_family = AF_INET; 
if (*host_name>='0' && *host_name<='9') {                              
     tac_port.sin_addr.s_addr = inet_addr(host_name);                     
} else {                                                                
     tac_h = gethostbyname(host_name);                                    
     if (!tac_h) {                                                         
	 /*myerror("Cannot resolve host name");*/
	 tac_err="Cannot resolve host name";
         return -1;                                                   
     }                                                              
     tac_port.sin_addr = *((struct in_addr *) tac_h->h_addr);                 
}                                                                  
if (port_name==NULL) port_name="tacacs";
if (*port_name>='0' && *port_name<='9') {
  tac_port.sin_port=htons (atoi(port_name));
} else {
    tac_serv=getservbyname(port_name,"tcp");
    if (tac_serv)
      tac_port.sin_port=tac_serv->s_port;
    else {
        /*myerror("Unknown port");*/
	tac_err="Unknown port";
        return -1;
    }
}
if((tac_fd = socket (AF_INET, SOCK_STREAM, 0))<0) return -1;
if (connect (tac_fd, (struct sockaddr *) &tac_port, sizeof tac_port) < 0)
    return -1;
return tac_fd;
}

int send_data(void* buf, int buf_len, int fd)
{
fd_set fds;
int rr;
int try;
struct timeval tv;

FD_ZERO(&fds);
FD_SET(fd,&fds);
tv.tv_sec = tac_timeout;
tv.tv_usec = 0;     
for(try=0;try<tac_maxtry;try++){
	rr=select(fd+1,NULL,&fds,NULL,&tv);
	if (FD_ISSET(fd,&fds)) {
        	if (buf_len!=write(fd,buf,buf_len)) continue;
		return 0;
	}
	myerror("Write error");
	return -1;
}

}
int read_data(void* buf, int buf_len, int fd)                 
{                                                             
fd_set fds;                                                   
int rr;                                                       
int try;                                                      
struct timeval tv;                                            
                                                              
FD_ZERO(&fds);                                                
FD_SET(fd,&fds);                                              
tv.tv_sec = tac_timeout;                                      
tv.tv_usec = 0;                                               
for(try=0;try<tac_maxtry;try++){                              
        rr=select(fd+1,&fds,NULL,NULL,&tv);                   
        if (FD_ISSET(fd,&fds)) {                              
                if (buf_len!=read(fd,buf,buf_len)) continue; 
                return 0;                                     
        }                                                     
        myerror("read error");                               
        return -1;                                            
}                                                             
                                                              
}                                                             

int read_reply(void** data)
{
int data_len;
struct tac_plus_pak_hdr hdr;
if (read_data(&hdr,TAC_PLUS_HDR_SIZE,tac_fd)==-1) return -1;
data_len=ntohl(hdr.datalength);
tac_sequence=hdr.seq_no+1;
*data=malloc(data_len);
if (read_data(*data,data_len,tac_fd)==-1) return -1;
md5_xor(&hdr, *data, tac_key); 
return data_len;
}

void deinit_tac_session()
{
	shutdown(tac_fd,2);
	close(tac_fd);
}


void report()
{
}

int debug;
