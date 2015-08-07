#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <dirent.h>



#define BUF_SIZE 1024

int get_dir_in_dir(char *path);

int main (int argc, char** argv) {

  struct stat sb;

  char *path = "/home/logan/Desktop/";
  //char *path = "/";

  stat(path, &sb);

  switch (sb.st_mode & S_IFMT) {
    case S_IFBLK:  printf("block dev\n"); break;
    case S_IFCHR:  printf("character device\n"); break;
    case S_IFDIR:  printf("directory\n"); 
                   get_dir_in_dir(path);
                   break;
    case S_IFREG:  printf("regular file\n"); 
                   break;
    case S_IFIFO:  printf("fifo\n"); break;
    case S_IFLNK:  printf("symlink\n"); break;
    case S_IFSOCK: printf("socket\n"); break;
    default:       printf("unknown?\n"); break;
  }

  return 0;
}

int get_dir_in_dir(char *path){

  DIR *dp;
  struct stat sb;
  struct dirent *entry;

  dp = opendir(path);

  char entrypath[256];

  while((entry = readdir(dp))){
    snprintf(entrypath, 256, "%s%s", path, entry->d_name);
    stat(entrypath, &sb); 
    if((sb.st_mode & S_IFMT) == S_IFDIR){
      printf("%s\n", entry->d_name);
    }
  }

  closedir(dp);
  return 0;
}



