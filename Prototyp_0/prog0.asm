//
// prog1.asm -- an assembler example with global variables
//

// global Integer x;
// global Integer y;
// x = 2;
// y = x + 3;
// x = 7 * y + x;
// writeInteger(x + -33);
// writeCharacter('\n');

	pushc	4194304
	pushc	4194304
    add
	pushc	32392
    add
	wrint
	pushc	'\n'
	wrchr
	halt
 