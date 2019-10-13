#include <stdio.h>

int main(int argc, char * argv[]) {
	unsigned short regnum = 0, sep = 0, elements = 0, step = 1, PC, CR, lc3[300], r[8], SR1, SR2, ir, DR; //Assumption 100 lines is maximum as given
	int SEXT(int n, int b), imm5, PCoffset9;
	unsigned char bytes[2], cc, setcc(int n), n, z, p;
	void Out(unsigned short c, unsigned short i, unsigned short pc, unsigned short r[]);
	FILE *file;
	//printf("Initial state\n");
	for(regnum = 0; regnum < 8; regnum++){
		r[regnum] = 0; //Clears all registers
		//printf("R%d\t0x%04x\n", regnum, r[regnum]); //Initial Registers
	}

	file = fopen(argv[1], "rb+"); //Takes command line arg of obj file type
	while(fread(bytes, 1, 2, file) == 2) { //Reads the obj file in twos
		lc3[elements++] = bytes[0] << 8 | bytes[1]; //Shifts byte to MSB and concates LSB to form 2 byte word. Stores it in lc3 int array.
	}

	fclose(file);

	PC = lc3[0]; //part 4
	//printf("PC\t0x%x\n", PC); //Initial Program Counter

	ir = 0; //Clears IR
	//printf("IR\t0x%04x\n", ir); //Initial Instruction Register

	cc = 'Z';
	//printf("CC\t%c\n", cc); //Initial setccition Code
	//for (sep = 0; sep < 18; ++sep)
	//        printf ("="); //18 "="
	//if(sep == 18) {
	//          printf("\n");
	//    }
	for(step; step < elements; ++step) { //iterates through the elements in the lc3 int array except PC.
		DR = (lc3[step] >> 9) & 0x7;
		if(lc3[step] == 0xf025) { //breaks at halt
			break;
		}
		else if((lc3[step] / 0x1000) == 0x2) { //search for opcode 0010 LD in hex
			PC += 1; //increments PC
			ir = lc3[step]; //sets instruction register
			PCoffset9 = (lc3[step] % 0x1000) & 0x1ff; //sets PCoffset9 9 bits in hex.
			r[DR] = lc3[SEXT(PCoffset9, 9) + step + 1]; //finds the register and set it to the value at PCoffset9 address.
			cc = setcc(r[DR]); //determines the setccition of the value loaded
			//Out(cc, ir, PC, r);
		}
		else if((lc3[step] / 0x1000) == 0xe) { //search for opcode 1110 LEA in hex
			PC += 1; //increments PC
			PCoffset9 = (lc3[step] % 0x1000) & 0x1ff; //sets PCoffset9 9 bits in hex.
			ir = lc3[step]; //sets instruction register
			r[DR] = PC + SEXT(PCoffset9, 9); //finds the register and set it to pc + PCoffset9
			cc = setcc((short) r[DR]);
			//Out(cc, ir, PC, r);
		}
		else if((lc3[step] / 0x1000) == 0xa) { //search for opcode 1010 LDI in hex
			PC += 1; //increments PC
			PCoffset9 = (lc3[step] % 0x1000) & 0x1ff; //sets PCoffset9 9 bits in hex.
			ir = lc3[step]; //sets instruction register
			r[DR] = lc3[lc3[SEXT(PCoffset9, 9) + step + 1] % lc3[0] + 1]; //finds the register and sets it to the value at PCoffset9 address then sets the value as the address
			cc = setcc(r[DR]);
			//Out(cc, ir, PC, r);
		}
		else if((lc3[step] / 0x1000) == 0x5) { //search for opcode 0101 AND in hex
			if((lc3[step] & 0x0020) == 0) { //extracts the 5 bit. 0 for SR1, SR2. 1 for just SR1.
				PC += 1; //increments PC
				SR1 = (lc3[step] >> 6) & 0x7;
				SR2 = (lc3[step] & 0x0007);
				ir = lc3[step]; //sets instruction register
				r[DR] = r[SR1] & r[SR2]; //finds the register and sets it to the value at PCoffset9 ad$
				cc = setcc((short)  r[DR]);
				//Out(cc, ir, PC, r);
			} else {
				PC += 1; //increments PC
				SR1 = (lc3[step] >> 6) & 0x7;
				imm5 = (lc3[step] & 0x001f);
				ir = lc3[step]; //sets instruction register
				r[DR] = r[SR1] & SEXT(imm5, 5); //finds the register and sets it to the value at PCoffset9 ad$
				cc = setcc((short)  r[DR]);
				//Out(cc, ir, PC, r);
			}
		}
		else if((lc3[step] / 0x1000) == 0x9) { //search for opcode 1001 NOT in hex
			PC += 1; //increments PC
			ir = lc3[step]; //sets instruction register
			r[DR] = ~r[(lc3[step] >> 6) & 0x7]; //finds the register and sets it to the value at PCoffset9 ad$
			cc = setcc((short)  r[DR]);
			//Out(cc, ir, PC, r);
		}
		else if((lc3[step] / 0x1000) == 0x1) { //search for opcode 0001 ADD in hex
			if((lc3[step] & 0x0020) == 0) { //extracts the 5 bit. 0 for SR1, SR2. 1 for just SR1.
				PC += 1; //increments PC
				SR1 = (lc3[step] >> 6) & 0x7;
				SR2 = (lc3[step] & 0x0007);
				ir = lc3[step]; //sets instruction register
				r[DR] = r[SR1] + r[SR2]; //finds the register and sets it to the value at PCoffset9 ad$
				cc = setcc((short) r[DR]);
				//Out(cc, ir, PC, r);
			} else {
				PC += 1; //increments PC
				SR1 = (lc3[step] >> 6) & 0x7;
				imm5 = (lc3[step] & 0x001f);
				ir = lc3[step]; //sets instruction register
				r[DR] = (r[SR1] + SEXT(imm5, 5)); //finds the register and sets it to the value at PCoffset9 add
				cc = setcc((short) r[DR]);
				//Out(cc, ir, PC, r);
			}
		}
		else if((lc3[step] / 0x1000) == 0x0) { //search for opcode 0000 BR in hex
			PC += 1;
			CR = (lc3[step - 1] >> 9) & 0x7;
			ir = lc3[step]; //sets instruction register
			PCoffset9 = (lc3[step] % 0x1000) & 0x1ff; //sets PCoffset9 9 bits in hex.
			n = (lc3[step] >> 11) & 0x1;
			z = (lc3[step] >> 10) & 0x1;
			p = (lc3[step] >> 9) & 0x1;
			cc = setcc((short) r[CR]); //gets the register destination of the previous line
			if(~n & ~z & p) {
				if((short) r[CR] > 0) {
					step = step + SEXT(PCoffset9, 9);
					PC += SEXT(PCoffset9, 9);
				}
			}
			else if(~n & z & ~p) {
				if((short) r[CR] == 0) {
					step = step + SEXT(PCoffset9, 9);
					PC += SEXT(PCoffset9, 9);
				}
			}
			else if(n & ~z & ~p) {
				if((short) r[CR] < 0) {
					step = step + SEXT(PCoffset9, 9);
					PC += SEXT(PCoffset9, 9);
				}
			}
			else if(~n & z & p) {
				if((short) r[CR] >= 0) {
					step = step + SEXT(PCoffset9, 9);
					PC += SEXT(PCoffset9, 9);
				}
			}
			else if(n & ~n & p) {
				if((short) r[CR] < 0 || (short) r[CR] > 0) {
					step = step + SEXT(PCoffset9, 9);
					PC += SEXT(PCoffset9, 9);
				}
			}
			else if(n & z & ~p) {
				if((short) r[CR] <= 0) {
					step = step + SEXT(PCoffset9, 9);
					PC  += SEXT(PCoffset9, 9);
				}
			} else if(n & z & p) {
				step = step + SEXT(PCoffset9, 9);
				PC += SEXT(PCoffset9, 9);
				cc = setcc(SEXT(PCoffset9, 9));
			}
			Out(cc, ir, PC, r);
		}
	}
}

int SEXT(int n, int b) {
	int value;
	int mask;
	if(b == 5){
		value = (0x001f & n);
		mask = 0x00f0;
		if (mask & n) {
			value += 0xffe0;
		}
		return value;
	} else if(b == 9) {
		value = (0x01ff & n);
		mask = 0x0f00;
		if (mask & n) {
			value += 0xfe00;
		}
		return value;
	}
}

unsigned char setcc(int n) {
	char cc;
	if(n < 0) {
		cc = 'N';
	}
	else if(n == 0) {
		cc = 'Z';
	}
	else if(n > 0) {
		cc = 'P';
	}
	return cc;
}

void Out(unsigned short c, unsigned short i, unsigned short pc, unsigned short r[]) {
	int sep;
	printf("after executing instruction\t0x%04x\n", i);
	for(int regnum = 0; regnum < 8; regnum++){
		printf("R%d\t0x%04x\n", regnum, r[regnum]); //Initial Registers
	}
	printf("PC\t0x%x\n", pc);
	printf("IR\t0x%04x\n", i);
	printf("CC\t%c\n", c);
	for (sep = 0; sep < 18; ++sep)
	printf ("="); //18 "="
	if (sep == 18) {
		printf("\n");
	}
}



