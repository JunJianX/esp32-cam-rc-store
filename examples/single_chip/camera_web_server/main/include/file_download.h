#ifndef __FILE_DOWNLOAD_H
#define __FILE_DOWNLOAD_H

#define BUFFSIZE 1500
#define TEXT_BUFFSIZE 1024

void file_download_init(char *cfn,char *fn,char*ip,int P);
void file_download_store_task(void);
#endif