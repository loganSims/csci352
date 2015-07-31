#include "btree.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main (int argc, char** argv) {

  printf("page size = %lu\n", sizeof(struct Node));
  return 0;
}
