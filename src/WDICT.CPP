// wdict.cpp
// member functions for WHYP dictionary
// Copyright 1996/1998 by Richard E. Haskell -- All rights reserved.

#include <stdio.h>

#include "srecord.h"
#include "wdict.h"

#include <iostream>
#include <iomanip>

using namespace std;

// ***************************************
// header member functions
// ***************************************

//	constructor for class header
header::header()
	{
	next = NULL;
	name = NULL;
	}

//	destuctor for class header
header::~header()
	{
	if(next)
	   delete next;
	if(name)
	   delete name;
	}

// ***************************************
// linked list dictionary member functions
// ***************************************

//	Constructor for class Dict
Dict::Dict(int jsr)			// constructor
	{
	first = NULL;
	tseg = new char[16000];			// big segment for hc11 code
	hdp = 0;
	base_addr = 0;
	jsr_opcode = jsr;
	}

//	Destructor for class Dict
Dict::~Dict()					// destructor
	{
	delete tseg;
	if(first)
	   delete first;
	}

//	Add a word to the dictionary
void    Dict::add_word(char* wordname, int sub_addr)
	{
	header* newlink = new header;           	// make a new link
	int length = (int)strlen(wordname);				// length of wordname
	newlink->name = new char[length+1];     	// get memory for name
	strcpy(newlink->name,wordname);				// copy wordname to it
	newlink->sub = sub_addr;						// add ptr to hc11 code
	newlink->size = 0;								// set size to 0
	newlink->flag = false;							// set flag to false
	newlink->next = first;                  	// it points to next link
	first = newlink;                        	// now first points to this
	}

//	Load hc11 code words from s19 file
void	Dict::load(FILE * s19file)
	{
	int  checksum, count, val, c, i, j;
	i = hdp;
	do
	   {
	   // look for 'S'
	   do
	      c = getc(s19file);
	   while(c != 'S' && c != EOF);
	   if(c == EOF)
	      {
	      cout << endl << "End of file ";
	      break;
	      }
	   c = getc(s19file);							// get next char
	   if(c == '9')									// if '9'
	      break;										//   exit
	   if(c == '1')                         	// if '1'
	      {
	      checksum = 0;                     	//   load in line
	      count = get_hex_byte(s19file);
	      checksum += count;
	      count -= 3;                       	//   # of bytes
	      checksum += get_hex_byte(s19file);  // read addr
	      checksum += get_hex_byte(s19file);
	      for(j = 0; j < count; j++)
		 {
		 val = get_hex_byte(s19file);
		 checksum += val;
		 tseg[i++] = val;
		 }
	      if(get_hex_byte(s19file) != (~checksum & 0xff))
		 cout << endl << "Checksum error ";
	      }
	   }
	while(true);
	hdp = i;				// update hdp
	}

//	Get 2-character hex byte from file
int	Dict::get_hex_byte(FILE * infile)
	{
	char  str[3];
	char* stop_at;
	unsigned long value;
	int radix = 16;		// hex conversion
	int c;
	c = getc(infile);
	str[0] = c;
	c = getc(infile);
	str[1] = c;
	str[2] = '\0';
	value = strtoul(str, &stop_at, radix);
	return value;
	}

//	Search for word in dictionary
bool	Dict::is_in_dict(char* wordname, header*& wordhead,
		bool& dwload, int& sub_addr, int& num_bytes)
	{
	header* current = first;
	bool found = false;
	while(current != NULL && !found)
	   {
	   if(_stricmp(wordname, current->name) == 0)
	      {
	      found = true;
	      wordhead = current;
	      dwload = current->flag;
	      sub_addr = current->sub;
	      num_bytes = current->size;
	      }
	   current = current->next;
	   }
	return found;
	}

	//	Search for address in dictionary
bool	Dict::find_name(int sub_addr, char* wordname)
	{
	header* current = first;
	bool found = false;
	while(current != NULL && !found)
	   {
	   if(sub_addr == current->sub)
	      {
	      found = true;
	      strcpy(wordname, current->name);
	      }
	   current = current->next;
	   }
	return found;
	}

//	Compile int word in tseg
void	Dict::tcomma(int val)
	{
	int hibyte, lobyte, i;
	i = hdp;
	hibyte = val >> 8;		// Motorola order
	tseg[i++] = hibyte;
	lobyte = val & 0xff;
	tseg[i++] = lobyte;
	hdp = i;						// update hdp
	}

//	Compile byte in tseg
void    Dict::tccomma(int val)
	{
	int i;
	i = hdp;
	tseg[i++] = val;                // compile low byte
	hdp = i;                        // update hdp
	}


//	Add string to tseg
void	Dict::add_string(int nbytes, char* str)
	{
	int i, val;
	for(i = 0; i < nbytes; i++)
	   {
	   val = str[i];
	   tccomma(val);
	   }
	}

//	Get string from tseg
void	Dict::get_string(int nbytes, int start_addr, char* strbuf)
	{
	int i, j;
	j=0;
	for(i = start_addr; i < start_addr+nbytes; i++)
	   strbuf[j++] = tseg[i];
	}

//	Get string from tseg for first word
void	Dict::get_first_string(char* strbuf)
	{
	int i, j;
	j=0;
	for(i = first->sub; i < first->sub+first->size; i++)
	   strbuf[j++] = tseg[i];
	}

//	Set target subroutine address in header
void	Dict::set_address(header* wordhead, int address)
	{
	wordhead->sub = address;
	wordhead->flag = true;
	}

//	Set target subroutine address in first header
void	Dict::set_first_address(int address)
	{
	first->sub = address;
	first->flag = true;
	}

//	Get hdp
int	Dict::get_hdp()
	{
	return hdp;
	}

//	Get first size
int	Dict::first_size()
	{
	return first->size;
	}

//	Store int val at tseg[addr]
void	Dict::store(int val, int addr)
	{
	int hibyte, lobyte;
	hibyte = val >> 8;		// Motorola order
	tseg[addr] = hibyte;
	lobyte = val & 0xff;
	tseg[addr+1] = lobyte;
	}


//	Set flag true for all words in dictionary
void    Dict::set_flags()
	{
	header* current = first ;           		// set ptr to 1st link
	while( current != NULL )
	   {
	   current->flag = true;
	   current = current->next;             	// move to next link
	   }
	}

//	Set hed base address
void 	Dict::set_hed_base_addr(int val)     	// set hed base address
	{
	hed_base_addr = val;
	}


//	Set base address
void 	Dict::set_base_addr(int val)		// set base address
	{
	base_addr = val;
	}


//	Save tseg as s-record file
void	Dict::save_tseg()
	{
	srecord srec;
	int dest_addr;
	char filename[80];
	FILE * datfile;
	// open file
	cout << endl << "Enter an s19 filename for tseg: ";
	cin >> filename;
	if((datfile = fopen(filename,"w")) == NULL)
	   {
	   cout << "problem opening file\n";
	   exit(1);
	   }
	cout << endl << "Enter destination hex address for data: ";
	cout << setiosflags(ios::hex);
	cin >> dest_addr;
	srec.make_srecord_file(datfile, tseg, 0, hdp-1, dest_addr);
	cout << setiosflags(ios::dec);
	fclose(datfile);
	}


//	Build dictionary from hc11 code words
void	Dict::build_dict(FILE * namefile)
	{
	int old_addr;
	char old_name[81];
	fscanf(namefile,"%s%x", old_name, &old_addr);
	while( strcmp(old_name, "END") != 0)
	   {
	   old_addr = hed_base_addr + old_addr;
	   add_word(old_name, old_addr);
	   fscanf(namefile,"%s%x", old_name, &old_addr);
	   }
	}

//      Compile JSR word in tseg
void	Dict::compile(int val)
	{
	tccomma(jsr_opcode);		// compile it
	tcomma(val);			// compile address
	}


//	Fix size of colon definition
void	Dict::fix_size()
	{
	// first points to last word defined
	first->size = hdp + base_addr - first->sub;
	}

//	Save dictionary headers as .sym file
void	Dict::save_headers()
	{
	char* ps;				// ptr to string
	int subaddr;
	header* current = first ;           	// set ptr to 1st link
	char filename[80];
	FILE * datfile;
	// open file
	cout << endl << "Enter a filename for header names: ";
	cin >> filename;
	if((datfile = fopen(filename,"w")) == NULL)
	   {
	   cout << "problem opening file\n";
	   exit(1);
	   }
	while( current != NULL )
	   {
	   ps = current->name;
	   subaddr = current->sub;
	   fprintf(datfile, "\n");
	   while( *ps )										// until null char
	      fprintf(datfile, "%c", *ps++);    		// print characters
	   fprintf(datfile, "\t\t%x",
			subaddr);  										// print sub address
	   current = current->next;             		// move to next link
	   }
	fprintf(datfile, "\nEND\t\t%x",0);
	fclose(datfile);
	}
