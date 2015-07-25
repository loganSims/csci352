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
  int offset; //location of data in inventory.txt
};

struct Node
{
  int count; //number of Data structs stored in node
  long offsets[(2 * ORDER) + 1];
  struct Data data[2 * ORDER];
};

int insert(struct Node *root, struct Data *item);
