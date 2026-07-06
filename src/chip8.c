#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <time.h>

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

uint16_t push(uint16_t value) {
  if(stack.size < STACK_LIMIT) {
    stack.entries[stack.size] = value;
    stack.size++;
    return (uint16_t)value;
  }
  else {
    fprintf(stderr, "Error, pushing to full stack\n");
    return -1;
  }
}

uint16_t pop() {
  uint16_t ret_val;
  if(stack.size == 0) {
    fprintf(stderr, "Error, popping from empty stack");
    ret_val = -1;
    return ret_val;
  }
  else {
    ret_val = stack.entries[stack.size - 1];
    stack.size--;
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

  srand(time(NULL));
		  
  pc = 0x200;
  sound_timer = 0;
  delay_timer = 0;
  v[0xF] = 0;
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

void printRegisters() {

	printf("v[0]: 0x%x v[1]: 0x%x v[2]: 0x%x v[3]: 0x%x\nv[4]: 0x%x v[5]: 0x%x v[6]: 0x%x v[7]: 0x%x\nv[8]: 0x%x v[9]: 0x%x v[A]: 0x%x v[B]: 0x%x\nv[C]: 0x%x v[D]: 0x%x v[E]: 0x%x v[F]: 0x%x\n", v[0], v[1], v[2], v[3], v[4], v[5], v[6] ,v[7], v[8], v[9], v[0xA], v[0xB], v[0xC], v[0xD], v[0xE], v[0xF]);
	printf("PC: 0x%x I: 0x%x Sound: 0x%x, Delay: 0x%x\n", pc, i, sound_timer, delay_timer);
}

void runLoop() {
  uint16_t curr_inst;
  int is_running = 1;
  while(is_running) {
    
    SDL_Event event;
    while(SDL_PollEvent(&event)) {

      if( event.window.event == SDL_WINDOWEVENT_CLOSE) {
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

    printf("Running instruction: 0x%x\n", curr_inst);
    switch( FIRST_NIBBLE_SHIFT(curr_inst) ) {
      case 0x0: // Execute Machine instr

        if(curr_inst == 0x00E0) {
          printf("Clear screen\n");

          for(int x_pixel = 0; x_pixel < DISPLAY_WIDTH; x_pixel++) {
            for(int y_pixel = 0; y_pixel < DISPLAY_HEIGHT; y_pixel++) {
              display[x_pixel][y_pixel] = 0;
            }
          }
        }
        else if(curr_inst == 0x00EE) {     
          printf("Return from subroutine "); 
          pc = pop();
          printf("to addr 0x%x\n", pc);
        }

        break;

      case 0x1: // Jump to addr NNN
        printf("Jump to addr at 0x%x\n", last_three_nibble);

        pc = last_three_nibble;
        break;

      case 0x2:
        printf("Jump to subroutine at 0x%x\n", last_three_nibble);
        push(pc);
        pc = last_three_nibble;
        break;

      case 0x3:
        printf("Skip next instr if 0x%x == 0x%x\n", v[second_nibble], second_byte);
        if(v[second_nibble] == second_byte) {
          pc+=2;
        }
        break;

      case 0x4:
        printf("Skip next instr if 0x%x != 0x%x\n", v[second_nibble], second_byte);
        if(v[second_nibble] != second_byte) {
          pc+=2;
        }
        break;

      case 0x5:
        if(fourth_nibble == 0) {
          printf("Skip next instr if v[%x] == v[%x]\n", second_nibble, third_nibble);
          if(v[second_nibble] == v[third_nibble]) {
            pc+=2;
          }
        }
        else {
          printf("Unimplemented opcode\n");
        }
        break;    

      case 0x6:
        printf("Setting register v[%d] to %d\n", second_nibble, second_byte);
        v[second_nibble] = second_byte;
        break;

      case 0x7:
        printf("Adding %d to v[%d]\n", second_byte, second_nibble);
        v[second_nibble]+=second_byte;

        // Setting overflow
        if(v[second_nibble] < second_byte) {
          v[0xF] = 1;
        }
        break;

      case 0x8:
        switch(fourth_nibble) {
          case 0x0:
            printf("Set v[%d] to v[%d]\n", second_nibble, third_nibble);
            v[second_nibble] = v[third_nibble];
            break;
		  case 0x1:
			printf("Set v[%d] to v[%d] | v[%d]", second_nibble, second_nibble, third_nibble);
			v[second_nibble] = v[second_nibble] | v[third_nibble];
			break;
		  case 0x2:
			printf("Set v[%d] to v[%d] & v[%d]", second_nibble, second_nibble, third_nibble);
			v[second_nibble] = v[second_nibble] & v[third_nibble];
			break;
		  case 0x3:
			printf("Set v[%d] to v[%d] ^ v[%d]", second_nibble, second_nibble, third_nibble);
			v[second_nibble] = v[second_nibble] ^ v[third_nibble];
			break;
		  case 0x4:
			printf("Set v[%d] = v[%d] + v[%d]",second_nibble, second_nibble, third_nibble);
			if((uint16_t)v[second_nibble] + (uint16_t)v[third_nibble] > 255) {
				v[0xF] = 1;				
			}
			else {
				v[0xF] = 0;
			}
			v[second_nibble] = v[second_nibble] + v[third_nibble];
			break;
		  case 0x5:
			printf("Set v[%d] = v[%d] - v[%d]",second_nibble, second_nibble, third_nibble);
			if(v[second_nibble] >= v[third_nibble]) {
				v[0xF] = 1;
			}
			else {
				v[0xF] = 0;
			}
			v[second_nibble] = v[second_nibble] - v[third_nibble];
			break;
		  case 0x6:
			printf("Shifting V[%d] to right", second_nibble);
			v[second_nibble] = v[third_nibble];
			if(v[second_nibble] & 1) {
				v[0xF] = 1;
			}
			else {
				v[0xF] = 0;
			}
			v[second_nibble] = v[second_nibble] >> 1;
			break;
		  case 0x7:
			printf("Set v[%d] = v[%d] - v[%d]",second_nibble, third_nibble, second_nibble);
			if(v[third_nibble] >= v[second_nibble]) {
				v[0xF] = 1;
			}
			else {
				v[0xF] = 0;
			}
			v[second_nibble] = v[third_nibble] - v[second_nibble];
			break;
		  case 0xE:
			printf("Shifting V[%d] to left", second_nibble);
			v[second_nibble] = v[third_nibble];
			if(v[second_nibble] & 0x80) {
				v[0xF] = 1;
			}
			else {
				v[0xF] = 0;
			}
			v[second_nibble] = v[second_nibble] << 1;
			break;
          default:
            printf("Unimplemented opcode\n");
            break;
        }
        break;

      case 0x9:
        if(fourth_nibble == 0) {
          printf("Skip next instr if v[%x] != v[%x]\n", second_nibble, third_nibble);
          if(v[second_nibble] != v[third_nibble]) {
            pc+=2;
          }
        }
        else {
          printf("Error, invalid opcode\n");
        }
        break;

      case 0xA:
        printf("Set i to %x\n", last_three_nibble);
        i = last_three_nibble;
        break;
	  case 0xB:
		i = last_three_nibble + v[0];
		break;
	  case 0xC:
		v[second_nibble] = (rand() % 0x100) & second_byte;
		break;
      case 0xD: // Draw case
        printf("Draw sprite at %d\n", i);
        v[0xF] = 0;
        draw_x = v[second_nibble] % DISPLAY_WIDTH;
        draw_y = v[third_nibble] % DISPLAY_HEIGHT;
        
        for(int draw_row = 0; draw_row < fourth_nibble; draw_row++) {
          sprite_data = memory[i + draw_row];
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
	  case 0xF:
		switch(second_byte) {
			case 0x55: // Store 
				for(uint8_t j = 0;j <= second_nibble;j++) {
					memory[i+j] = v[j];
				}
				break;
			case 0x65: // Load
				for(uint8_t j = 0;j <= second_nibble;j++) {
					v[j] = memory[i+j];  
				}
				break;
      		default:
        	printf("Unimplemented opcode\n");
        	break;
		}
		break;
      default:
        printf("Unimplemented opcode\n");
        break;

	
    }

	printRegisters();

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
