
#ifndef TACPLUS_CLIENT
#define TACPLUS_CLIENT

extern int  make_auth (char* username, char* password); 

extern int init_tac_session (char* host_name,char* port_name, char* key, int timeout);

extern void deinit_tac_session();
extern char* tac_err;

#endif
