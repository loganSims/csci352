#define ORDER 2

struct Data
{
  char code[9];
  char desc[31];
  int dollar;
  int cent; 
  char cate[13];
  int stock;
  int history[13];
  //int offset; //location of data in inventory.txt
};

struct Node
{
  long fileOffset;
  int count; //number of Data structs stored in node
  int leaf;
  long offsets[(2 * ORDER) + 1];
  struct Data data[2 * ORDER];
};

int insert(struct Data *item);

struct Node *getNode(long offset);