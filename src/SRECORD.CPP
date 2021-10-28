// srecord.cpp
// member functions for s-record files
// Written by Richard E. Haskell -- February 1995

#include <stdio.h>

#include "srecord.h"

// s-record member functions

void	srecord::make_srecord_file(FILE * filename, char* vseg,
			int ix_start, int ix_end, int dest_addr)
	{
	int num_bytes, num_lines, i;
	num_bytes = ix_end - ix_start + 1;
	num_lines = (num_bytes + 31)/32;
	for(i = 0; i < num_lines; i++)
	   send_32bytes(filename, vseg, ix_start, i, dest_addr);
	send_s9_line(filename);
	}

void	srecord::send_32bytes(FILE * filename,
		char* vseg, int start, int i, int dest_addr)
	{
	int j;
	checksum = 0;
	fprintf(filename, "%c", 'S');			// send S
	fprintf(filename, "%c", '1');			// send 1
	send_ascii_byte(filename, 35);                   // send #bytes
	send_ascii_word(filename, dest_addr + i*32);		// send addr
	for(j = 0; j < 32; j++)
	   send_ascii_byte(filename, vseg[start + i*32 + j]);	// send data
	send_ascii_byte(filename, ~checksum);
	fprintf(filename, "\n");
	}

void	srecord::send_s9_line(FILE * filename)
	{
	checksum = 0;
	fprintf(filename, "%c", 'S');							// send S
	fprintf(filename, "%c", '9');							// send 9
	send_ascii_byte(filename, 3);                   // send #bytes
	send_ascii_word(filename, 0);							// send addr
	send_ascii_byte(filename, ~checksum);           // send checksum
	fprintf(filename, "\n\n");
	}

int	srecord::hex2asc(int n)
	{
	if(n > 9)               		// if n > 9
	   n += 0x37;                	//    add 37H
	else
	   n += 0x30;             		// else add 30H
	return n;
	}

void	srecord::send_ascii_byte(FILE * filename, int n)
	{
	int hi_nibble, lo_nibble;
	checksum += n;					// add to checksum
	hi_nibble = (n >> 4) & 0x0f;
	fprintf(filename, "%c", hex2asc(hi_nibble));   	// send hi_nibble
	lo_nibble = n & 0x0f;
	fprintf(filename, "%c", hex2asc(lo_nibble));   	// send lo_nibble
	}

void	srecord::send_ascii_word(FILE * filename, int n)
	{
	int hi_byte, lo_byte;
	hi_byte = (n >> 8) & 0xff;
	send_ascii_byte(filename, hi_byte);
	lo_byte = n & 0xff;
	send_ascii_byte(filename, lo_byte);
	}
