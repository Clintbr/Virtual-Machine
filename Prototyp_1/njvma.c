#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
//#include "../bigint/build/include/bigint.h"
//#include "../bigint/build/include/support.h"
//+++++++++++++++++++++++++
//Attach Bibliothek Bigint
//+++++++++++++++++++++++++
/*
 * support.c -- support functions for big integer library
 */
 
 typedef struct {
       unsigned int size;                /* byte count of payload data */
         unsigned char data[1];  /* payload data, size as needed */
 } *ObjRef;
 

 /**************************************************************/


void dump(char *prefix, ObjRef x, char *suffix) {
    if (prefix != NULL && *prefix != '\0') {
      printf("%s", prefix);
    }
    bigDump(stdout, x);
    if (suffix != NULL && *suffix != '\0') {
      printf("%s", suffix);
    }
  }
  
  
  /**************************************************************/
  
 
 /*
  * This routine is called in case a fatal error has occurred.
  * It should print the error message and terminate the program.
*/
 void fatalError(char *msg) {
   printf("Fatal error: %s\n", msg);
   exit(1);
 }

 /*
  * This function is called whenever a new primitive object with
  * a certain amount of internal memory is needed. It should return
  * an object reference to a regular object, which contains a freely
  * usable memory area of at least the requested size (measured in
  * bytes). The memory area need not be initialized in any way.
  *
  * Note that this function may move all objects in memory at will
  * (due to, e.g., garbage collection), as long as the pointers in
  * the global "bip" structure point to the correct objects when
  * the function returns.
*/

 void * newPrimObject(int dataSize) {
   ObjRef bigObjRef;
 
   bigObjRef = malloc(sizeof(unsigned int) +
                   dataSize * sizeof(unsigned char));
   if (bigObjRef == NULL) {
     fatalError("newPrimObject() got no memory");
   }
   bigObjRef->size = dataSize;
   return bigObjRef;
 }

 /*void * setPrimObject(void * obj) {
    ObjRef objRef = ((ObjRef) (obj));
    *(ObjRef*)objRef->data = obj;
    return objRef;
 }
 
 void * getPrimObjectDataPointer(void * obj){
     ObjRef oo = ((ObjRef) (obj));
     return *(ObjRef*)oo->data;
 }

//+++++++++++++++++++++++++
//
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
#define call 26
#define ret  27
#define drop 28
#define pushr 29
#define popr   30
#define dup    31
#define STACK_SIZE 10000

// ende ninja instructions
// extends for negative values
#define sign_extend(i) ((i) & 0x00800000 ? (i) | 0xFF000000 : (i))
// arrays with instructions as Strings
char *commandos[] = {"halt", "puschc", "add", "sub", "mul", "div", "mod", "rdint", "wrint", "rdchr", "wrchr", "puschg", "popg", "asf", "rsf", "pushl", "popl", "eq", "ne", "lt", "le", "gt", "ge", "jmp", "brf", "brt", "call", "ret", "drop", "pushr", "popr", "dup"};
// return value register
//int rvr[1];
// caller
int ra = 0; // return adress
int statecode = 1;
//endCaller
int version = 6;
// debugger
int limit = 0;
int breakpointer = 0;
bool isset = false;
// end debugger

int fp = 0;// localvariable
int reservedspace = 0; //Number of allocated Cells
int reservedtime = 0;// How many time were memoryspace allocated ?
// programmcounter
int pc = 0;
int stackpointer = 0;

//unsigned int stack[STACK_SIZE];
FILE *file;
char *path;
int infos[4];
//unsigned int *prog_memory = NULL;
//unsigned int *SDA = NULL;
int u = 0;

// Heap Verwaltung(Äderungen)
//neuen Datentyp anlegen
typedef struct {
    int value;
} IntObject;

//neue Daten erstellen
IntObject* create_int_object(int val) {
    IntObject* obj = (IntObject*)malloc(sizeof(IntObject));
    obj->value = sign_extend(val);
    return obj;
}

// Payload zurückgeben
int get_int_value(IntObject* obj) {
    return obj->value;
}

//new Stack version with stackslots
typedef struct {
    bool isObjref;
    union {
        ObjRef objref; //isObjRef = True
        int number; //isObjRef = false
    } u;
} Stackslot;

Stackslot stack[STACK_SIZE];
/*
ObjRef* SDA = NULL; // in `open()` malloc für `infos[3]`
ObjRef* rvr[1];


int open(void) {
    pc = stackpointer = fp = 0;
    file = fopen(path, "r");
    if (file == NULL)
    {
        printf("Error: cannot open code file '%s'\n", path);
        exit(99);
    }
    fread(&infos[0], sizeof(int), 4, file);
    int prog[4 + infos[2]];
    // memory for the instruction reserved
    size_t num_values = fread(&prog[0], sizeof(int), infos[2], file);
    if (num_values != infos[2]) {
        fprintf(stderr, "Fehler: Datei enthält weniger Befehle als erwartet.\n");
        exit(99);
    }
    // memory for the instruction reserved
    prog_memory = (unsigned int *)malloc(infos[2] * sizeof(unsigned int));
    // memory for global variable
    SDA = (ObjRef*)malloc(infos[3] * sizeof(unsigned int));
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
    ObjRef result;
    //printf("\nOpcode %d", opcode);
    signed int value = instruction & 0x00FFFFFF;
    value = sign_extend(value);
    switch (opcode)
    {
    case halt:
        break;
    case puschc:
    {
        Stackslot stackslot;
        typedef struct {
            bool isObjref;
            union {
                ObjRef objref; //isObjRef = True
                int number; //isObjRef = false
            } u;
        } Stackslot;

        value = sign_extend(value);
        bigFromInt(value);
        result = newPrimObject(sizeof(bip.op1));
        result = bip.op1;
        stack[stackpointer].isObjref = true;
        stack[stackpointer].u.objref = bip.res;
        stackpointer = stackpointer + 1;
        break;
    }
    case add:
    {
        BigObjRef a = *stack[--stackpointer];
        BigObjRef b = *stack[--stackpointer];
        bip.op1 = a;
        bip.op2 = b;
        bigAdd();
        result = bip.res;
        stack[stackpointer++] = &result;
        break;
    }
    case sub:
    {
        if(stackpointer <= 1) {
            printf("AAA");
            exit(99);
        }
        BigObjRef b = *stack[--stackpointer];
        BigObjRef a = *stack[--stackpointer];
        bip.op1 = a;
        bip.op2 = b;
        bigSub();
        result = bip.res;
        stack[stackpointer++] = &result;
        break;
    }
    case mul:
    {
        if(stackpointer <= 1) {
            printf("AAA");
            exit(99);
        }
        BigObjRef a = *stack[--stackpointer];
        BigObjRef b = *stack[--stackpointer];
        bip.op1 = a;
        bip.op2 = b;
        bigMul();
        result = bip.res;
        stack[stackpointer++] = &result;
        break;
    }
    case div:
    {
        if(stackpointer <= 1) {
            printf("AAA");
            exit(99);
        }
        BigObjRef b = *stack[--stackpointer];
        BigObjRef a = *stack[--stackpointer];
        bip.op1 = a;
        bip.op2 = b;
        bigDiv();
        result = bip.res;
        stack[stackpointer++] = &result;
        break;
    }
    case mod:
    {
        if(stackpointer <= 1) {
            printf("AAA");
            exit(99);
        }
        BigObjRef b = *stack[--stackpointer];
        BigObjRef a = *stack[--stackpointer];
        bip.op1 = a;
        bip.op2 = b;
        bigDiv();
        result = bip.rem;
        stack[stackpointer++] = &result;
        break;
    }
    case rdint:
    {
        fflush(stdout);
        bigRead(stdin);
        result = bip.res;
        stack[stackpointer++] = &result;
        break;
    }
    case wrint:
    {
        result = *stack[--stackpointer];
        bip.op1 = result;
        bigPrint(stdout);
        stackpointer = stackpointer - 1;
        break;
    }
        */
    /**
     * rdchr and wrchr don't use the bibliothek due to
     * the fact that they don#t need uge values
     * (I schould verify it while completing the exercise) 
     */
    /*
    case rdchr:
    {
        char chr_result = ' ';
        scanf("%c", &chr_result);
        int res = (unsigned int)chr_result;
        IntObject* result = create_int_object(res);
        stack[stackpointer] = result;
        stackpointer = stackpointer + 1;
        break;
    }
    case wrchr:
    {
        char chr_result = (char)get_int_value((stack[stackpointer - 1]));
        printf("%c", chr_result);
        stackpointer = stackpointer - 1;
        break;
    }

    case pushg:
    {
        value = sign_extend(value);
        IntObject* result = SDA[value];
        stack[stackpointer++] = result;
        break;
    }
    case popg:
    {
        value = sign_extend(value);
        IntObject* result = stack[stackpointer - 1];
        stackpointer = stackpointer - 1;
        SDA[value] = result;
        break;
    }
    case asf:
    {
        value = sign_extend(value);
        if(value < 0) {
            printf("AAA");
            exit(99);
        }
        stack[stackpointer] = create_int_object(fp);
        stackpointer = stackpointer + 1;
        fp = stackpointer;
        stackpointer = fp + value;
        reservedtime++;
        break;
    }
    case rsf:
    {
        if(reservedtime < 0) {
            printf("BBB");
            exit(99);
        }
        stackpointer = fp;
        fp = get_int_value(stack[stackpointer - 1]);
        stackpointer = stackpointer - 1;
        reservedtime--;
        break;
    }
    case pushl:
    {
        value = sign_extend(value);
        if((fp + value) < 0) {
            printf("CCC");
            exit(99);
        }
        stack[stackpointer] = stack[fp + value];
        stackpointer = stackpointer + 1;
        break;
    }
    case popl:
    {
        value = sign_extend(value);
        if((fp + value) < 0) {
            printf("DDD");
            exit(99);
        }
        stack[fp + value] = stack[stackpointer - 1];
        stackpointer = stackpointer - 1;
        break;
    }
    case eq:
    {
        int a1 = get_int_value(stack[stackpointer - 2]);
        int a2 = get_int_value(stack[stackpointer - 1]);
        bigFromInt(a1);
        bip.op1 = bip.res;
        bigFromInt(a2);
        bip.op2 = bip.res;
        result = bigCmp();
        bool equals = (result == 0);
        stack[stackpointer - 2] = equals ? create_int_object(1) : create_int_object(0);
        stackpointer = stackpointer - 1;
        break;
    }
    case ne:
    {
        int a1 = get_int_value(stack[stackpointer - 2]);
        int a2 = get_int_value(stack[stackpointer - 1]);
        bigFromInt(a1);
        bip.op1 = bip.res;
        bigFromInt(a2);
        bip.op2 = bip.res;
        result = bigCmp();
        bool equals = (result == 0);
        stack[stackpointer - 2] = equals ? create_int_object(0) : create_int_object(1);
        stackpointer = stackpointer - 1;
        break;
    }
    case lt:
    {
        int a1 = get_int_value(stack[stackpointer - 2]);
        int a2 = get_int_value(stack[stackpointer - 1]);
        bigFromInt(a1);
        bip.op1 = bip.res;
        bigFromInt(a2);
        bip.op2 = bip.res;
        result = bigCmp();
        bool lower = (result < 0);
        stack[stackpointer - 2] = lower ? create_int_object(1) : create_int_object(0);
        stackpointer = stackpointer - 1;
        break;
    }
    case le:
    {
        int a1 = get_int_value(stack[stackpointer - 2]);
        int a2 = get_int_value(stack[stackpointer - 1]);
        bigFromInt(a1);
        bip.op1 = bip.res;
        bigFromInt(a2);
        bip.op2 = bip.res;
        result = bigCmp();
        bool lower = (result <= 0);
        stack[stackpointer - 2] = lower ? create_int_object(1) : create_int_object(0);
        stackpointer = stackpointer - 1;
        break;
    }
    case gt:
    {
        int a1 = get_int_value(stack[stackpointer - 2]);
        int a2 = get_int_value(stack[stackpointer - 1]);
        bigFromInt(a1);
        bip.op1 = bip.res;
        bigFromInt(a2);
        bip.op2 = bip.res;
        result = bigCmp();
        bool lower = (result > 0);
        stack[stackpointer - 2] = lower ? create_int_object(1) : create_int_object(0);
        stackpointer = stackpointer - 1;
        break;
    }
    case ge:
    {
        int a1 = get_int_value(stack[stackpointer - 2]);
        int a2 = get_int_value(stack[stackpointer - 1]);
        bigFromInt(a1);
        bip.op1 = bip.res;
        bigFromInt(a2);
        bip.op2 = bip.res;
        result = bigCmp();
        bool lower = (result >= 0);
        stack[stackpointer - 2] = lower ? create_int_object(1) : create_int_object(0);
        stackpointer = stackpointer - 1;
        break;
    }
    case jmp:
    {
        value = sign_extend(value);
        pc = value;
        break;
    }
    case brf:
    {
        value = sign_extend(value);
        if (get_int_value(stack[stackpointer - 1]) == 0) pc = value;
        stackpointer = stackpointer - 1;
        break;
    }
    case brt:
    {
        value = sign_extend(value);
        if (get_int_value(stack[stackpointer - 1]) == 1) pc = value;
        stackpointer = stackpointer - 1;
        break;
    }
    case call:
    {
        value = sign_extend(value);
        if((value < 0) | (value > limit)) {
            printf("EEE");
            exit(99);
        }
        ra = pc;
        stack[stackpointer] = create_int_object(ra);
        stackpointer = stackpointer + 1;
        pc = value;
        break;
    }
    case ret:
    {
        pc = get_int_value(stack[stackpointer - 1]);
        stackpointer--;
        break;
    }
    case drop: 
    {
        value = sign_extend(value);
        if(value < 0) {
            printf("FFF");
            exit(99);
        }
        for(int i = 0; i < value; i++) {
            stackpointer--;
        }
        break;
    }
    case pushr:
    {
        IntObject* rv = rvr[0];// return value
        stack[stackpointer] = rv;
        stackpointer++;
        break;
    }
    case popr: 
    {
        if(stackpointer == 0) {
            printf("GGG");
            exit(99);
        }
        IntObject* rv = stack[stackpointer - 1];// return value
        rvr[0] = rv;
        stackpointer--;
        break;
    }
    case dup:
    {
        stack[stackpointer] = stack[stackpointer - 1];
        stackpointer++;
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
    statecode = opcode;
    signed int value = instruction & 0x00FFFFFF;
    value = sign_extend(value);

    if ((opcode == 1) | (opcode == 11) | (opcode == 12) | (opcode == 13) | (opcode == 15) | (opcode == 16) | (opcode == 23) | (opcode == 24) | (opcode == 25) | (opcode == 26) | (opcode == 28))
    {
        printf("%04d:   %s    %d\n", pc, commandos[opcode], sign_extend(value));
    }
    else
    {
        printf("%04d:   %s    \n", pc, commandos[opcode]);
    }
    return 0;
}

int list(void) {
    unsigned int instruction = 0;
    int opcode = 1;
    // Nja PROGRAMM in menschenlesbarer Form
    */
    /*
        Aufgelistet:    |0000 ...
                        |0001 ...
                        | ...
    */ 
 /*   int remember = pc;
    pc = 0;
    for (int i = 0; i < infos[2]; i++)
    {
        instruction = prog_memory[pc];
        opcode = instruction >> 24;
        pc = pc + 1;
        signed int value = instruction & 0x00FFFFFF;
        value = sign_extend(value);
        if ((opcode == 1) | (opcode == 11) | (opcode == 12) | (opcode == 13) | (opcode == 15) | (opcode == 16) | (opcode == 23) | (opcode == 24) | (opcode == 25) | (opcode == 26) | (opcode == 28))
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
        printf("data[%04d]:     (objref) %p\n", i, (void *)(SDA[i]));
    }
    printf("        --- end of data ---\n");
    return 0;
}
// Ajouts
void print_stack_entry(int index) {
    if (index < stackpointer) {
        printf("  %04d: OBJ @ %p => %d\n", index, (void*)stack[index], get_int_value(stack[index]));
    } else {
        printf("  %04d: RAW => %u\n", index, (unsigned int)(uintptr_t)stack[index]);
    }
}
int stacks(void) {
    for(int i = stackpointer; i >= 0; i--) {
        if((i == stackpointer) & (i == fp)) {
            printf("sp, fp  --->    %04d:   (xxxxxx) xxxxxx\n", i);
        } 
        else if(i == stackpointer) {
            printf("sp      --->    %04d:   (xxxxxx) xxxxxx\n", i);
        }
        else if(i == fp) {
            printf("fp      --->    %04d:   (objref) %p\n", i, (void *)(stack[fp]));
        }
        else {
            printf("                %04d:   (objref) %p\n", i, (void *)(stack[i]));
        }
    }
    printf("                --- bottom of stack ---\n");
    return 0;
}

int object(void) {
    void* addr;
    printf("object reference?");
    scanf("%p", &addr);
    IntObject* obj = (IntObject*)addr;
    printf("Object at %p: value = %d\n", addr, get_int_value(obj));
    return 0;
}

int inspect(void) {
    printf("DEBUG [inspect]: stack, data, object?\n");
    char command[10000];
    fgets(command, 10000, stdin);
    if(strcmp(command, "data\n") == 0){
        data();
        return 0;
    } else
    if(strcmp(command, "stack\n") == 0){
        stacks();
        return 0;
    } else
    if(strcmp(command, "object\n") == 0){
        object();
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
        value = sign_extend(value);
        if ((opcode == 1) | (opcode == 11) | (opcode == 12) | (opcode == 13) | (opcode == 15) | (opcode == 16) | (opcode == 23) | (opcode == 24) | (opcode == 25) | (opcode == 26) | (opcode == 28))
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
        if(statecode == halt) return 0;
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
        value = sign_extend(value);
        if ((opcode == 1) | (opcode == 11) | (opcode == 12) | (opcode == 13) | (opcode == 15) | (opcode == 16) | (opcode == 23) | (opcode == 24) | (opcode == 25) | (opcode == 26) | (opcode == 28))
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
        value = sign_extend(value);
        if ((opcode == 1) | (opcode == 11) | (opcode == 12) | (opcode == 13) | (opcode == 15) | (opcode == 16) | (opcode == 23) | (opcode == 24) | (opcode == 25) | (opcode == 26) | (opcode == 28))
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
    printf("DEBUG: file '%s' loaded (code size = %d, data size = %d)\n", path, infos[2], infos[3]);
    printf("Ninja Virtual Machine started\n");
    instruction = prog_memory[pc];
    opcode = instruction >> 24;
    signed int value = instruction & 0x00FFFFFF;
    value = sign_extend(value);
    if ((opcode == 1) | (opcode == 11) | (opcode == 12) | (opcode == 13) | (opcode == 15) | (opcode == 16) | (opcode == 23) | (opcode == 24) | (opcode == 25) | (opcode == 26) | (opcode == 28))
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
        printf("usage: %s [options] <code file>\n", argv[0]);
        printf("Options:\n");
        printf("  --debug         start virtual machine in debug mode\n");
        printf("  --version       show version and exit\n");
        printf("  --help          show this help and exit\n");
        return 0;
    }
    else if (strcmp(argv[line], "--version") == 0)
    {
        // print Version mit Compile-Datum
        printf("Ninja Virtual Machine version %d (compiled Sep 23 2015, 10:36:55)\n", version);
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
        printf("unknown command line argument '%s', try './njvm --help'\n", argv[line]);
        exit(99);
        return 0;
    }
    return 0;
}
*/
/*************
 * 
 * test
 */

 void test02(void) {
    int m[7];
    ObjRef n[7];
    int i, j;
    int res;
  
    m[0] = 0;
    bigFromInt(m[0]);
    n[0] = bip.res;
    printf("n[0] = %d\n", m[0]);
  
    m[1] = 100;
    bigFromInt(m[1]);
    n[1] = bip.res;
    printf("n[1] = %d\n", m[1]);
  
    m[2] = -100;
    bigFromInt(m[2]);
    n[2] = bip.res;
    printf("n[2] = %d\n", m[2]);
  
    m[3] = 101;
    bigFromInt(m[3]);
    n[3] = bip.res;
    printf("n[3] = %d\n", m[3]);
  
    m[4] = -101;
    bigFromInt(m[4]);
    n[4] = bip.res;
    printf("n[4] = %d\n", m[4]);
  
    m[5] = 12345678;
    bigFromInt(m[5]);
    n[5] = bip.res;
    printf("n[5] = %d\n", m[5]);
  
    m[6] = -12345678;
    bigFromInt(m[6]);
    n[6] = bip.res;
    printf("n[6] = %d\n", m[6]);
  
    for (i = 0; i < 7; i++) {
      for (j = 0; j < 7; j++) {
        printf("%12d ", m[i]);
        bip.op1 = n[i];
        bip.op2 = n[j];
        res = bigCmp();
        if (res < 0) {
          printf("<");
        } else
        if (res > 0) {
          printf(">");
        } else {
          printf("=");
        }
        printf(" %12d", m[j]);
        printf("\n");
      }
    }
  }
  
  //////////////////

  void test08(void) {
    int i;
  
    bigFromInt(1);
    for (i = 1; i < 100; i++) {
      bip.op1 = bip.res;
      bip.op2 = bip.res;
      bigAdd();
      printf("2 ^ %2d = ", i);
      dump("", bip.res, "\n");
    }
  }
  
  /**************************************************************/
  
  void test20(void) {
    printf("please enter a number: ");
    fflush(stdout);
    bigRead(stdin);
    dump("", bip.res, "\n");
  }
/******************************** */
void test21(void) {
    bigFromInt(0);
    dump("", bip.res, " = ");
    bip.op1 = bip.res;
    bigPrint(stdout);
    printf("\n");
  
    bigFromInt(1);
    dump("", bip.res, " = ");
    bip.op1 = bip.res;
    bigPrint(stdout);
    printf("\n");
  
    bigFromInt(-1);
    dump("", bip.res, " = ");
    bip.op1 = bip.res;
    bigPrint(stdout);
    printf("\n");
  
    bigFromInt(0x1234);
    dump("", bip.res, " = ");
    bip.op1 = bip.res;
    bigPrint(stdout);
    printf("\n");
  
    bigFromInt(-0x1234);
    dump("", bip.res, " = ");
    bip.op1 = bip.res;
    bigPrint(stdout);
    printf("\n");
  
    bigFromInt(0x186A0);
    dump("", bip.res, " = ");
    bip.op1 = bip.res;
    bigPrint(stdout);
    printf("\n");
  
    bigFromInt(-0x186A0);
    dump("", bip.res, " = ");
    bip.op1 = bip.res;
    bigPrint(stdout);
    printf("\n");
  
    bigFromInt(0x12345678);
    dump("", bip.res, " = ");
    bip.op1 = bip.res;
    bigPrint(stdout);
    printf("\n");
  
    bigFromInt(-0x12345678);
    dump("", bip.res, " = ");
    bip.op1 = bip.res;
    bigPrint(stdout);
    printf("\n");
  
    bigFromInt(987654321);
    bip.op1 = bip.res;
    bip.op2 = bip.res;
    bigMul();
    bip.op1 = bip.res;
    bip.op2 = bip.res;
    bigMul();
    dump("", bip.res, " =\n");
    bip.op1 = bip.res;
    bigPrint(stdout);
    printf("\n");
  }
 /************
  * 
  * endTest
  */

int main(int argc, char *argv[])
{
    /*
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
        printf("Error: no code file specified\n");
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
            printf("Error: more than one code file specified\n");
            exit(99);
            return 0;
        } else
        if (strcmp(argv[line], "--debug") == 0)
        {
            debug(argc, argv);
            return 0;
        }
        else {
            codenumber = codenumber;
        }
    }
    if(codenumber == 1) {
        printf("Ninja Virtual Machine started\n");
        open();
        run(path);
        printf("Ninja Virtual Machine stopped\n");
        return 0;
    } else if (codenumber > 1) {
        printf("Error: more than one code file specified\n");
        exit(99);
        return 0;
    }
*/
    //test02();
    printf(":::::::::::::::::::::::");
    test08();
    //printf(":::::::::::::::::::::::");
    //test21();
    //test
    //int a = 4;
    //bigFromInt(a);
    //ObjRef result = newPrimObject(sizeof(bip.op1));
    //result = bip.op1;
    //bip.res = getPrimObjectDataPointer(result);
    //stack[stackpointer].isObjref = true;
    //stack[stackpointer].u.objref = bip.res;
    //stackpointer = stackpointer + 1;
    //Stackslot b = stack[stackpointer - 1];
    //bip.op1 = b.u.objref;
    //bigPrint(stdout);
    //(printf("%d", a);
    return 0; 

}