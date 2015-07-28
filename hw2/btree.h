#define ORDER 2

struct Data
{
  char code[9];
  char desc[31];
  int dollar;
  int cent; 
  char cate[12];
  int stock;
  int history[13];
  //int offset; //location of data in inventory.txt
};

struct Node
{
  char padding[16];
  int fileOffset;
  int count; //number of Data structs stored in node
  int leaf;
  int offsets[(2 * ORDER) + 1];
  struct Data data[2 * ORDER];
};

int insert(struct Data *item);

int search(struct Node *node, char *code, struct Node *found);

struct Node *getNode(int offset);

int buildData(struct Data *item, char *line);
