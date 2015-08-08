#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

#define BUF_SIZE 1024

int get_subdir(char *path);
int get_files_in_dir(char *path);

/*
int main (int argc, char** argv) {

  struct stat sb;

  char *path = "/home/logan/Desktop/";
  //char *path = "/";

  stat(path, &sb);

  switch (sb.st_mode & S_IFMT) {
    case S_IFBLK:  printf("block dev\n"); break;
    case S_IFCHR:  printf("character device\n"); break;
    case S_IFDIR:  printf("directory\n"); 
                   //get_subdir(path);
                   get_files_in_dir(path);
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
*/
int get_subdir(char *path){

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

int makePerm(struct stat *sb, char *perm){

  // User
  perm[0] = (sb->st_mode & S_IRUSR) ? 'r' : '-';
  perm[1] = (sb->st_mode & S_IWUSR) ? 'w' : '-';
  perm[2] = (sb->st_mode & S_IXUSR) ? 'x' : '-';

  // Group
  perm[3] = (sb->st_mode & S_IRGRP) ? 'r' : '-';
  perm[4] = (sb->st_mode & S_IWGRP) ? 'w' : '-';
  perm[5] = (sb->st_mode & S_IXGRP) ? 'x' : '-';

  // Other 
  perm[6] = (sb->st_mode & S_IROTH) ? 'r' : '-';
  perm[7] = (sb->st_mode & S_IWOTH) ? 'w' : '-';
  perm[8] = (sb->st_mode & S_IXOTH) ? 'x' : '-';

  perm[9] = '\0';

  return 0;
}

int get_files_in_dir(char *path){

  DIR *dp;
  struct stat sb;
  struct dirent *entry;
  struct tm time;
  char entrypath[256];
  char modtime[9];
  char permission[10];

  dp = opendir(path);

  while((entry = readdir(dp))){

    // Get path of entry for stat
    snprintf(entrypath, 256, "%s%s", path, entry->d_name);
    stat(entrypath, &sb); 

    // Format time
    localtime_r(&(sb.st_mtime), &time);
    strftime(modtime, sizeof(modtime), "%D", &time);
    
    makePerm(&sb, permission);


    printf("%s ", entry->d_name); // name
    printf("%lu ", sb.st_size);   // size in bytes
    printf("%s ", modtime);      // mod date
    printf("%s\n", permission);

  }

  closedir(dp);
  return 0;
}
