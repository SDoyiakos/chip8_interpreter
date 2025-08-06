#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

void push(Stack* stack, uint16_t addr) {
  if(stack->size == 0) {
    stack->head = malloc(sizeof(StackEntry));
    if(stack->head == NULL) { // Check if stack allocated
      exit(1);
    }
    (stack->head)->addr = addr;
    stack->size++;
  }
}

uint16_t pop(Stack* stack) {
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

uint16_t seeHead(Stack* stack) {
  if(stack->size > 0) {
    return head->addr;
  }
  else {
    return -1;
  }
}
