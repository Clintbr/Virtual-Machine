#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include <stdlib.h>
#include <time.h>
//ninja instuctions
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
#define pushg 11
#define popg 12
#define asf  13
#define rsf  14
#define pushl 15
#define popl  16
// ende ninja instructions
// extends for negative values
#define sign_extend(i) ((i) & 0x00800000 ? (i) | 0xFF000000 : (i))
// arrays with instructions as Strings
char * commandos[] = {"halt","puschc", "add", "sub", "mul", "div", "mod", "rdint", "wrint", "rdchr", "wrchr", "puschg", "popg","asf", "rsf", "pushll", "popl"};
int version = 2;
int pc = 0;
int stackpointer = 0;
int fp = 0;
unsigned int stack[10];
FILE * file;
char * path;
int prog[10000];
int infos[4];
unsigned int *prog_memory = NULL;
unsigned int *DSA = NULL;

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
            printf("%d",(signed int)sign_extend(result));
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
            printf("%c", chr_result);
            break;
        }
        case pushg:{
            result = DSA[value];
            stack[stackpointer] = result;
            stackpointer = stackpointer + 1;
            break;
        }
        case popg:{
            result = stack[stackpointer - 1];
            stackpointer = stackpointer - 1;
            DSA[value] = result;
            break;
        }
        case asf: {
            value = sign_extend(value);
            stack[stackpointer] = fp;
            stackpointer = stackpointer + 1;
            fp = stackpointer;
            stackpointer = fp + value;
            break;
        }
        case rsf: {
            stackpointer = fp;
            fp = stackpointer - 1;
            stackpointer = stackpointer - 1;
            break;
        }
        case pushl: {
            int lvalue = stack[fp + value];
            lvalue = sign_extend(lvalue);
            stack[stackpointer] = lvalue;
            stackpointer = stackpointer + 1;
            break;
        }
        case popl: {
            int lvalue = stack[stackpointer - 1];
            stackpointer = stackpointer - 1;
            stack[fp + value] = lvalue;
            break;
        }
        default:
            break;
    }

    return 0;
}

int run(char * path) {
    unsigned int instruction = 0;
    int opcode = 1;
    file = fopen(path,"r");
    if(file == NULL) {
        printf("Error: cannot open code file '%s' \n", path);
        exit(99);
    }

    size_t num_values = fread(&prog[0], sizeof(int), 200000, file);
    prog_memory = (unsigned int *)malloc(num_values * sizeof(unsigned int));
    for(int i = 0; i < 4; i++) {
        infos[i] = prog[i];
        printf("%d \n", infos[i]);
    }
    DSA = (unsigned int *)malloc(infos[3] * sizeof(unsigned int));
    for(int i = 4; i < num_values; i++) {
        prog_memory[i - 4] = prog[i];
    }
    if(version != infos[1]) {
        printf("Error: incompatible version (expected %d, got %d)\n", version, infos[1]);
        exit(99);
    }
    // Nja PROGRAMM in menschenlesbarer Form
    /*
        Aufgelistet:    |000 ...
                        |001 ...
                        | ...
    */
    printf("Ninja Virtual Machine started \n");
    for (int i = 0; i < infos[2]; i++) {
        instruction = prog_memory[pc];
        opcode = instruction >> 24;
        pc = pc + 1;
        signed int value = instruction & 0x00FFFFFF;
        if((opcode == 1 )| (opcode == 11) | (opcode == 12) | (opcode == 13) | (opcode == 15) | (opcode == 16)) {
            printf("%03d:   %-6s    %d\n", i, commandos[opcode], sign_extend(value));
        } else {
            printf("%03d:   %-6s    \n", i, commandos[opcode]);
        }
    }
    // end of instructionlisting

    // variable reset
    opcode = 1;
    pc = 0;
    while (opcode != halt)
    {
        instruction = prog_memory[pc];
        opcode = instruction >> 24;
        pc = pc + 1;
        execute(instruction);

    }
    printf("Ninja Virtual Machine stopped\n"); 
    fclose(file);
    free(prog_memory);
    return 0;
}

int test00(int argc, char * argv[]) {
    for(int line = 1; line < 2; line++) {
        //print help-Menu
        if(strcmp(argv[line], "--help") == 0) {
            printf("usage: %s [options] <code file> \n", argv[0]);
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
            exit(99);
            return 0;
            
        }
    }
    return 0; 
}

int main(int argc, char * argv[]) { 
    ///home/tests/test19/test19.bin
    if(argc == 1) {
        printf("Error: no code file specified");
        exit(99);
    } else
    if((argc > 1) & (*argv[1] == '-')) {
       test00(argc, argv);
       return 0; 
    } else {
       //printf("Ninja Virtual Machine started\n");
        for(int line = 1; line < argc; line++) {
            path = argv[line];
            run(path);
        }
        //printf("Ninja Virtual Machine stopped\n"); 
        return 0; 
    }
    
}