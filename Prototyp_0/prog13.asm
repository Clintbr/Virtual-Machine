//
// version
//
        .vers   4

//
// execution framework
//
__start:
        call    _main
        call    _exit
__stop:
        jmp     __stop

//
// Integer readInteger()
//
_readInteger:
        asf     0
        rdint
        popr
        rsf
        ret

//
// void writeInteger(Integer)
//
_writeInteger:
        asf     0
        pushl   -3
        wrint
        rsf
        ret

//
// Character readCharacter()
//
_readCharacter:
        asf     0
        rdchr
        popr
        rsf
        ret

//
// void writeCharacter(Character)
//
_writeCharacter:
        asf     0
        pushl   -3
        wrchr
        rsf
        ret

//
// Integer char2int(Character)
//
_char2int:
        asf     0
        pushl   -3
        popr
        rsf
        ret

//
// Character int2char(Integer)
//
_int2char:
        asf     0
        pushl   -3
        popr
        rsf
        ret

//
// void exit()
//
_exit:
        asf     0
        halt
        rsf
        ret

//
// void main()
//
_main:
        asf     0
        call    _c
        pushr
        call    _b
        drop    1
        pushr
        call    _a
        drop    1
        pushr
        call    _writeInteger
        drop    1
        pushc   10
        call    _writeCharacter
        drop    1
__0:
        rsf
        ret

//
// Integer a(Integer)
//
_a:
        asf     0
        pushl   -3
        pushc   1
        add
        popr
        jmp     __1
__1:
        rsf
        ret

//
// Integer b(Integer)
//
_b:
        asf     0
        pushl   -3
        pushc   2
        mul
        popr
        jmp     __2
__2:
        rsf
        ret

//
// Integer c()
//
_c:
        asf     0
        pushc   3
        popr
        jmp     __3
__3:
        rsf
        ret
