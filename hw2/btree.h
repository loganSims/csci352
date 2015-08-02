#define ORDER 2

#define PADDING ((ORDER - 2 ) * 2)

struct Data
{
  char code[9];
  char desc[31];
  int dollar;
  int cent; 
  char cate[12];
  int stock;
  int history[13];
  int padding;
};

struct Node
{
  int padding[PADDING];
  int fileOffset;
  int count; //number of Data structs stored in node
  int leaf;
  int offsets[(2 * ORDER) + 1];
  struct Data data[2 * ORDER];
};

int initBtree(struct Data *item);

int initNode(struct Node *node);

int insert(struct Data *item);

int search(struct Node *node, char *code, struct Node *found);

int getSibOffset(struct Node *node, char* choice);

int getSibItem(struct Node *node, struct Data item, char *choice);

int deleteKey(struct Node *node, char *code);

int saveNode(struct Node *node);

int getNode(int offset, struct Node *node);

int buildData(struct Data *item, char *line);
