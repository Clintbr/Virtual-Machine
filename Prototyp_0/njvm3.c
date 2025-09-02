#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
// ninja instuctions
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
#define asf 13
#define rsf 14
#define pushl 15
#define popl 16
#define eq 17
#define ne 18
#define lt 19
#define le 20
#define gt 21
#define ge 22
#define jmp 23
#define brf 24
#define brt 25
// ende ninja instructions
// extends for negative values
#define sign_extend(i) ((i) & 0x00800000 ? (i) | 0xFF000000 : (i))
// arrays with instructions as Strings
char *commandos[] = {"halt", "puschc", "add", "sub", "mul", "div", "mod", "rdint", "wrint", "rdchr", "wrchr", "puschg", "popg", "asf", "rsf", "pushl", "popl", "eq", "ne", "lt", "le", "gt", "ge", "jmp", "brf", "brt"};
int version = 4;
int limit = 0;
int breakpointer = 0;
bool isset = false;
int pc = 0;
int stackpointer = 0;
int fp = 0;
unsigned int stack[10];
FILE *file;
char *path;
int infos[4];
unsigned int *prog_memory = NULL;
unsigned int *SDA = NULL;


int open(void) {
    pc = stackpointer = fp = 0;
    file = fopen(path, "r");
    if (file == NULL)
    {
        printf("Error: cannot open code file '%s' \n", path);
        exit(99);
    }
    fread(&infos[0], sizeof(int), 4, file);
    int prog[4 + infos[2]];
    // memory for the instruction reserved
    size_t num_values = fread(&prog[0], sizeof(int), infos[2], file);
    // memory for the instruction reserved
    prog_memory = (unsigned int *)malloc(infos[2] * sizeof(unsigned int));
    // memory for global variable
    SDA = (unsigned int *)malloc(infos[3] * sizeof(unsigned int));
    for (int i = 0; i < num_values; i++)
    {
        prog_memory[i] = prog[i];
    }
    if (version != infos[1])
    {
        printf("Error: incompatible version (expected %d, got %d)\n", version, infos[1]);
        exit(99);
    }

    limit = infos[2];
    return 0;
}

int close(void) {
    fclose(file);
    free(prog_memory);
    free(SDA);
    return 0;
}

int execute(unsigned int instruction)
{
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
        result = stack[stackpointer - 2] + stack[stackpointer - 1];
        stack[stackpointer - 2] = result;
        stackpointer = stackpointer - 1;
        break;
    case sub:
        result = stack[stackpointer - 2] - stack[stackpointer - 1];
        stack[stackpointer - 2] = result;
        stackpointer = stackpointer - 1;
        break;
    case mul:
        result = stack[stackpointer - 2] * stack[stackpointer - 1];
        stack[stackpointer - 2] = result;
        stackpointer = stackpointer - 1;
        break;
    case div:
        if (stack[stackpointer - 1] == 0)
        {
            printf("cannot divide in 0 \n");
            break;
        }
        result = stack[stackpointer - 2] / stack[stackpointer - 1];
        stack[stackpointer - 2] = result;
        stackpointer = stackpointer - 1;
        break;
    case mod:
        if (stack[stackpointer - 1] == 0)
        {
            printf("cannot divide in 0 \n");
            break;
        }
        result = stack[stackpointer - 2] % stack[stackpointer - 1];
        stack[stackpointer - 2] = result;
        stackpointer = stackpointer - 1;
        break;
    case rdint:
        scanf("%d", &result);
        stack[stackpointer] = result;
        stackpointer = stackpointer + 1;
        break;
    case wrint:
        result = stack[stackpointer - 1];
        printf("%d", (signed int)sign_extend(result));
        break;
    case rdchr:
    {
        char chr_result = ' ';
        scanf("%c", &chr_result);
        result = (unsigned int)chr_result;
        stack[stackpointer] = result;
        stackpointer = stackpointer + 1;
        break;
    }
    case wrchr:
    {
        char chr_result = (char)(stack[stackpointer - 1]);
        printf("%c", chr_result);
        break;
    }
    case pushg:
    {
        result = SDA[value];
        stack[stackpointer] = result;
        stackpointer = stackpointer + 1;
        break;
    }
    case popg:
    {
        result = stack[stackpointer - 1];
        stackpointer = stackpointer - 1;
        SDA[value] = result;
        break;
    }
    case asf:
    {
        value = sign_extend(value);
        stack[stackpointer] = fp;
        stackpointer = stackpointer + 1;
        fp = stackpointer;
        stackpointer = fp + value;
        break;
    }
    case rsf:
    {
        stackpointer = fp;
        fp = stackpointer - 1;
        stackpointer = stackpointer - 1;
        break;
    }
    case pushl:
    {
        int lvalue = stack[fp + value];
        lvalue = sign_extend(lvalue);
        stack[stackpointer] = lvalue;
        stackpointer = stackpointer + 1;
        break;
    }
    case popl:
    {
        int lvalue = stack[stackpointer - 1];
        stackpointer = stackpointer - 1;
        stack[fp + value] = lvalue;
        break;
    }
    case eq:
    {
        bool equals = stack[stackpointer - 1] == stack[stackpointer - 2];
        stack[stackpointer - 2] = equals ? 1 : 0;
        stackpointer = stackpointer - 1;
        break;
    }
    case ne:
    {
        bool equals = stack[stackpointer - 1] == stack[stackpointer - 2];
        stack[stackpointer - 2] = equals ? 0 : 1;
        stackpointer = stackpointer - 1;
        break;
    }
    case lt:
    {
        bool lower = stack[stackpointer - 1] > stack[stackpointer - 2];
        stack[stackpointer - 2] = lower ? 1 : 0;
        stackpointer = stackpointer - 1;
        break;
    }
    case le:
    {
        bool lower = stack[stackpointer - 1] >= stack[stackpointer - 2];
        stack[stackpointer - 2] = lower ? 1 : 0;
        stackpointer = stackpointer - 1;
        break;
    }
    case gt:
    {
        bool greater = stack[stackpointer - 1] < stack[stackpointer - 2];
        stack[stackpointer - 2] = greater ? 1 : 0;
        stackpointer = stackpointer - 1;
        break;
    }
    case ge:
    {
        bool greater = stack[stackpointer - 1] <= stack[stackpointer - 2];
        stack[stackpointer - 2] = greater ? 1 : 0;
        stackpointer = stackpointer - 1;
        break;
    }
    case jmp:
    {
        pc = value;
        break;
    }
    case brf:
    {
        if (stack[stackpointer - 1] == 0)
        pc = value;
        stackpointer = stackpointer - 1;
        break;
    }
    case brt:
    {
        if (stack[stackpointer - 1] == 1)
        pc = value;
        stackpointer = stackpointer - 1;
        break;
    }
    default:
        break;
    }

    return pc;
}

int run(char *path)
{
    unsigned int instruction = 0;
    int opcode = 1;
    close();
    open();
    // Nja PROGRAMM in menschenlesbarer Form
    /*
        Aufgelistet:    |000 ...
                        |001 ...
                        | ...
    */
    // end of instructionlisting

    // variable reset
    pc = 0;
    while (opcode != halt)
    {
        instruction = prog_memory[pc];
        opcode = instruction >> 24;
        pc = pc + 1;
        execute(instruction);
    }
    close();
    return 0;
}

int step(char * path) {
    unsigned int instruction = 0;
    int opcode = 1;

    instruction = prog_memory[pc];
    pc = pc + 1;
    pc = execute(instruction);
    instruction = prog_memory[pc];
    opcode = instruction >> 24;
    signed int value = instruction & 0x00FFFFFF;
    if ((opcode == 1) | (opcode == 11) | (opcode == 12) | (opcode == 13) | (opcode == 15) | (opcode == 16) | (opcode == 23) | (opcode == 24) | (opcode == 25))
    {
        printf("%04d:   %-6s    %d\n", pc, commandos[opcode], sign_extend(value));
    }
    else
    {
        printf("%04d:   %-6s    \n", pc, commandos[opcode]);
    }
    return 0;
}

int list(void) {
    unsigned int instruction = 0;
    int opcode = 1;
    // Nja PROGRAMM in menschenlesbarer Form
    /*
        Aufgelistet:    |0000 ...
                        |0001 ...
                        | ...
    */
    int remember = pc;
    pc = 0;
    for (int i = 0; i < infos[2]; i++)
    {
        instruction = prog_memory[pc];
        opcode = instruction >> 24;
        pc = pc + 1;
        signed int value = instruction & 0x00FFFFFF;
        if ((opcode == 1) | (opcode == 11) | (opcode == 12) | (opcode == 13) | (opcode == 15) | (opcode == 16) | (opcode == 23) | (opcode == 24) | (opcode == 25))
        {
            printf("%04d:   %-6s    %d\n", i, commandos[opcode], sign_extend(value));
        }
        else
        {
            printf("%04d:   %-6s    \n", i, commandos[opcode]);
        }
    }
    printf("        --- end of code ---\n");
    pc = remember;
    //close();
    return 0;
}

int data(void) {
    for(int i = 0; i < infos[3]; i++) {
        printf("data[%04d]:     %d\n", i, SDA[i]);
    }
    printf("        --- end of data ---\n");
    return 0;
}

int stacks(void) {
    for(int i = stackpointer; i >= 0; i--) {
        if((i == stackpointer) & (i == fp)) {
            printf("sp, fp  --->    %04d:   xxxx\n", i);
        } 
        else if(i == stackpointer) {
            printf("sp      --->    %04d:   xxxx\n", i);
        }
        else if(i == fp) {
            printf("fp      --->    %04d:   %d\n", i, stack[fp]);
        }
        else {
            printf("                %04d:   %d\n", i, stack[i]);
        }
    }
    printf("                --- bottom of stack ---\n");
    return 0;
} 

int inspect(void) {
    printf("DEBUG [inspect]: stack, data?\n");
    char command[10000];
    fgets(command, 10000, stdin);
    if(strcmp(command, "data\n") == 0){
        data();
        return 0;
    } else
    if(strcmp(command, "stack\n") == 0){
        stacks();
        return 0;
    } else {
        return 0;
    }

    return 0;
}

int setting(int result) {
    if((result == 1) & ((breakpointer >= 0) | (breakpointer < -1))) {
       printf("DEBUG [breakpoint]: now set at %d\n", breakpointer); 
       isset = true;
    } else
    if((result == 1) & (breakpointer == -1)) {
        printf("DEBUG [breakpoint]: now cleared\n"); 
        isset = false;
    } else {
       return 0; 
    }
    return 0;
}

int breakpoint(void) {
    if(isset == false) {
        printf("DEBUG [breakpoint]: cleared\n");
    } else {
        printf("DEBUG [breakpoint]: set at %d\n", breakpointer);
    }
    printf("DEBUG [breakpoint]: address to set, -1 to clear, <ret> for no change?\n");
    int result = scanf("%d", &breakpointer);
    setting(result);
    return 0;
}

int debuging(char commando[])
{
    if (strcmp(commando, "continue") == 0)
    {
        printf("DEBUG: inspect, list, breakpoint, step, run, quit?\n");
    }
    else if (strcmp(commando, "run\n") == 0)
    {
        run(path);
        return 0;
    }
    else if (strcmp(commando, "inspect\n") == 0)
    {
        inspect();
        unsigned int instruction = 0;
        int opcode = 1;
        instruction = prog_memory[pc];
        opcode = instruction >> 24;
        signed int value = instruction & 0x00FFFFFF;
        if ((opcode == 1) | (opcode == 11) | (opcode == 12) | (opcode == 13) | (opcode == 15) | (opcode == 16) | (opcode == 23) | (opcode == 24) | (opcode == 25))
        {
            printf("%04d:   %-6s    %d\n", 0, commandos[opcode], sign_extend(value));
        }
        else
        {
            printf("%04d:   %-6s    \n", 0, commandos[opcode]);
        }
        printf("DEBUG: inspect, list, breakpoint, step, run, quit?\n");
    }
    else if (strcmp(commando, "step\n") == 0)
    {
        if(pc == limit - 1) return 0;
        step(path);
        printf("DEBUG: inspect, list, breakpoint, step, run, quit?\n");
    }
    else if (strcmp(commando, "quit\n") == 0)
    {
        return 0;
    }
    else if (strcmp(commando, "list\n") == 0)
    {   
        list();
        unsigned int instruction = 0;
        int opcode = 1;
        instruction = prog_memory[pc];
        opcode = instruction >> 24;
        signed int value = instruction & 0x00FFFFFF;
        if ((opcode == 1) | (opcode == 11) | (opcode == 12) | (opcode == 13) | (opcode == 15) | (opcode == 16) | (opcode == 23) | (opcode == 24) | (opcode == 25))
        {
            printf("%04d:   %-6s    %d\n", 0, commandos[opcode], sign_extend(value));
        }
        else
        {
            printf("%04d:   %-6s    \n", 0, commandos[opcode]);
        }
        printf("DEBUG: inspect, list, breakpoint, step, run, quit?\n");
    }
    else if (strcmp(commando, "breakpoint\n") == 0)
    {   
        breakpoint();
        unsigned int instruction = 0;
        int opcode = 1;
        instruction = prog_memory[pc];
        opcode = instruction >> 24;
        signed int value = instruction & 0x00FFFFFF;
        if ((opcode == 1) | (opcode == 11) | (opcode == 12) | (opcode == 13) | (opcode == 15) | (opcode == 16) | (opcode == 23) | (opcode == 24) | (opcode == 25))
        {
            printf("%04d:   %-6s    %d\n", 0, commandos[opcode], sign_extend(value));
        }
        else
        {
            printf("%04d:   %-6s    \n", 0, commandos[opcode]);
        }
    } else {
        printf("DEBUG: inspect, list, breakpoint, step, run, quit?\n");
    }
    char command[10000];
    fgets(command, 10000, stdin);
    debuging(command);
    return 0;
}

int debugmode(char *path)
{
    open();
    unsigned int instruction = 0;
    int opcode = 1;
    printf("DEBUG: file '%s' loaded (code size = %d, data size = %d) \n", path, infos[2], infos[3]);
    printf("Ninja Virtual Machine started \n");
    instruction = prog_memory[pc];
    opcode = instruction >> 24;
    signed int value = instruction & 0x00FFFFFF;
    if ((opcode == 1) | (opcode == 11) | (opcode == 12) | (opcode == 13) | (opcode == 15) | (opcode == 16) | (opcode == 23) | (opcode == 24) | (opcode == 25))
    {
        printf("%04d:   %-6s    %d\n", 0, commandos[opcode], sign_extend(value));
    }
    else
    {
        printf("%04d:   %-6s    \n", 0, commandos[opcode]);
    }

    debuging("continue");
    printf("Ninja Virtual Machine stopped\n");
    return 0;
    
}

int debug(int argc, char *argv[])
{
    for (int line = 1; line < argc; line++)
    {
        debugmode(path);
        return 0;
    }
    close();
    return 0;
}

int test00(int line, char *argv[])
{
    // print help-Menu
    if (strcmp(argv[line], "--help") == 0)
    {
        printf("usage: %s [options] <code file> \n", argv[0]);
        printf("Options: \n");
        printf("  --debug         start virtual machine in debug mode\n");
        printf("  --version       show version and exit\n");
        printf("  --help          show this help and exit\n");
        return 0;
    }
    else if (strcmp(argv[line], "--version") == 0)
    {
        // print Version mit Compile-Datum
        printf("Ninja Virtual Machine version %d (compiled %s, %s)\n", version, __DATE__, __TIME__);
        return 0;
    }
    else if (strcmp(argv[line], "--debug") == 0)
    {
        // debug();
        return 0;
    }
    else
    {
        // Reagiert auf ungültige Kommandos
        printf("unknown command line argument '%s', try './njvm --help'", argv[line]);
        exit(99);
        return 0;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int codenumber = 0;
    for (int line = 1; line < argc; line++)
    {
        if (*argv[line] != '-') {
           codenumber++; 
           path = argv[line];
        }
        codenumber = codenumber;
        
    }
    if (argc == 1)
    {
        printf("Error: no code file specified");
        exit(99);
        return 0;
    } else {

    }

    for (int line = 1; line < argc; line++)
    {
        if (((*argv[line] == '-') & (strcmp(argv[line], "--debug") != 0)))
        {
            test00(line, argv);
            return 0;
        }
        else {

        }
    }
    for (int line = 1; line < argc; line++)
    {
        if (codenumber > 1) {
            printf("Error: more than one code file specified");
            exit(99);
            return 0;
        } else
        if (strcmp(argv[line], "--debug") == 0)
        {
            printf("Ninja Virtual Machine started \n");
            debug(argc, argv);
            return 0;
        }
        else {
            codenumber = codenumber;
        }
    }
    if(codenumber == 1) {
        printf("Ninja Virtual Machine started \n");
        open();
        run(path);
        printf("Ninja Virtual Machine stopped\n");
        return 0;
    } else if (codenumber > 1) {
        printf("Error: more than one code file specified");
        exit(99);
        return 0;
    }
    
    return 0;
}