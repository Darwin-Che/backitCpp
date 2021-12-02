#ifndef CLIENT_H
#define CLIENT_H

int cl_connect();
int cl_ls(int argc, char ** argv);
int cl_sync_download(int argc, char ** argv);
int cl_rm(int argc, char ** argv);
int cl_sync_upload(int argc, char ** argv);


#endif
