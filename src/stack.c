#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

void push(struct Stack* stack, uint16_t addr) {
  if(stack->size == 0) {
    stack->head = malloc(sizeof(struct StackEntry));
    if(stack->head == NULL) { // Check if stack allocated
      exit(1);
    }
    (stack->head)->addr = addr;
    stack->size++;
  }
}

uint16_t pop(struct Stack* stack) {
  uint16_t ret_val;
  if(stack->size > 0) {
    ret_val = (stack->head)->addr;
    stack->head = (stack->head)->next;
    stack->size--;
    return ret_val;
  }
  else {
    return -1;
  }
}

uint16_t seeHead(struct Stack* stack) {
  if(stack->size > 0) {
    return (stack->head)->addr;
  }
  else {
    return -1;
  }
}
