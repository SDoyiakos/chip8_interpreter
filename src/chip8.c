#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <SDL2/SDL.h>

#define MEMORY_SIZE 4096
#define DISPLAY_HEIGHT 32
#define DISPLAY_WIDTH 64
#define RENDER_SCALE 5
#define REGISTER_COUNT 16
#define STACK_LIMIT 10
#define FONT_START 0x50

#define FIRST_NIBBLE_MASK 0xF000
#define SECOND_NIBBLE_MASK 0x0F00
#define THIRD_NIBBLE_MASK 0x00F0
#define FOURTH_NIBBLE_MASK 0x000F
#define LAST_THREE_NIBBLE_MASK 0x0FFF
#define SECOND_BYTE_MASK 0x00FF

#define FIRST_NIBBLE_SHIFT(X) ( (X & FIRST_NIBBLE_MASK) >> 12)
#define SECOND_NIBBLE_SHIFT(X) ( (X & SECOND_NIBBLE_MASK) >> 8)
#define THIRD_NIBBLE_SHIFT(X) ( (X & THIRD_NIBBLE_MASK) >> 4)
#define SECOND_BYTE_SHIFT(X) ( (X & SECOND_BYTE_MASK) )


int display[DISPLAY_WIDTH][DISPLAY_HEIGHT]; 
uint8_t memory[MEMORY_SIZE];
uint16_t i;
uint16_t pc;
uint8_t v[REGISTER_COUNT];
uint8_t r_flag;
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

uint16_t push(struct Stack stack, uint16_t value) {
  if(stack.size < STACK_LIMIT) {
    stack.entries[stack.size] = value;
    return (uint16_t)value;
  }
  else {
    fprintf(stderr, "Error, pushing to full stack\n");
    return -1;
  }
}

uint16_t pop(struct Stack stack) {
  uint16_t ret_val;
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
  if( (screen.window = SDL_CreateWindow("Chip8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DISPLAY_WIDTH * RENDER_SCALE, DISPLAY_HEIGHT * RENDER_SCALE, 0)) == NULL) {
    fprintf(stderr, SDL_GetError());
    exit(1);
  }

  if( (screen.renderer = SDL_CreateRenderer(screen.window, -1, 0)) == NULL) {
    fprintf(stderr, SDL_GetError());
    exit(1);
  }

  
  if( SDL_RenderSetLogicalSize(screen.renderer, DISPLAY_WIDTH, DISPLAY_HEIGHT) != 0) {
    fprintf(stderr, SDL_GetError());
    exit(1);
  }
  
  SDL_ShowWindow(screen.window);

}

void shutdownScreen() {
  SDL_Quit();
}

int initializeChip8(char* rom_name) {
  pc = 0x200;
  sound_timer = 0;
  delay_timer = 0;
  r_flag = 0;
  initializeStack(stack);
  memcpy(&memory[FONT_START], font, sizeof(font));

  FILE* rom_file = fopen(rom_name, "rb");
  if(rom_file == NULL) {
    fprintf(stderr, "Error opening file, %s\n",  rom_name);
    exit(1);
  }
  loadROM(rom_file);
  initializeScreen();
  return 0;
}

void runLoop() {
  uint16_t curr_inst;
  int is_running = 1;
  while(is_running) {
    
    SDL_Event event;
    while(SDL_PollEvent(&event)) {

      if( event.window.event == SDL_WINDOWEVENT_CLOSE) {
        printf("Window close event\n");
        is_running = 0;
        continue;
      }
    }

    curr_inst = memory[pc] << 8;
    curr_inst+= memory[pc+1];
    pc+=2;

    uint8_t second_nibble = SECOND_NIBBLE_SHIFT(curr_inst);
    uint8_t third_nibble = THIRD_NIBBLE_SHIFT(curr_inst);
    uint8_t fourth_nibble = curr_inst & FOURTH_NIBBLE_MASK;
    uint16_t second_byte = SECOND_BYTE_SHIFT(curr_inst);
    uint16_t last_three_nibble = curr_inst & LAST_THREE_NIBBLE_MASK;
    uint16_t draw_x;
    uint16_t draw_y;
    uint16_t draw_x_offset;
    uint16_t draw_y_offset;
    uint16_t sprite_data;
    switch( FIRST_NIBBLE_SHIFT(curr_inst) ) {
      
      case 0x0: // Execute Machine instr
        if(curr_inst == 0x00E0) {
          for(int x_pixel = 0; x_pixel < DISPLAY_WIDTH; x_pixel++) {
            for(int y_pixel = 0; y_pixel < DISPLAY_HEIGHT; y_pixel++) {
              display[x_pixel][y_pixel] = 0;
            }
          }
        }
        else if(curr_inst == 0x00EE) {
          pc = pop(stack);
        }

        break;

      case 0x2:
        push(stack, pc);
        pc = last_three_nibble;
        break;

      case 0x1: // Jump to addr NNN
      
        printf("Jump Instruction to addr: 0x%x\n", last_three_nibble);
        pc = last_three_nibble;
        break;

      case 0x6:
        
        v[second_nibble] = second_byte;
        printf("Setting register[%d]: 0x%x\n", second_nibble, second_byte);
        break;

      case 0x7:
         
        v[second_nibble]+=second_byte;

        // Setting overflow
        if(v[second_nibble] < second_byte) {
          r_flag = 1;
        }
        break;

      case 0x8:
        switch(fourth_nibble) {
          case 0x0:
            v[second_nibble] = v[third_nibble];
            break;
          default:
            printf("Unimplemented opcode\n");
            break;
        }
        break;

      case 0xA:
        i = last_three_nibble;
        printf("Setting index to: 0x%x\n", last_three_nibble);
        break;

      case 0xD: // Draw case
        v[0xF] = 0;
        draw_x = v[second_nibble] % DISPLAY_WIDTH;
        draw_y = v[third_nibble] % DISPLAY_HEIGHT;
        
        for(int draw_row = 0; draw_row < fourth_nibble; draw_row++) {
          sprite_data = memory[i + draw_row];
          printf("Drawing: 0x%x\n", sprite_data);
          draw_y_offset = draw_y + draw_row;
          if(draw_y_offset >= DISPLAY_HEIGHT) { // Edge of screen
            break;
          }
          
          for(int draw_pixel_col = 0; draw_pixel_col < 8 && draw_x + draw_pixel_col < DISPLAY_WIDTH; draw_pixel_col++) {
            draw_x_offset = draw_x + draw_pixel_col;
            if(0x80 >>  draw_pixel_col & sprite_data ) {
              display[draw_x_offset][draw_y_offset] ^= 1;
              if(display[draw_x_offset][draw_y_offset] == 0) {
                v[0xF] = 1;
              }
            }
          }
        }
        break;

      default:
        printf("Unimplemented opcode\n");
        break;
    }


    SDL_RenderClear(screen.renderer);
    for(int x_pixel = 0; x_pixel < DISPLAY_WIDTH; x_pixel++) {
      for(int y_pixel = 0; y_pixel < DISPLAY_HEIGHT; y_pixel++) {
        if(display[x_pixel][y_pixel]) {
          SDL_SetRenderDrawColor(screen.renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
          SDL_RenderDrawPoint(screen.renderer, x_pixel, y_pixel);
        }
        else {
          SDL_SetRenderDrawColor(screen.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
          SDL_RenderDrawPoint(screen.renderer, x_pixel, y_pixel);
        }
      }
    }
    SDL_RenderPresent(screen.renderer);
    SDL_Delay(16);
  }
  shutdownScreen();
}

int main(int argc, char* argv[]) {
  initializeChip8(argv[1]);
  runLoop();
  return 0;
}
