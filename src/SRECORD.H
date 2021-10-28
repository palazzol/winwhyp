
// srecord.h
// srecord class for WHYP
// Written by Richard E. Haskell -- February 1995

#ifndef SRECORD_H
#define SRECORD_H

class   srecord	                	   // a Motorola s-record file
	{
	private:
	   int checksum;                  	// line checksum
	public:
	   srecord()             	     		// constructor
	      { }
	   ~srecord()           	    		// destructor
	      { }
	   void make_srecord_file(FILE *, char*,
			int, int, int);					// save srecord file
	private:
	   void send_s9_line(FILE *);
	   void send_32bytes(FILE *, char*, int, int, int);
	   int	hex2asc(int);					// hex to ascii conv
	   void send_ascii_byte(FILE *, int);
	   void send_ascii_word(FILE *, int);
	};

#endif
