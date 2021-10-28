
// wdict.h
// linked list dictionary for WHYP
// Copyright 1996 by Richard E. Haskell -- All rights reserved.
// Unified wdict11 and wdict12, Frank Palazzolo 2021

#ifndef WDICT_H
#define WDICT_H

class   header    	// one entry in dictionary
	{
	public:
	   header();						// constructor
	   ~header();						// desctructor
	   bool flag;						// true if word is in target
	   int	sub;						// address of 68HC11 code for word
	   int	size;						// number of bytes in subroutine
	   header* next;        		// ptr to next link
	   char*   name;        		// pointer to word name
	};

class   Dict 			// a linked dictionary
	{
	protected:
	   header* first;            				// ptr to first link
	   char*  tseg;		   	   			// target segment
	   int hdp;			   						// tseg dict ptr
	   int base_addr;
	   int hed_base_addr;
	   int jsr_opcode;
	public:
	   Dict(int jsr);             		    // constructor
	   ~Dict();           		    			// destructor - same as kill
	   void add_word(char*,int);        	// add word to list
	   bool is_in_dict(char*, header*&,
	       bool&, int&, int&);  	    		// is word in dict?
	   bool find_name(int, char*);   		// find name from addr
	   void load(FILE *);	    			// load an s19 file
	   int get_hex_byte(FILE *);	    		// get hex byte from file
	public:
	   void tcomma(int);		    				// compile int in tseg
	   void tccomma(int);		    			// compile char in tseg
	   void add_string(int, char*);	    	// add string to tseg
	   void get_string(int, int, char*);  // get string from tseg
	   void	get_first_string(char*);   	// get first string from tseg
	   void set_address(header*, int);  	// set sub address in header
	   void set_first_address(int);	    	// set sub addr in first header
	   int	get_hdp();		    				// get current hdp
	   int  first_size();		    			// get first->size
	   void store(int, int);	    			// store int in tseg[int]
	   void set_flags();						  	// set all flags true
	   void set_hed_base_addr(int);		  	// set hed base address
	   void set_base_addr(int);				// set base address
	   void build_dict(FILE *); 				// build dict from name & srec files
	   void compile(int);		    			// compile JSR int in tseg
	   void fix_size();		    				// fix size of colon def
	   void save_tseg();		    				// save tseg to s-record file
	   void	save_headers();	    	    	// save dictionary headers
	};

#endif
