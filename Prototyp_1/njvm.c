#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "../bigint/build/include/bigint.h"
#include "../bigint/build/include/support.h"
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
// instructions for Records and Arrays
#define new     32
#define getf    33
#define putf    34
#define newa    35
#define getfa   36
#define putfa   37
#define getsz   38
#define puschn  39
#define refeq   40
#define refne   41
// ende ninja instructions
#define STACK_SIZE 100000
/**
 * a) Primite object definition
 */
#define MSB (1 << (8 * sizeof(unsigned int) - 1)) // Most Significant Bit (für Compound-Objekte)
#define IS_PRIM(objRef) (((objRef)->size & MSB) == 0)
// set MSB
#define SET_MSB(size) (size | MSB)
/**
 * b) Number of objRef in an Object
 */
#define GET_SIZE(objRef) ((objRef)->size & ~MSB)
/**
 * c) Pointer over the first objRef in an Object
 */
#define GET_REFS(objRef) ((ObjRef *) (objRef)->data)
// extends for negative values
#define sign_extend(i) ((i) & 0x00800000 ? (i) | 0xFF000000 : (i))

/*
 * support.c -- support functions for big integer library
 */
 
typedef struct {
    unsigned int size;                /* byte count of payload data */
      unsigned char data[1];  /* payload data, size as needed */
} *ObjRef;

void fatalError(char *msg) {
    printf("Fatal error: %s\n", msg);
    exit(1);
}

int errorPrint(ObjRef object, int value) {
    if(object == NULL) {
        fatalError("Zugriff auf Nil-Wert verboten");
    } else
    if(value < 0 || value > GET_SIZE(object)) {
        fatalError("Out_Of_Bound_Exception");
    }
    return 0;
}

void * getPrimObjectDataPointer(void * obj){
    ObjRef oo = ((ObjRef)  (obj));
    return oo->data;
}
//new Stack version with stackslots
typedef struct {
    bool isObjref;
    union {
        ObjRef objRef; //isObjRef = True
        int number; //isObjRef = false
    } u;
} Stackslot;


// arrays with instructions as Strings
char *commandos[] = {"halt", "puschc", "add", "sub", "mul", "div", "mod", "rdint", "wrint", "rdchr", "wrchr", "puschg", "popg", "asf", "rsf", "pushl", "popl", "eq", "ne", "lt", "le", "gt", "ge", "jmp", "brf", "brt", "call", "ret", "drop", "pushr", "popr", "dup", "new", "getf", "putf", "newa", "getfa", "putfa", "getsz", "pushn", "refeq", "refne"};
// return value register
//int rvr[1];
// caller
int ra = 0; // return adress
int statecode = 1;
//endCaller
int version = 8;
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
// VM-heap 
int heap_size_kb = 8192;  // Default 8 MiB
char *heap = NULL;           // Der Heap-Speicher
char *heap_ptr = NULL;       // Aktuelle Position im Heap
int heap_used = 0;        // Verwendeter Speicher
// heap teilen
char *heap_half1 = NULL;
char *heap_half2 = NULL;
int half_size = 0;
char *current_half = NULL;
char *current_heap_ptr = NULL;
//stack
Stackslot *stack = NULL;
int stack_size_kb = 64;
ObjRef* SDA = NULL; // in `open()` malloc für `infos[3]`
ObjRef rvr[1];
unsigned int *prog_memory = NULL;
//unsigned int *SDA = NULL;
int u = 0;
// Neue Defines für den Garbage Collector
#define BROKEN_HEART (1 << 30)        // Broken Heart Flag
#define FORWARD_MASK (~(0xC0000000))  // Maske für Forward Pointer (30 Bits)

#define IS_PRIM(objRef) (((objRef)->size & MSB) == 0)
#define IS_COPIED(objRef) ((objRef)->size & BROKEN_HEART)
#define SET_BROKEN_HEART(objRef) ((objRef)->size |= BROKEN_HEART)
#define GET_FORWARD_POINTER(objRef) ((objRef)->size & FORWARD_MASK)
#define SET_FORWARD_POINTER(objRef, pos) ((objRef)->size = BROKEN_HEART | (pos))
// Globale Variablen
bool gc_purge_flag = false;  // Standardmäßig deaktiviert


int stackAllocate() {
    stack = malloc(stack_size_kb * 1024);
    if (stack == NULL) {
        fatalError("Could not allocate stack memory");
    }
    return 0;
}

int heapAllocate() {
    heap = malloc(heap_size_kb * 1024);
    if (heap == NULL) {
        fatalError("Could not allocate heap memory");
    }
    heap_ptr = heap;
    return 0;
}

int heapInitialise() {
    // Initialisierung in main():
    half_size = (heap_size_kb * 512);
    heap_half1 = heap;
    heap_half2 = heap + half_size;
    current_half = heap_half1;
    current_heap_ptr = current_half;
    return 0;
}

ObjRef copy_object(ObjRef obj, char *new_half, char **free_ptr) {
    // Wenn das Objekt bereits kopiert wurde, gib den Forward Pointer zurück
    if (IS_COPIED(obj)) {
        return (ObjRef)(new_half + GET_FORWARD_POINTER(obj));
    }

    // Größe des Objekts berechnen
    size_t obj_size;
    if (IS_PRIM(obj)) {
        obj_size = sizeof(unsigned int) + obj->size;
    } else {
        obj_size = sizeof(unsigned int) + GET_SIZE(obj) * sizeof(ObjRef);
    }

    // Objekt kopieren
    memcpy(*free_ptr, obj, obj_size);
    ObjRef new_obj = (ObjRef)*free_ptr;
    
    // Forward Pointer im alten Objekt setzen
    size_t offset = *free_ptr - new_half;
    SET_FORWARD_POINTER(obj, offset);
    SET_BROKEN_HEART(obj);

    // Freien Pointer aktualisieren
    *free_ptr += obj_size;

    return new_obj;
}

void garbage_collect() {
    // Wechsle die Heap-Hälften
    char *old_half = current_half;
    char *new_half = (current_half == heap_half1) ? heap_half2 : heap_half1;
    char *scan_ptr = new_half;
    char *free_ptr = new_half;

    // 1. Phase: Root-Set kopieren
    // a) Statischer Datenbereich (SDA)
    for (int i = 0; i < infos[3]; i++) {
        if (SDA[i] != NULL && ((char*)SDA[i] >= old_half && (char*)SDA[i] < old_half + half_size)) {
            SDA[i] = copy_object(SDA[i], new_half, &free_ptr);
        }
    }

    // b) Return-Value Register
    if (rvr[0] != NULL && ((char*)rvr[0] >= old_half && (char*)rvr[0] < old_half + half_size)) {
        rvr[0] = copy_object(rvr[0], new_half, &free_ptr);
    }

    // c) Stack durchlaufen
    for (int i = 0; i < stackpointer; i++) {
        if (stack[i].isObjref && stack[i].u.objRef != NULL && 
            ((char*)stack[i].u.objRef >= old_half && (char*)stack[i].u.objRef < old_half + half_size)) {
            stack[i].u.objRef = copy_object(stack[i].u.objRef, new_half, &free_ptr);
        }
    }

    // d) Big-Integer-Prozessor (bip) Komponenten
    /*if (bip.op1 != NULL && ((char*)bip.op1 >= old_half && (char*)bip.op1 < old_half + half_size)) {
        bip.op1 = copy_object(bip.op1, new_half, &free_ptr);
    }
    if (bip.op2 != NULL && ((char*)bip.op2 >= old_half && (char*)bip.op2 < old_half + half_size)) {
        bip.op2 = copy_object(bip.op2, new_half, &free_ptr);
    }
    if (bip.res != NULL && ((char*)bip.res >= old_half && (char*)bip.res < old_half + half_size)) {
        bip.res = copy_object(bip.res, new_half, &free_ptr);
    }
    if (bip.rem != NULL && ((char*)bip.rem >= old_half && (char*)bip.rem < old_half + half_size)) {
        bip.rem = copy_object(bip.rem, new_half, &free_ptr);
    }
*/
    // 2. Phase: Transitive Hülle kopieren
    while (scan_ptr < free_ptr) {
        ObjRef obj = (ObjRef)scan_ptr;
        
        if (!IS_PRIM(obj)) {
            // Compound-Objekt - Referenzen aktualisieren
            int num_refs = GET_SIZE(obj);
            ObjRef *refs = GET_REFS(obj);
            
            for (int i = 0; i < num_refs; i++) {
                if (refs[i] != NULL && ((char*)refs[i] >= old_half && (char*)refs[i] < old_half + half_size)) {
                    refs[i] = copy_object(refs[i], new_half, &free_ptr);
                }
            }
        }
        
        // Zum nächsten Objekt gehen
        scan_ptr += (IS_PRIM(obj)) ? sizeof(unsigned int) + obj->size : 
                                     sizeof(unsigned int) + GET_SIZE(obj) * sizeof(ObjRef);
    }
    // Heap-Hälften aktualisieren
    current_half = new_half;
    current_heap_ptr = free_ptr;
    heap_used = free_ptr - new_half;

    // --gcpurge Funktionalität
    if (gc_purge_flag) {
        memset(old_half, 0, half_size);  // Alten Halbspeicher mit Nullen überschreiben
        //printf("GC Purge: Old heap half zeroed out\n");  // Debug-Ausgabe
    }
}

void * newPrimObject(int dataSize) {
    int total_size = sizeof(unsigned int) + (dataSize * sizeof(unsigned char));
    // Versuche Allokation
    if (current_heap_ptr + total_size > current_half + half_size) {
        garbage_collect();
        // Nach GC nochmal versuchen
        if (current_heap_ptr + total_size > current_half + half_size) {
            fatalError("Out of memory after garbage collection");
        }
    }

    ObjRef bigObjRef = (ObjRef)current_heap_ptr;
    current_heap_ptr += total_size;
    heap_used += total_size;
    
    bigObjRef->size = dataSize;
    return bigObjRef;
  }

void * newCompoundObject(int objSize) {
    int total_size = sizeof(unsigned int) + (objSize * sizeof(ObjRef));
    // Versuche Allokation
    if (current_heap_ptr + total_size > current_half + half_size) {
        garbage_collect();
        // Nach GC nochmal versuchen
        if (current_heap_ptr + total_size > current_half + half_size) {
            fatalError("Out of memory after garbage collection");
        }
    }

    ObjRef compObjRef = (ObjRef)current_heap_ptr;
    current_heap_ptr += total_size;
    heap_used += total_size;
    
    compObjRef->size = SET_MSB(objSize);
    return compObjRef;
}

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
    SDA = (ObjRef *)malloc(infos[3] * sizeof(unsigned int));
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
    //printf("\nOpcode %d", opcode);
    signed int value = instruction & 0x00FFFFFF;
    value = sign_extend(value);
    //signed int result = 0;
    switch (opcode)
    {
    case halt:
        break;
    case puschc:
    {
        Stackslot stackslot;
        int val = sign_extend(value);
        bigFromInt(val);
        ObjRef obj = newPrimObject(sizeof(bip.res));
        *(BigObjRef *)obj->data = bip.res;
        stackslot.isObjref = true;
        stackslot.u.objRef = obj;
        stack[stackpointer] = stackslot;
        stackpointer = stackpointer + 1;
        break;
    }
    case add:
    {
        if(stackpointer <= 1) {
            printf("AAA");
            exit(99);
        }
        Stackslot stackslot;
        Stackslot a1 = stack[stackpointer - 2];
        Stackslot a2 = stack[stackpointer - 1];
        BigObjRef val1 = *(BigObjRef *)a1.u.objRef->data;
        BigObjRef val2 = *(BigObjRef *)a2.u.objRef->data;
        bip.op1 = val1;
        bip.op2 = val2;
        bigAdd();
        ObjRef res = newPrimObject(sizeof(bip.res));
        *(BigObjRef *)res->data = bip.res;
        stackslot.isObjref = true;
        stackslot.u.objRef = res;
        stack[stackpointer - 2] = stackslot;
        stackpointer = stackpointer - 1;
        break;
    }
    case sub:
    {
        if(stackpointer <= 1) {
            printf("AAA");
            exit(99);
        }
        Stackslot stackslot;
        Stackslot a1 = stack[stackpointer - 2];
        Stackslot a2 = stack[stackpointer - 1];
        BigObjRef val1 = *(BigObjRef *)a1.u.objRef->data;
        BigObjRef val2 = *(BigObjRef *)a2.u.objRef->data;
        bip.op1 = val1;
        bip.op2 = val2;
        bigSub();
        //BigObjRef result = bip.res;
        ObjRef res = newPrimObject(sizeof(bip.res));
        *(BigObjRef *)res->data = bip.res;
        stackslot.isObjref = true;
        stackslot.u.objRef = res;
        stack[stackpointer - 2] = stackslot;
        stackpointer = stackpointer - 1;
        break;
    }
    case mul:
    {
        if(stackpointer <= 1) {
            printf("AAA");
            exit(99);
        }
        Stackslot stackslot;
        Stackslot a1 = stack[stackpointer - 2];
        Stackslot a2 = stack[stackpointer - 1];
        BigObjRef val1 = *(BigObjRef *)a1.u.objRef->data;
        BigObjRef val2 = *(BigObjRef *)a2.u.objRef->data;
        bip.op1 = val1;
        bip.op2 = val2;
        bigMul();
        /*bip.op1 = bip.res;
        printf("mul:::");
        bigPrint(stdout);
        printf("\n");
        BigObjRef result = bip.res;*/
        ObjRef res = newPrimObject(sizeof(bip.res));
        *(BigObjRef *)res->data = bip.res;
        stackslot.isObjref = true;
        stackslot.u.objRef = res;
        stack[stackpointer - 2] = stackslot;
        stackpointer = stackpointer - 1;
        break;
    }
    case div:
    {
        if(stackpointer <= 1) {
            printf("AAA");
            exit(99);
        }
        /*if ((val1 == 0) | (val2 == 0))
        {
            printf("cannot divide in 0\n");
            exit(99);
            break;
        }*/
        Stackslot stackslot;
        Stackslot a1 = stack[stackpointer - 2];
        Stackslot a2 = stack[stackpointer - 1];
        BigObjRef val1 = *(BigObjRef *)a1.u.objRef->data;
        BigObjRef val2 = *(BigObjRef *)a2.u.objRef->data;
        bip.op1 = val1;
        bip.op2 = val2;
        bigDiv();
        //BigObjRef result = bip.res;
        ObjRef res = newPrimObject(sizeof(bip.res));
        *(BigObjRef *)res->data = bip.res;
        stackslot.isObjref = true;
        stackslot.u.objRef = res;
        stack[stackpointer - 2] = stackslot;
        stackpointer = stackpointer - 1;
        break;
    }
    case mod:
    {
        if(stackpointer <= 1) {
            printf("AAA");
            exit(99);
        }
        /*if ((val1 == 0) | (val2 == 0))
        {
            printf("cannot divide in 0\n");
            stackpointer--;
            break;
        }*/
        Stackslot stackslot;
        Stackslot a1 = stack[stackpointer - 2];
        Stackslot a2 = stack[stackpointer - 1];
        BigObjRef val1 = *(BigObjRef *)a1.u.objRef->data;
        BigObjRef val2 = *(BigObjRef *)a2.u.objRef->data;
        bip.op1 = val1;
        bip.op2 = val2;
        bigDiv();
        //BigObjRef result = bip.rem;
        ObjRef res = newPrimObject(sizeof(bip.rem));
        *(BigObjRef *)res->data = bip.rem;
        stackslot.isObjref = true;
        stackslot.u.objRef = res;
        stack[stackpointer - 2] = stackslot;
        stackpointer = stackpointer - 1;
        break;
    }
    case rdint:
    {
        Stackslot stackslot;
        bigRead(stdin);
        BigObjRef result = bip.res;
        ObjRef res = newPrimObject(sizeof(result));
        *(BigObjRef *)res->data = result;
        stackslot.isObjref = true;
        stackslot.u.objRef = res;
        stack[stackpointer - 1] = stackslot;
        stackpointer = stackpointer + 1;
        break;
    }
    case wrint:
    {
        Stackslot stackslot = stack[stackpointer - 1];
        BigObjRef result = *(BigObjRef *)stackslot.u.objRef->data;
        bip.op1 = result;
        bigPrint(stdout);
        stackpointer = stackpointer - 1;
        break;
    }
    case rdchr:
    {
        Stackslot stackslot;
        char chr_result = ' ';
        scanf("%c", &chr_result);
        int result = (unsigned int)chr_result;
        bigFromInt(result);
        ObjRef res = newPrimObject(sizeof(bip.res));
        *(BigObjRef *)res->data = bip.res;
        stackslot.isObjref = true;
        stackslot.u.objRef = res;
        stack[stackpointer - 1] = stackslot;
        stackpointer = stackpointer + 1;
        break;
    }
    case wrchr:
    {
        Stackslot stackslot = stack[stackpointer - 1];
        BigObjRef result = *(BigObjRef *)stackslot.u.objRef->data;
        bip.op1 = result;
        int res = bigToInt();
        char chr_result = (char)(res);
        printf("%c", chr_result);
        stackpointer = stackpointer - 1;
        break;
    }
    case pushg:
    {
        Stackslot stackslot;
        ObjRef obj = SDA[value];
        stackslot.isObjref = true;
        stackslot.u.objRef = obj;
        stack[stackpointer] = stackslot;
        stackpointer = stackpointer + 1;
        break;
    }
    case popg:
    {
        Stackslot stackslot = stack[stackpointer - 1];
        if(stackslot.isObjref == false) {
            printf("falsch");
            exit(99);
        }    
        SDA[value] = stackslot.u.objRef;
        stackpointer = stackpointer - 1;
        break;
    }
    case asf:
    {
        value = sign_extend(value);
        if(value < 0) {
            printf("AAA");
            exit(99);
        }
        stack[stackpointer].isObjref = false;
        stack[stackpointer].u.number = fp;
        stackpointer = stackpointer + 1;
        fp = stackpointer;
        for(int i = 0; i < value; i++) {
            stack[stackpointer + i].isObjref = true;
        }
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
        fp = stack[stackpointer - 1].u.number;
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
        Stackslot stackslot = stack[fp + value];
        stack[stackpointer].isObjref = true;
        stack[stackpointer] = stackslot;
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
        stack[fp + value].isObjref = true;
        stack[fp + value] = stack[stackpointer - 1];
        stackpointer = stackpointer - 1;
        break;
    }
    case eq:
    {
        Stackslot stackslot;
        Stackslot a1 = stack[stackpointer - 2];
        Stackslot a2 = stack[stackpointer - 1];
        BigObjRef val1 = *(BigObjRef *)a1.u.objRef->data;
        BigObjRef val2 = *(BigObjRef *)a2.u.objRef->data;
        bip.op1 = val1;
        bip.op2 = val2;
        int result = bigCmp();
        bool cmp = (result == 0);
        result = cmp ? 1 : 0;
        bigFromInt(result);
        ObjRef res = newPrimObject(sizeof(bip.res));
        *(BigObjRef *)res->data = bip.res;
        stackslot.isObjref = true;
        stackslot.u.objRef = res;
        stack[stackpointer - 2] = stackslot;
        stackpointer = stackpointer - 1;
        break;
    }
    case ne:
    {
        Stackslot stackslot;
        Stackslot a1 = stack[stackpointer - 2];
        Stackslot a2 = stack[stackpointer - 1];
        BigObjRef val1 = *(BigObjRef *)a1.u.objRef->data;
        BigObjRef val2 = *(BigObjRef *)a2.u.objRef->data;
        bip.op1 = val1;
        bip.op2 = val2;
        int result = bigCmp();
        bool cmp = (result == 0);
        result = cmp ? 0 : 1;
        bigFromInt(result);
        ObjRef res = newPrimObject(sizeof(bip.res));
        *(BigObjRef *)res->data = bip.res;
        stackslot.isObjref = true;
        stackslot.u.objRef = res;
        stack[stackpointer - 2] = stackslot;
        stackpointer = stackpointer - 1;
        break;
    }
    case lt:
    {
        Stackslot stackslot;
        Stackslot a1 = stack[stackpointer - 2];
        Stackslot a2 = stack[stackpointer - 1];
        BigObjRef val1 = *(BigObjRef *)a1.u.objRef->data;
        BigObjRef val2 = *(BigObjRef *)a2.u.objRef->data;
        bip.op1 = val1;
        bip.op2 = val2;
        int result = bigCmp();
        bool cmp = (result < 0);
        result = cmp ? 1 : 0;
        bigFromInt(result);
        ObjRef res = newPrimObject(sizeof(bip.res));
        *(BigObjRef *)res->data = bip.res;
        stackslot.isObjref = true;
        stackslot.u.objRef = res;
        stack[stackpointer - 2] = stackslot;
        stackpointer = stackpointer - 1;
        break;
    }
    case le:
    {
        Stackslot stackslot;
        Stackslot a1 = stack[stackpointer - 2];
        Stackslot a2 = stack[stackpointer - 1];
        BigObjRef val1 = *(BigObjRef *)a1.u.objRef->data;
        BigObjRef val2 = *(BigObjRef *)a2.u.objRef->data;
        bip.op1 = val1;
        bip.op2 = val2;
        int result = bigCmp();
        bool cmp = (result <= 0);
        result = cmp ? 1 : 0;
        bigFromInt(result);
        ObjRef res = newPrimObject(sizeof(bip.res));
        *(BigObjRef *)res->data = bip.res;
        stackslot.isObjref = true;
        stackslot.u.objRef = res;
        stack[stackpointer - 2] = stackslot;
        stackpointer = stackpointer - 1;
        break;
    }
    case gt:
    {
        Stackslot stackslot;
        Stackslot a1 = stack[stackpointer - 2];
        Stackslot a2 = stack[stackpointer - 1];
        BigObjRef val1 = *(BigObjRef *)a1.u.objRef->data;
        BigObjRef val2 = *(BigObjRef *)a2.u.objRef->data;
        bip.op1 = val1;
        bip.op2 = val2;
        int result = bigCmp();
        bool cmp = (result > 0);
        result = cmp ? 1 : 0;
        bigFromInt(result);
        ObjRef res = newPrimObject(sizeof(bip.res));
        *(BigObjRef *)res->data = bip.res;
        stackslot.isObjref = true;
        stackslot.u.objRef = res;
        stack[stackpointer - 2] = stackslot;
        stackpointer = stackpointer - 1;
        break;
    }
    case ge:
    {
        Stackslot stackslot;
        Stackslot a1 = stack[stackpointer - 2];
        Stackslot a2 = stack[stackpointer - 1];
        BigObjRef val1 = *(BigObjRef *)a1.u.objRef->data;
        BigObjRef val2 = *(BigObjRef *)a2.u.objRef->data;
        bip.op1 = val1;
        bip.op2 = val2;
        int result = bigCmp();
        bool cmp = (result >= 0);
        result = cmp ? 1 : 0;
        bigFromInt(result);
        ObjRef res = newPrimObject(sizeof(bip.res));
        *(BigObjRef *)res->data = bip.res;
        stackslot.isObjref = true;
        stackslot.u.objRef = res;
        stack[stackpointer - 2] = stackslot;
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
        Stackslot a = stack[--stackpointer];
        BigObjRef val = *(BigObjRef *)a.u.objRef->data;
        bip.op1 = val;
        int result = bigToInt();
        if ((stack[stackpointer].isObjref == true) && (result == 0)) pc = value;
        break;
        break;
    }
    case brt:
    {
        value = sign_extend(value);
        Stackslot a = stack[--stackpointer];
        BigObjRef val = *(BigObjRef *)a.u.objRef->data;
        bip.op1 = val;
        int result = bigToInt();
        if ((stack[stackpointer].isObjref == true) && (result == 1)) pc = value;
        break;
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
        Stackslot stackslot;
        stackslot.isObjref = false;
        stackslot.u.number = ra;
        stack[stackpointer] = stackslot;
        stackpointer++;
        pc = value;
        break;
    }
    case ret:
    {
        pc = stack[stackpointer - 1].u.number;
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
         // return value
        Stackslot stackslot;
        stackslot.isObjref = true;
        stackslot.u.objRef = rvr[0];
        stack[stackpointer] = stackslot;
        stackpointer++;
        break;
    }
    case popr: 
    {
        if(stackpointer == 0) {
            printf("GGG");
            exit(99);
        }
        Stackslot stackslot = stack[stackpointer - 1];// return value
        rvr[0] = stackslot.u.objRef;
        stackpointer--;
        break;
    }
    case dup:
    {
        stack[stackpointer] = stack[stackpointer - 1];
        stackpointer++;
        break;
    }
    case new:
    {
        ObjRef object = newCompoundObject(value);
        int size = GET_SIZE(object);
        for(int i = 0; i < size; i++) {
            ((ObjRef *)(object)->data)[i] = NULL;
        }
        Stackslot stackslot;
        stackslot.isObjref = true;
        stackslot.u.objRef = object;
        stack[stackpointer] = stackslot;
        stackpointer++;
        break;
    }
    case getf:
    {
        Stackslot stackslot;
        ObjRef object = stack[--stackpointer].u.objRef;
        bool isObjRef = IS_PRIM(object);
        if(isObjRef) {
            printf("Cannot handle this operation on primitiv objects");
            exit(99);
        }
        errorPrint(object, value);
        ObjRef objRef = GET_REFS(object)[value];
        stackslot.isObjref = true;
        stackslot.u.objRef = objRef;
        stack[stackpointer] = stackslot;
        stackpointer++;
        break;
    }
    case putf:
    {
        ObjRef val = (ObjRef)stack[--stackpointer].u.objRef;
        ObjRef object = (ObjRef)stack[--stackpointer].u.objRef;
        bool isObjRef = IS_PRIM(object);
        if(isObjRef) {
            printf("Cannot handle this operation on primitiv objects");
            exit(99);
        }
        ((ObjRef *)(object)->data)[value] = val;
        break;
    }
    case newa:
    {
        ObjRef objRef = stack[--stackpointer].u.objRef;
        bool isObjRef = IS_PRIM(objRef);
        if(isObjRef == false) {
            printf("Cannot handle this operation on compound objects");
            exit(99);
        }
        bip.op1 = *(BigObjRef *)objRef->data;
        int value = bigToInt();
        ObjRef object = newCompoundObject(value);
        int size = GET_SIZE(object);
        for(int i = 0; i < size; i++) {
            ((ObjRef *)(object)->data)[i] = NULL;
        }
        Stackslot stackslot;
        stackslot.isObjref = true;
        stackslot.u.objRef = object;
        stack[stackpointer] = stackslot;
        stackpointer++;
        break;
    }
    case getfa:
    {
        Stackslot stackslot;
        ObjRef index = stack[--stackpointer].u.objRef;
        ObjRef object = stack[--stackpointer].u.objRef;
        bool isObjRef = IS_PRIM(object);
        bool isNotObjRef = !(IS_PRIM(index));
        if(isObjRef && isNotObjRef) {
            printf("Cannot handle this operation on primitiv objects");
            exit(99);
        }
        bip.op1 = *(BigObjRef *)index->data;
        int value = bigToInt();
        errorPrint(object, value);
        ObjRef objRef = GET_REFS(object)[value];
        stackslot.isObjref = true;
        stackslot.u.objRef = objRef;
        stack[stackpointer] = stackslot;
        stackpointer++;
        break;
    }
    case putfa:
    {
        ObjRef val = stack[--stackpointer].u.objRef;
        ObjRef index = stack[--stackpointer].u.objRef;
        ObjRef object = stack[--stackpointer].u.objRef;
        bool isObjRef = IS_PRIM(object);
        bool isNotObjRef = !(IS_PRIM(index));
        bool isNotObjRefVal = !(IS_PRIM(index));
        if(isObjRef && isNotObjRef && isNotObjRefVal) {
            printf("Cannot handle this operation on primitiv objects");
            exit(99);
        }
        bip.op1 = *(BigObjRef *)index->data;
        int value = bigToInt();
        ((ObjRef *)(object)->data)[value] = val;
        break;
    }
    case getsz:
    {
        ObjRef object = stack[--stackpointer].u.objRef;
        bool isObjRef = IS_PRIM(object);
        int size;
        if(isObjRef) {
            size = -1;
        } else {
        size = GET_SIZE(object);
        }
        bigFromInt(size);
        ObjRef res = newPrimObject(sizeof(bip.res));
        *(BigObjRef *)res->data = bip.res;
        Stackslot stackslot;
        stackslot.isObjref = true;
        stackslot.u.objRef = res;
        stack[stackpointer] = stackslot;
        stackpointer++;
        break;
    }
    case puschn:
    {
        Stackslot stackslot;
        stackslot.isObjref = true;
        stackslot.u.objRef = NULL;
        stack[stackpointer] = stackslot;
        stackpointer++;
        break;
    }
    case refeq:
    {
        ObjRef ref1 = stack[--stackpointer].u.objRef;
        ObjRef ref2 = stack[--stackpointer].u.objRef;
        int equal = (ref1 == ref2)? 1 : 0;
        bigFromInt(equal);
        ObjRef res = newPrimObject(sizeof(bip.res));
        *(BigObjRef *)res->data = bip.res;
        Stackslot stackslot;
        stackslot.isObjref = true;
        stackslot.u.objRef = res;
        stack[stackpointer] = stackslot;
        stackpointer++;
        break;
    }
    case refne:
    {
        ObjRef ref1 = stack[--stackpointer].u.objRef;
        ObjRef ref2 = stack[--stackpointer].u.objRef;
        int equal = (ref1 != ref2)? 1 : 0;
        bigFromInt(equal);
        ObjRef res = newPrimObject(sizeof(bip.res));
        *(BigObjRef *)res->data = bip.res;
        Stackslot stackslot;
        stackslot.isObjref = true;
        stackslot.u.objRef = res;
        stack[stackpointer] = stackslot;
        stackpointer++;
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

    if ((opcode == 1) | (opcode == 11) | (opcode == 12) | (opcode == 13) | (opcode == 15) | (opcode == 16) | (opcode == 23) | (opcode == 24) | (opcode == 25) | (opcode == 26) | (opcode == 28) | (opcode == 32) | (opcode == 33) | (opcode == 34))
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
        value = sign_extend(value);
        if ((opcode == 1) | (opcode == 11) | (opcode == 12) | (opcode == 13) | (opcode == 15) | (opcode == 16) | (opcode == 23) | (opcode == 24) | (opcode == 25) | (opcode == 26) | (opcode == 28) | (opcode == 32) | (opcode == 33) | (opcode == 34))
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
        printf("data[%04d]:     %p\n", i, (void *)SDA[i]);
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
        else if(i == fp && stack[i].isObjref) {
            printf("fp      --->    %04d:   (ObjRef)%p\n", i, (void *)stack[i].u.objRef);
        }
        else if(i == fp && !stack[i].isObjref) {
            printf("fp      --->    %04d:   (number)%d\n", i, stack[i].u.number);
        }
        else if(stack[i].isObjref == false) {
            printf("        --->    %04d:   (number)%d\n", i, stack[i].u.number);
        }
        else {
            printf("                %04d:   (ObjRef)%p\n", i, (void *)stack[i].u.objRef);
        }
    }
    printf("                --- bottom of stack ---\n");
    return 0;
} 

int object(void) {
    ObjRef addr;
    printf("object reference?");
    scanf("%p", (void **)&addr);
    printf("Object at %p: value = ", (void *)addr);
    BigObjRef op = *(BigObjRef *)addr->data;
    //BigObjRef op = (BigObjRef)obj.u.objRef;
    bip.op1 = op;
    bigPrint(stdout);
    printf("\n");
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
    }  else
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
        if ((opcode == 1) | (opcode == 11) | (opcode == 12) | (opcode == 13) | (opcode == 15) | (opcode == 16) | (opcode == 23) | (opcode == 24) | (opcode == 25) | (opcode == 26) | (opcode == 28) | (opcode == 32) | (opcode == 33) | (opcode == 34))
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
    else if (strcmp(commando, "s\n") == 0)
    {
        if(statecode == halt) return 0;
        for(int i = 0; i < 20; i++) {
            step(path);
        }
        
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
        if ((opcode == 1) | (opcode == 11) | (opcode == 12) | (opcode == 13) | (opcode == 15) | (opcode == 16) | (opcode == 23) | (opcode == 24) | (opcode == 25) | (opcode == 26) | (opcode == 28) | (opcode == 32) | (opcode == 33) | (opcode == 34))
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
        if ((opcode == 1) | (opcode == 11) | (opcode == 12) | (opcode == 13) | (opcode == 15) | (opcode == 16) | (opcode == 23) | (opcode == 24) | (opcode == 25) | (opcode == 26) | (opcode == 28) | (opcode == 32) | (opcode == 33) | (opcode == 34))
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
    if ((opcode == 1) | (opcode == 11) | (opcode == 12) | (opcode == 13) | (opcode == 15) | (opcode == 16) | (opcode == 23) | (opcode == 24) | (opcode == 25) | (opcode == 26) | (opcode == 28) | (opcode == 32) | (opcode == 33) | (opcode == 34))
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

int main(int argc, char *argv[])
{
    int codenumber = 0;
    for (int line = 1; line < argc; line++)
    {
        if((strcmp(argv[line], "--stack") == 0) || (strcmp(argv[line], "--heap") == 0)) {
            line++;
        }
        else if ((*argv[line] != '-') ) {
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
        if (((*argv[line] == '-') && (strcmp(argv[line], "--debug") != 0) && (strcmp(argv[line], "--stack") != 0) && (strcmp(argv[line], "--heap") != 0) && (strcmp(argv[line], "--gcpurge") != 0)))
        {
            test00(line, argv);
            return 0;
        }
        else {
            
        }
    }
    for (int line = 1; line < argc; line++)
    {
        if (strcmp(argv[line], "--stack") == 0)
        {   
            int n = atoi(argv[line + 1]);
            stack_size_kb = n;
        }
        else if (strcmp(argv[line], "--heap") == 0)
        {   
            int n = atoi(argv[line + 1]);
            heap_size_kb  = n;
        }
        else if (strcmp(argv[line], "--gcpurge") == 0) {
            gc_purge_flag = false;
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
            //stackallokation
            stackAllocate();
            heapAllocate();
            heapInitialise();
            debug(argc, argv);
            free(stack);
            free(heap);
            return 0;
        }
        else {
            codenumber = codenumber;
        }
    }

    if(codenumber == 1) {
        printf("Ninja Virtual Machine started\n");
        //stackallokation
        stackAllocate();
        heapAllocate();
        heapInitialise();
        open();
        run(path);
        free(stack);
        free(heap);
        printf("Ninja Virtual Machine stopped\n");
        return 0;
    } else if (codenumber > 1) {
        printf("Error: more than one code file specified\n");
        exit(99);
        return 0;
    }
    
    return 0;
}
