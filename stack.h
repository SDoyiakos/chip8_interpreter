#include <stdint.h>

struct StackEntry {
  uint16_t addr;
  StackEntry* next;
};

struct Stack{
  struct StackEntry* head;
  int size;
  
};
  

void push(Stack* stack, uint16_t addr);

uint16_t pop(Stack* stack);

uint16_t seeHead(Stack* stack);
