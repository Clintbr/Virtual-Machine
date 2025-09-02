#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include <stdlib.h>
#include <time.h>
#define halt 0
#define puschc 1
#define add 2
#define sub 3
#define mul 4
#define div 5
#define mod 6
#define rdint 7
#define wrint 8
#define rdchr 9
#define wrchr 10
#define immediate(x) ((x) & 0x00FFFFFF)
#define sign_extend(i) ((i) & 0x00800000 ? (i) | 0xFF000000 : (i))
int version = 1;
int pc = 0;
int stackpointer = 0;
unsigned int stack[10];

unsigned int prog1[] = {
    (puschc << 24) | immediate(3),
    (puschc << 24) | immediate(4),
    (add << 24),
    (puschc << 24) | immediate(10),
    (puschc << 24) | immediate(6),
    (sub << 24),
    (mul << 24),
    (wrint << 24),
    (puschc << 24) | immediate(10),
    (wrchr << 24),
    (halt << 24)
};

unsigned int prog2[] = {
    (puschc << 24) | immediate(-2),
    (rdint << 24),
    (mul << 24),
    (puschc << 24) | immediate(3),
    (add << 24),
    (wrint<< 24),
    (puschc << 24) | immediate('\n'),
	(wrchr << 24),
    (halt << 24)
};

unsigned int prog3[] = {
    (rdchr << 24),
    (wrint << 24),
    (puschc << 24) | immediate('\n'),
    (wrchr << 24),
    (halt << 24)
};



int execute(unsigned int instruction) {
    int opcode = instruction >> 24;
    signed int value = instruction & 0x00FFFFFF;
    signed int result = 0;
    switch (opcode)
    {
    case puschc:
        value = sign_extend(value);
        stack[stackpointer] = value;
        stackpointer = stackpointer + 1;
        break;
    case add:
        result = stack[stackpointer -2] + stack[stackpointer - 1];
        stack[stackpointer - 2] = result;
        stackpointer = stackpointer -1;
        break;
    case sub:
        result = stack[stackpointer -2] - stack[stackpointer - 1];
        stack[stackpointer - 2] = result;
        stackpointer = stackpointer -1;
        break;
    case mul:
        result = stack[stackpointer -2] * stack[stackpointer - 1];
        stack[stackpointer - 2] = result;
        stackpointer = stackpointer -1;
        break;
    case div:
        if(stack[stackpointer - 1] == 0) {
            printf("cannot divide in 0");
            break;
        }
        result = stack[stackpointer -2] / stack[stackpointer - 1];
        stack[stackpointer - 2] = result;
        stackpointer = stackpointer -1;
        break;
    case mod:
        if(stack[stackpointer - 1] == 0) {
            printf("cannot divide in 0");
            break;
        }
        result = stack[stackpointer -2] % stack[stackpointer - 1];
        stack[stackpointer - 2] = result;
        stackpointer = stackpointer -1;
        break;
    case rdint:
        scanf("%d", &result);
        stack[stackpointer] = result;
        stackpointer = stackpointer + 1;
        break;
    case wrint:
        result = stack[stackpointer - 1];
        printf("%d \n",(signed int)sign_extend(result));
        break;
    case rdchr:{
        char chr_result = ' ';
        scanf("%c", &chr_result);
        result = (unsigned int)chr_result;
        stack[stackpointer] = result;
        stackpointer = stackpointer + 1;
        break;
    }
    case wrchr:{
        char chr_result = (char)(stack[stackpointer - 1]);
        printf("%c \n", chr_result);
        break;
    }
       
    default:
        break;
    }

    return 0;
}

int run(unsigned int prog[]) {
    unsigned int instruction = 0;
    int opcode = 1;
    
    while (opcode != halt)
    {
        instruction = prog[pc];
        opcode = instruction >> 24;
        pc = pc + 1;
        execute(instruction);

    }
    
    return 0;
}

int choice(char * prog) {
    if(strcmp(prog, "--prog1") == 0) {
        printf(" 000:    pushc   3 \n 001:    pushc   4 \n 002:    add \n 003:    pushc   10 \n 004:    pushc   6 \n 005:    sub \n 006:    mul \n 007:    wrint \n 008:    pushc   10 \n 009:    wrchr \n 010:    halt \n");
        run(prog1);
    } else 
    if(strcmp(prog, "--prog2") == 0) {
        printf(" 000:    pushc   -2 \n 001:    rdint \n 002:    mul \n 003:    pushc   3 \n 004:    add \n 005:    wrint \n 006:    pushc   10 \n 007:    wrchr \n 008:    halt \n");
        run(prog2);
    } else 
    if(strcmp(prog, "--prog3") == 0) {
        printf(" 000:    rdchr \n 001:    wrint \n 002:    pushc   10 \n 003:    wrchr \n 004:    halt \n");
        run(prog3);
    } else {
        printf("unknown expression");
    }
    return 0;
}


int test00(int argc, char * argv[]) {
    for(int line = 2; line < argc; line++) {
        //print help-Menu
        if(strcmp(argv[line], "--help") == 0) {
            printf("usage: %s [Option] [Option] ...\n", argv[0]);
            printf("  --version       show version and exit\n");
            printf("  --help          show this help and exit\n");
            return 0;
        } else if(strcmp(argv[line], "--version") == 0) {
            //print Version mit Compile-Datum
            printf("Ninja Virtual Machine version %d (compiled %s, %s)\n", version, __DATE__, __TIME__);
            return 0;
        } else {
            //Reagiert auf ungültige Kommandos
            printf("unknown command line argument '%s', try './njvm --help'", argv[line]);
            return 0;
        }
    }
    return 0; 
}

int main(int argc, char * argv[]) {
    ///home/tests/test19/test19.bin
    printf("Ninja Virtual Machine started\n");
    for(int line = 1; line < argc; line++) {
        if(line == 1 && strcmp(argv[line], "/home/tests/test00/test00.bin") == 0) {
            test00(argc, argv);
        } else {
            choice(argv[line]);
        }
    }
    //printf("%x \n", -8388608);
    printf("Ninja Virtual Machine stopped\n"); 
    return 0;
}