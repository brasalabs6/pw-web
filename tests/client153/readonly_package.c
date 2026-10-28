#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
int main(void) {
 struct stat st; if (stat("/fixture.pck", &st) || st.st_size != 8) return 1;
 FILE *f=fopen("/fixture.pck","rb"); if(!f) return 2;
 if(fseek(f,4,SEEK_SET)) return 3;
 char data[3]={0}; if(fread(data,1,2,f)!=2 || data[0]!='e' || data[1]!='f') return 4;
 fclose(f); int fd=open("/fixture.pck",O_WRONLY|O_TRUNC); if(fd>=0){close(fd);return 5;}
 return 0;
}
