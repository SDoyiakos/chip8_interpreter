#include <stdint.h>

struct StackEntry {
  uint16_t addr;
  struct StackEntry* next;
};

struct Stack {
  struct StackEntry* head;
  int size;
};
  

void push(struct Stack* stack, uint16_t addr);

uint16_t pop(struct Stack* stack);

uint16_t seeHead(struct Stack* stack);
