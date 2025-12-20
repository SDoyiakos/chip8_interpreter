#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <SDL2/SDL.h>

#define MEMORY_SIZE 4096
#define DISPLAY_HEIGHT 64
#define DISPLAY_WIDTH 32
#define REGISTER_COUNT 16
#define STACK_LIMIT 10
#define FONT_START 0x50

int display[DISPLAY_WIDTH][DISPLAY_HEIGHT]; 
uint8_t memory[MEMORY_SIZE];
uint16_t i;
uint16_t pc;
uint8_t v[REGISTER_COUNT];
uint8_t sound_timer;
uint8_t delay_timer;

uint8_t font[] = 
{
0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
0x20, 0x60, 0x20, 0x20, 0x70, // 1
0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
0x90, 0x90, 0xF0, 0x10, 0x10, // 4
0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
0xF0, 0x10, 0x20, 0x40, 0x40, // 7
0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
0xF0, 0x90, 0xF0, 0x90, 0x90, // A
0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
0xF0, 0x80, 0x80, 0x80, 0xF0, // C
0xE0, 0x90, 0x90, 0x90, 0xE0, // D
0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

struct SDL_Screen {
  SDL_Window* window;
  SDL_Renderer* renderer;
} screen;

struct Stack {
  uint16_t entries[STACK_LIMIT];
  int size;
} stack;

struct Stack initializeStack() {
  struct Stack stack;
  stack.size = 0;
  return stack;
}

int push(struct Stack stack, uint8_t value) {
  if(stack.size < STACK_LIMIT) {
    stack.entries[stack.size] = value;
    return (uint8_t)value;
  }
  else {
    fprintf(stderr, "Error, pushing to full stack\n");
    return -1;
  }
}

uint8_t pop(struct Stack stack) {
  uint8_t ret_val;
  if(stack.size == 0) {
    fprintf(stderr, "Error, popping from empty stack");
    ret_val = 0;
    return ret_val;
  }
  else {
    ret_val = stack.entries[stack.size];
    fprintf(stderr, "Error, popping from empty stack\n");
    return ret_val;
  }
}

void loadROM(FILE* rom_file) {
  uint16_t mem_index = 0x200;
  int next_byte;

  while( (next_byte = fgetc(rom_file)) != EOF) {
    memory[mem_index] = (uint8_t)next_byte;
    mem_index++;
    if(mem_index >= MEMORY_SIZE) {
      fprintf(stderr, "Error, ROM too large\n");
      exit(1);
    }
  }
}

void initializeScreen() {
  if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    fprintf(stderr, SDL_GetError());
    exit(1);
  }
  if(SDL_CreateWindow("Chip8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0) != 0) {
    fprintf(stderr, SDL_GetError());
    exit(1);
  }
  
}

int initializeChip8(char* rom_name) {
  pc = 0x200;
  sound_timer = 0;
  delay_timer = 0;
  initializeStack(stack);
  memcpy(&memory[FONT_START], font, sizeof(font));

  FILE* rom_file = fopen(rom_name, "rb");
  if(rom_file == NULL) {
    fprintf(stderr, "Error opening file, %s\n",  rom_name);
    exit(1);
  }
  loadROM(rom_file);
  return 0;
}

int main(int argc, char* argv[]) {
  initializeChip8(argv[1]);
  for(int index = 0x200; index < 0x300; index++) {
    printf("[0x%x]: 0x%x\n", index, memory[index]);
  }
  return 0;
}
