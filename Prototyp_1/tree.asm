//
// version
//
        .vers   7

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
// void writeString(String)
//
_writeString:
        asf     1
        pushc   0
        popl    0
        jmp     _writeString_L2
_writeString_L1:
        pushl   -3
        pushl   0
        getfa
        call    _writeCharacter
        drop    1
        pushl   0
        pushc   1
        add
        popl    0
_writeString_L2:
        pushl   0
        pushl   -3
        getsz
        lt
        brt     _writeString_L1
        rsf
        ret

//
// record { Boolean isLeaf; Character op; Node left; Node right; Integer num; } newLeafNode(Integer)
//
_newLeafNode:
        asf     1
        new     5
        popl    0
        pushl   0
        pushc   1
        putf    0
        pushl   0
        pushl   -3
        putf    4
        pushl   0
        popr
        jmp     __0
__0:
        rsf
        ret

//
// record { Boolean isLeaf; Character op; Node left; Node right; Integer num; } newInnerNode(Character, record { Boolean isLeaf; Character op; Node left; Node right; Integer num; }, record { Boolean isLeaf; Character op; Node left; Node right; Integer num; })
//
_newInnerNode:
        asf     1
        new     5
        popl    0
        pushl   0
        pushc   0
        putf    0
        pushl   0
        pushl   -5
        putf    1
        pushl   0
        pushl   -4
        putf    2
        pushl   0
        pushl   -3
        putf    3
        pushl   0
        popr
        jmp     __1
__1:
        rsf
        ret

//
// void compileTree(record { Boolean isLeaf; Character op; Node left; Node right; Integer num; })
//
_compileTree:
        asf     0
        pushl   -3
        getf    0
        brf     __3
        pushc   7
        newa
        dup
        pushc   0
        pushc   9
        putfa
        dup
        pushc   1
        pushc   112
        putfa
        dup
        pushc   2
        pushc   117
        putfa
        dup
        pushc   3
        pushc   115
        putfa
        dup
        pushc   4
        pushc   104
        putfa
        dup
        pushc   5
        pushc   99
        putfa
        dup
        pushc   6
        pushc   9
        putfa
        call    _writeString
        drop    1
        pushl   -3
        getf    4
        call    _writeInteger
        drop    1
        pushc   1
        newa
        dup
        pushc   0
        pushc   10
        putfa
        call    _writeString
        drop    1
        jmp     __4
__3:
        pushl   -3
        getf    2
        call    _compileTree
        drop    1
        pushl   -3
        getf    3
        call    _compileTree
        drop    1
        pushl   -3
        getf    1
        pushc   43
        eq
        brf     __5
        pushc   5
        newa
        dup
        pushc   0
        pushc   9
        putfa
        dup
        pushc   1
        pushc   97
        putfa
        dup
        pushc   2
        pushc   100
        putfa
        dup
        pushc   3
        pushc   100
        putfa
        dup
        pushc   4
        pushc   10
        putfa
        call    _writeString
        drop    1
        jmp     __2
__5:
        pushl   -3
        getf    1
        pushc   45
        eq
        brf     __6
        pushc   5
        newa
        dup
        pushc   0
        pushc   9
        putfa
        dup
        pushc   1
        pushc   115
        putfa
        dup
        pushc   2
        pushc   117
        putfa
        dup
        pushc   3
        pushc   98
        putfa
        dup
        pushc   4
        pushc   10
        putfa
        call    _writeString
        drop    1
        jmp     __2
__6:
        pushl   -3
        getf    1
        pushc   42
        eq
        brf     __7
        pushc   5
        newa
        dup
        pushc   0
        pushc   9
        putfa
        dup
        pushc   1
        pushc   109
        putfa
        dup
        pushc   2
        pushc   117
        putfa
        dup
        pushc   3
        pushc   108
        putfa
        dup
        pushc   4
        pushc   10
        putfa
        call    _writeString
        drop    1
        jmp     __2
__7:
        pushl   -3
        getf    1
        pushc   47
        eq
        brf     __8
        pushc   5
        newa
        dup
        pushc   0
        pushc   9
        putfa
        dup
        pushc   1
        pushc   100
        putfa
        dup
        pushc   2
        pushc   105
        putfa
        dup
        pushc   3
        pushc   118
        putfa
        dup
        pushc   4
        pushc   10
        putfa
        call    _writeString
        drop    1
        jmp     __2
__8:
        pushc   25
        newa
        dup
        pushc   0
        pushc   101
        putfa
        dup
        pushc   1
        pushc   114
        putfa
        dup
        pushc   2
        pushc   114
        putfa
        dup
        pushc   3
        pushc   111
        putfa
        dup
        pushc   4
        pushc   114
        putfa
        dup
        pushc   5
        pushc   58
        putfa
        dup
        pushc   6
        pushc   32
        putfa
        dup
        pushc   7
        pushc   117
        putfa
        dup
        pushc   8
        pushc   110
        putfa
        dup
        pushc   9
        pushc   107
        putfa
        dup
        pushc   10
        pushc   110
        putfa
        dup
        pushc   11
        pushc   111
        putfa
        dup
        pushc   12
        pushc   119
        putfa
        dup
        pushc   13
        pushc   110
        putfa
        dup
        pushc   14
        pushc   32
        putfa
        dup
        pushc   15
        pushc   111
        putfa
        dup
        pushc   16
        pushc   112
        putfa
        dup
        pushc   17
        pushc   101
        putfa
        dup
        pushc   18
        pushc   114
        putfa
        dup
        pushc   19
        pushc   97
        putfa
        dup
        pushc   20
        pushc   116
        putfa
        dup
        pushc   21
        pushc   105
        putfa
        dup
        pushc   22
        pushc   111
        putfa
        dup
        pushc   23
        pushc   110
        putfa
        dup
        pushc   24
        pushc   10
        putfa
        call    _writeString
        drop    1
        call    _exit
__4:
__2:
        rsf
        ret

//
// record { Boolean isLeaf; Character op; Node left; Node right; Integer num; } makeTree()
//
_makeTree:
        asf     1
        pushc   45
        pushc   5
        call    _newLeafNode
        drop    1
        pushr
        pushc   42
        pushc   43
        pushc   1
        call    _newLeafNode
        drop    1
        pushr
        pushc   3
        call    _newLeafNode
        drop    1
        pushr
        call    _newInnerNode
        drop    3
        pushr
        pushc   45
        pushc   4
        call    _newLeafNode
        drop    1
        pushr
        pushc   7
        call    _newLeafNode
        drop    1
        pushr
        call    _newInnerNode
        drop    3
        pushr
        call    _newInnerNode
        drop    3
        pushr
        call    _newInnerNode
        drop    3
        pushr
        popl    0
        pushl   0
        popr
        jmp     __9
__9:
        rsf
        ret

//
// void main()
//
_main:
        asf     1
        call    _makeTree
        pushr
        popl    0
        pushl   0
        call    _compileTree
        drop    1
        pushc   7
        newa
        dup
        pushc   0
        pushc   9
        putfa
        dup
        pushc   1
        pushc   119
        putfa
        dup
        pushc   2
        pushc   114
        putfa
        dup
        pushc   3
        pushc   105
        putfa
        dup
        pushc   4
        pushc   110
        putfa
        dup
        pushc   5
        pushc   116
        putfa
        dup
        pushc   6
        pushc   10
        putfa
        call    _writeString
        drop    1
        pushc   6
        newa
        dup
        pushc   0
        pushc   9
        putfa
        dup
        pushc   1
        pushc   104
        putfa
        dup
        pushc   2
        pushc   97
        putfa
        dup
        pushc   3
        pushc   108
        putfa
        dup
        pushc   4
        pushc   116
        putfa
        dup
        pushc   5
        pushc   10
        putfa
        call    _writeString
        drop    1
__10:
        rsf
        ret
        