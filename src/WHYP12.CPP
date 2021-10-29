// WHYP12.CPP
// Copyright 1997/1998 by Richard E. Haskell
//      WHYP for the 68HC12 family of microcontrollers

#include <iostream>
#include <iomanip>
#include <conio.h>
#include <vector>

#include "uart.h"
#include "srecord.h"
#include "wdict.h"
#include "linklist.h"

#include <windows.h>
#include <synchapi.h>

// create a uart object
Uart uart_8250;

// create a dictionary object
Dict dict(0x16); // JSR opcode = 16

// create an int linklist object
Linklist<int> bstack;             

//      Function prototypes
void    check_COM_ports();
void    do_key(int);
void    do_recv(char, bool&);
int     getkeyq();
void    queryq();
void    host();
void    prompt();
void    doword(char*);
bool 	  in_imed_dict(char*, int&);
void    compile_sub(header*&);
void    download_word();
int     sub_address(header*, bool, int, int);
void    do_imed_word(int);
bool    number(char*, long&, bool&, int&);
void    single_number(int);
void    double_number(long);
int     tpop();
void    tpush(int);
void    tpush2(long);
int     getq();
int     getqword();
void    wait6();
void    dokey(int, int&, bool&);
void    backspace(int&);
void    send_int(int);
void    tblock_store(int, char*, int);
void    forr();
void    next();
void    doo();
void    loop();
void    iff();
void    elsee();
void    thenn();
void    begin();
void    again();
void    until();
void    whilee();
void    repeat();
void    hex();
void    decimal();
void    colon();
void    semis();
void    constant();
void    variable();
void    backslash();
void    paren();
void    load();
void    load_s19_file();
void    load_tseg();
void    create();
void    does();
void    tick();
void    literal();
void    dot_quote();
void    quote();
void    sei();
void    cli();
void    int_colon();
void    rti_semis();
void    comma();
void    ccomma();
void    _dots();
void    _dotr();
void    _dot_quote();
void    _dot_regs();
void    _create();
void    _here();
void	  _vhere();
void	  _allot();
void    _vallot();
void    _tick();
void    _sfile();
void    recurse();
void    ascii();
void    control();
void    read_cfg_file(FILE*);
void    set_int(char*, int);
void    see();
void    show();
void	  step();
void    single_step(unsigned int*);
void    decompile(unsigned int*);


//      Global variables to read in from .CFG file
//      Target subroutine offset addresses
char    NAME_FILE[14];
int     TORG;
int	  HEDBASE;
int	  RAMBASE;
int     INCHAR;
int     OUTPUT;
int     TPUSH;
int     TPOP;
int     INWDY;
int     INWDX;
int     STOREW;
int     TBLKST;
int     SNDSUB;

unsigned int  	tdp;          // target dictionary pointer
unsigned int  	vdp;          // variable dictionary pointer
int     			last;         // address of last
int     			rts_code;  	  // address of RTS in WKE9.S19
bool 				set_flags;    // set flags true for PROM
unsigned int	EESTART;      // EEPROM addresses
unsigned int	EESTOP;

// Other global variables
char    kbuf[81];                       // input keyboard buffer
int     span;                           // no. of char in kbuf
int     to_in;                          // ptr to chars in kbuf
bool    compile;                        // state flag
bool    done = false;                   // set true with esc key or BYE
bool    skip = false;                   // set true with function keys
bool    dwload;                         // download flag
bool    to_tseg = false;                // true when loading to tseg
bool    what_flag = false;              // set true if word not in dict
header* wordhead;                       // ptr to dict header
int     base_addr;                      // base address for tseg
int     sub_addr;                       // sub address from dict
int     this_addr;                      // address of current colon defn
int     num_bytes;                      // no. of sub bytes from dict
char    strbuf[256];                    // string buffer for hc11 data
int     base = 10;                      // current radix - default decimal
char*   here;                           // pointer to parsed word
char*   latest;                         // pointer to colon defn word
char    tokensep[] = " \t\n";           // parse on blank, tab or newline
char	LBEQ[] = "\xec\x31\x18\x27";		 // LDD 2,X+  LBEQ -- --
char	LBRA[] = "\x18\x20";					 // LBRA -- --
const   int nwords = 47;
const   char*   imed_dict[nwords] = {
		"FOR",          // 0  forr()
		"NEXT",         // 1  next()
		"IF",           // 2  iff()
		"ELSE",         // 3  elsee()
		"THEN",         // 4  thenn()
		"BEGIN",        // 5  begin()
		"AGAIN",        // 6  again()
		"UNTIL",        // 7  until()
		"WHILE",        // 8  whilee()
		"REPEAT",       // 9  repeat()
		"HEX",          // 10 hex()
		"DECIMAL",      // 11 decimal()
		":",            // 12 colon()
		";",            // 13 semis()
		"CONSTANT",     // 14 constant()
		"VARIABLE",     // 15 variable()
		"\\",           // 16 backslash()
		"(",            // 17 paren()
		"LOAD",         // 18 load()
		"CREATE",       // 19 create()
		"DOES>",        // 20 does()
		"'",            // 21 tick()
		"[",            // 22 left bracket
		"]",            // 23 right bracket
		"LITERAL",      // 24 literal()
		".\"",			 // 25 dot_quote()
		"SEI",          // 26 sei()
		"CLI",          // 27 cli()
		"INT:",         // 28 int_colon()
		"RTI;",         // 29 rti_semis()
		"BYE",          // 30 bye
		"\n",           // 31 newline  (in files)
		"DO",           // 32 doo()
		"LOOP",         // 33 loop()
		"SAVE.TSEG",    // 34 save_tseg()
		"SAVE.HEADERS", // 35 save_headers()
		"LOAD>TSEG",    // 36 load_tseg()
		",",            // 37 comma()
		"C,",           // 38 ccomma()
		"LOAD.S19.FILE", // 39 load_s19_file()
		"RECURSE",      // 40 recurse()
		"ASCII",        // 41 ascii()
		"CONTROL",      // 42 control()
		"SEE",          // 43 see()
		"SHOW",         // 44 show()
		"STEP",			 // 45 step()
      "\""   			 // 46 quote()
		};

using namespace std;

//	The WHYP main program
int    main()
	{
	FILE * cfgfile;
	// open files for reading
	if((cfgfile = fopen("WHYP12.CFG", "r")) == NULL)
	   {
	   cout << "problem opening .CFG file\n";
	   exit(1);
	   }
	// read .cfg file
	read_cfg_file(cfgfile);
	// close file
	fclose(cfgfile);

	FILE * namefile;
	// open files for reading
	if((namefile = fopen(NAME_FILE, "r")) == NULL)
	   {
	   cout << "problem opening .HED file\n";
	   exit(1);
	   }
	// build dictionary
	dict.set_hed_base_addr(HEDBASE);
	dict.build_dict(namefile);
	if(set_flags)
	   dict.set_flags();                 // set all flags true
	// close files
	fclose(namefile);
	here = new char[81];
	latest = new char[81];
	check_COM_ports();
	cout << endl << "68HC12 WHYP12 - Version 4.6";
	cout << endl << "Press <Esc> or type BYE to exit" << endl;
   //  The outer interpreter
	while(!done)
	   {
	   skip = false;                        // don't skip
	   to_in = 0;                           // reset pointer
	   prompt();                            // display 'ok'
	   queryq();                            // input a line
	   if(done)                             // esc key quits
	      break;
	   if(skip)                             // function keys skip
	      continue;
	   here = strtok(kbuf, tokensep);       // parse 1st word
	   while(here != NULL)                  // while another word
	      {
	      doword(here);                     // process it
	      here = strtok(NULL, tokensep);    // parse next word
	      }
	   }
	}

//  Check for COM1 or COM2
void    check_COM_ports()
	{
	int comport = uart_8250.get_com();
	if (comport == 0) {
		cout << endl << "COM port uninitialized!";
	}
	vector<unsigned int> list;
	uart_8250.get_available_coms(list);
	for ( unsigned int elem : list )
	{
		cout << endl << "Trying COM" << elem << "...";
		uart_8250.change_com(elem); // set new COM port
		char recv_val;
		bool ack = false;
		send_int(rts_code);               // RTS
		Sleep(500);
		if(uart_8250.check_recv(recv_val))
			do_recv(recv_val, ack);
		if(ack) {
			cout << endl << "Communicating with COM" << comport;
			return;
		}
	}
	cout << endl << "Target board not responding" << endl;	
	return;
	}

//  Wait for key press and return generalized ASCII code
//  Check for characters in receive queue
int    getkeyq()
	{
	int keyval, ascii_code, scan_code;
	char recv_val;
	bool ack = false;
	while(!ack)
	   {
	   if(_kbhit())
	      {
	      keyval = _getch();
	      scan_code = keyval >> 8;
	      ascii_code = keyval & 0xff;
	      if(ascii_code == 0) {
			keyval = _getch();
			return keyval | 0x80;
		  }
	      else
		 return ascii_code;
	      }
	   if(uart_8250.check_recv(recv_val))
	      do_recv(recv_val, ack);
	   }
	return 0;
	}

//  Terminal host program entered with F7
void    host()
	{
	int keyval, ascii_code, scan_code;
	char recv_val;
	unsigned char out_byte;
	bool F5key = false;
	while(!F5key)
	   {
	   if(_kbhit())
	      {
	      keyval = _getch();
	      scan_code = keyval >> 8;
	      ascii_code = keyval & 0xff;
	      if(ascii_code == 0) {
			keyval = _getch();
			ascii_code = keyval | 0x80;
		  }
	      if(ascii_code == 0xbf)    // F5 key
		 F5key = true;
	      else
		 {
		 out_byte = ascii_code;
		 uart_8250.transmit_byte(out_byte);
		 }
	      }
	   if(uart_8250.check_recv(recv_val))
	      {
	      if(recv_val == 0x0d)      // CR
		 cout << endl;
	      if(recv_val >= 0x20)
		 cout << recv_val;
	      }
	   }
	cout << endl << "Returning to WHYP" << endl;
	}

 //     Display ok prompt
void    prompt()
	{
	if(!compile)
	   cout << "ok";                // interpret only
	cout << endl;
	}

//      Get a line of input
void    queryq()
	{
	int keyval;
	int ix = 0;
	bool exit_key = false;
	do
	   {
	   keyval = getkeyq();
	   dokey(keyval, ix, exit_key);
	   }
	while(!exit_key);
	}

//      Process key in queryq
void    dokey(int keyval, int& ix, bool& exit_key)
	{
	switch(keyval)
	   {
	   char keybyte;
	   char dp_string[10];
	   char* stop_at;
	   case 0x1b:                   // esc key
	      exit_key = true;
	      done = true;
	      break;
	   case 8:                      // backspace
	      backspace(ix);
	      break;
	   case 13:                     // enter key
	      exit_key = true;
	      kbuf[ix++] = 0;
	      span = ix;
	      cout << " ";
	      break;
      case 0xc1:                   // F7  Host terminal
	      cout << endl << "Terminal host..." << endl;
	      cout << "(Press F5 to return to WHYP)" << endl << endl;
	      host();
	      break;
	   case 0xc2:                   // F8
	      cout << setiosflags(ios::hex);
	      cout << "Current value of VARIABLE vdp is " << vdp;
	      cout << endl << "Enter a new hex value for vdp ("
			   << vdp << "): ";
	      fgets(dp_string, 10, stdin);
	      if(strlen(dp_string) > 0)
		 vdp = strtoul(dp_string, &stop_at, 16);
	      cout << "New value is " << vdp << endl;
	      cout << setiosflags(ios::dec);
	      break;
	   case 0xc3:                   // F9
	      cout << setiosflags(ios::hex);
	      cout << "Current value of target dp is " << tdp;
	      cout << endl << "Enter a new hex value for tdp ("
			   << tdp << "): ";
	      fgets(dp_string, 10, stdin);
	      if(strlen(dp_string) > 0)
		 tdp = strtoul(dp_string, &stop_at, 16);
	      cout << "Value of tdp is " << tdp << endl;
	      cout << setiosflags(ios::dec);
	      break;
	   case 0xc4:                   // F10
      	check_COM_ports();		  // check connection to target
  	      break;
	   default:
	      {
	      if(ix > 79)
		 cout << "\a";          // beep
	      else
		 {
		 keybyte = keyval;
		 kbuf[ix++] = keybyte;
		 cout << keybyte;
		 }
	      }
	   }
	}

//      Handle backspace key
void    backspace(int& ix)
	{
	if(ix == 0)                     // if 1st char
	   cout << "\a";                //    beep
	else
	   {
	   kbuf[--ix] = ' ';            // back up & insert blank
	   cout << "\b \b";             // backspace
	   }
	}

//      Process a word from the input line
void    doword(char* here)
	{
	bool  single;
	int ix, dpl;
	long value;
	what_flag = false;
	if(dict.is_in_dict(here, wordhead, dwload,
			     sub_addr, num_bytes))      					// if in hc12 dict
	   if(compile)                                  	// if compiling
	      dict.compile(sub_address(wordhead, dwload,
			     sub_addr, num_bytes));    					//  compile
	   else
	      {                                         	// else
	      send_int(sub_address(wordhead, dwload,
			     sub_addr, num_bytes));     					//    execute sub
	      wait6();                          				// wait for ack
	      }
	else                                        			// else if immed word
	   if(in_imed_dict(here, ix))
	      do_imed_word(ix);                         	// process it
	   else
	      {
		 if(number(here, value, single, dpl))      		// else if number
		    if(single)
		       single_number(int(value));
		    else                                   		// push on data stack
		       double_number(value);
		 else
		    {
		    cout << endl << here << "  <--What? " << endl;    // else error
		    what_flag = true;
		    }
	      }
	}

//      download word at end of semis
void    download_word()
	{
	int word_addr;
	if(dict.is_in_dict(latest, wordhead, dwload,
			     sub_addr, num_bytes))      // if in hc11 dict
		word_addr = sub_address(wordhead, dwload,
			     sub_addr, num_bytes);    //    download
	}

//      Find subroutine address
int     sub_address(header* wordhead, bool dwload,
		    int sub_addr, int num_bytes)
	{
	int newaddr, taddr;
	newaddr = sub_addr;
	if(!dwload && !to_tseg)         // if word not downloaded & !>tseg
	   {                            //    download it
	   taddr = tdp;
	   dict.get_string(num_bytes, sub_addr, strbuf);
	   tblock_store(num_bytes, strbuf, taddr);
	   dict.set_address(wordhead, taddr);
	   tdp += num_bytes;                    // update tdp
	   newaddr = taddr;
	   }
	return newaddr;
	}

//      Check to see if a word is in the immediate dictionary
bool in_imed_dict(char* here, int& ix)
	{
	int i;
	bool found = false;
	for(i = 0; i < nwords; i++)
	   {
	   if(_stricmp(here, imed_dict[i]) == 0)
	      {
	      found = true;
	      ix = i;
	      break;
	      }
	   }
	return found;
	}

//      Check to see if a word is a valid number
bool number(char* here, long& value, bool& single, int& dpl)
	{
	char* stop_at;
	long val;
	value = strtol(here, &stop_at, base);
	if(*stop_at == NULL)
	   {
	   val = value & 0xffff0000;
	   if(val == 0 || val == 0xffff0000)
	      single = true;
	   else
	      single = false;
	   dpl = 0;
	   return true;
	   }
	else
	   if(*stop_at == 0x2e)	// decimal point
	      {
	      single = false;
	      return true;
	      }
	   else
	      return false;
	}

//      Handle a single number
void    single_number(int value)
	{
	char str[] = "(LIT)";                   				// compile (LIT)
	if(dict.is_in_dict(str, wordhead, dwload,
			     sub_addr, num_bytes))      					// if in hc11 dict
	   if(compile)
	      {                                 				// if compiling
	      dict.compile(sub_address(wordhead, dwload,
			     sub_addr, num_bytes));    					//    compile
	      dict.tcomma(value);                     	// value T,
	      }
	   else
	      tpush(value);                           		// push value on stack
	else
	   cout << endl << "Can't find (LIT) in dictionary" << endl;
	}

void    double_number(long value)
	{
	unsigned int hi_word, lo_word;
	char str[] = "(DLIT)";                  // compile (DLIT)
	if(dict.is_in_dict(str, wordhead, dwload,
			     sub_addr, num_bytes))      					// if in hc11 dict
	   if(compile)
	      {                                 				// if compiling
	      dict.compile(sub_address(wordhead, dwload,
			     sub_addr, num_bytes));    					//    compile
	      hi_word = value >> 16;
	      lo_word = value & 0xffff;
	      dict.tcomma(hi_word);                   	// value T,
	      dict.tcomma(lo_word);                   	// value T,
	      }
	   else
	      tpush2(value);                           		// push value on stack
	else
	   cout << endl << "Can't find (DLIT) in dictionary" << endl;
	}


//      Pop value from target data stack
int     tpop()
	{
	int val;
	val = TORG + TPOP;
	send_int(val);                  				// send TPOP addr
	val = getqword();               				// get word from target
	wait6();                                	// wait for ack
	return val;
	}

//      Push value on target data stack
void    tpush(int value)
	{
	int val;
	val = TORG + TPUSH;
	send_int(val);                  	// send TPUSH addr
	send_int(value);                	// send value
	wait6();                     		// wait for ack
	}

//      Push double number on target data stack
void    tpush2(long value)
	{
	unsigned int hi_word, lo_word;
	hi_word = value >> 16;
	lo_word = value & 0xffff;
	tpush(lo_word);
	tpush(hi_word);
	}

//      Send 16-bit integer to target
void    send_int(int val)
	{
	unsigned char hi_byte, lo_byte;
	hi_byte = val >> 8;
	lo_byte = val & 0xff;
	uart_8250.transmit_byte(lo_byte);
	uart_8250.transmit_byte(hi_byte);
	}

//      Store block of bytes in target
void    tblock_store(int nbytes, char* buf, int taddr)
	{
	unsigned int taddru;
	int val, i;
	val = TORG + TBLKST;
	taddru = taddr;
	send_int(val);                  				// send TBLKST addr
	send_int(nbytes);               				// send count
	send_int(taddr);                				// send target address
	for(i = 0; i < nbytes; i++)
	   {
	   uart_8250.transmit_byte(buf[i]);     	// sent bytes
	   //if(taddru >= EESTART && taddru <= EESTOP)
	      //delay(60);
	   }
	wait6();                                	// wait for ack
	}

//      Get word from queue
int     getqword()
	{
	int hi_byte;
	hi_byte = getq() << 8;          // get high byte
	return hi_byte | getq();        // get low byte
	}

//      Get byte from queue
int    getq()
	{
	int keyval, ascii_code, val;
	char recv_val;
	while(!uart_8250.check_recv(recv_val))
	   {
	   if(_kbhit())
	      {
	      keyval = _getch();
	      ascii_code = keyval & 0xff;
	      if(ascii_code == 0x1b)            	// if esc key
		 break;                         			//   quit
	      }
	   }
	val = recv_val & 0xff;
	return val;
	}

//      Wait for acknowledge (6) from target
void    wait6()
	{
	int val;
	do
	   val = getkeyq();
	while(val != 0 && val != 0x1b);
	}


//      Process an immediate word
void    do_imed_word(int ix)
	{
	switch(ix)
	   {
	   case 0:              // FOR
	      forr();
	      break;
	   case 1:              // NEXT
	      next();
	      break;
	   case 2:              // IF
	      iff();
	      break;
	   case 3:              // ELSE
	      elsee();
	      break;
	   case 4:              // THEN
	      thenn();
	      break;
	   case 5:              // BEGIN
	      begin();
	      break;
	   case 6:              // AGAIN
	      again();
	      break;
	   case 7:              // UNTIL
	      until();
	      break;
	   case 8:              // WHILE
	      whilee();
	      break;
	   case 9:              // REPEAT
	      repeat();
	      break;
	   case 10:             // HEX
	      hex();
	      break;
	   case 11:             // DECIMAL
	      decimal();
	      break;
	   case 12:             // :
	      colon();
	      break;
	   case 13:             // ;
	      semis();
	      break;
	   case 14:             // CONSTANT
	      constant();
	      break;
	   case 15:             // VARIABLE
	      variable();
	      break;
	   case 16:             // backslash
	      backslash();
	      break;
	   case 17:             // (
	      paren();
	      break;
	   case 18:             // LOAD
	      load();
	      break;
	   case 19:             // CREATE
	      create();
	      break;
	   case 20:             // DOES>
	      does();
	      break;
	   case 21:             // ' tick
	      tick();
	      break;
	   case 22:             // [  left bracket
	      compile = false;
	      break;
	   case 23:              // ]  right bracket
	      compile = true;
	      break;
	   case 24:             // LITERAL
	      literal();
	      break;
	   case 25:             // ."
	      dot_quote();
	      break;
	   case 26:             // SEI
	      sei();
	      break;
	   case 27:             // CLI
	      cli();
	      break;
	   case 28:             // INT:
	      int_colon();
	      break;
	   case 29:             // RTI;
	      rti_semis();
	      break;
	   case 30:             // BYE
	      done = true;
	      break;
	   case 31:             // \n  0x0a
	      cout << endl;
	      break;
	   case 32:              // DO
	      doo();
	      break;
	   case 33:              // LOOP
	      loop();
	      break;
	   case 34:              // SAVE.TSEG
	      dict.save_tseg();
	      break;
	   case 35:             // SAVE.HEADERS
	      dict.save_headers();            //   save dict names
	      break;
	   case 36:             // LOAD>TSEG
	      load_tseg();
	      break;
	   case 37:              // ,
	      comma();
	      break;
	   case 38:              // C,
	      ccomma();
	      break;
	   case 39:              // LOAD.S19.FILE
	      load_s19_file();
	      break;
	   case 40:              // RECURSE
	      recurse();
	      break;
	   case 41:              // ASCII
	      ascii();
	      break;
	   case 42:              // CONTROL
	      control();
	      break;
	   case 43:              // SEE
	      see();
	      break;
	   case 44:              // SHOW
	      show();
	      break;
	   case 45:              // STEP
	      step();
	      break;
 	   case 46:              // "
	      quote();
	      break;
	   }
	}

//      FOR...NEXT LOOP
//      FOR     ( n -- )        ( R: -- n )
void    forr()
	{
	char str[] = ">R";                      				// compile >R
	if(dict.is_in_dict(str, wordhead, dwload,
			     sub_addr, num_bytes))      					// if in hc11 dict
	   if(compile)
	      {                                 				// if compiling
	      dict.compile(sub_address(wordhead, dwload,
			     sub_addr, num_bytes));    					//    compile
	      bstack.push(dict.get_hdp());            	// push hdp
	      }
	   else
	      cout << endl << "FOR must be in colon definition" << endl;
	else
	   cout << endl << "Can't find >R in dictionary" << endl;
	}

void    next()
	{
	int displ;
	char str[] = "DONEXT";                  				// compile DONEXT
	char str2[] = "R>DROP";                 				// compile R>DROP
	char donxt_code[] =
		"\xb7\x76\xec\x40\x83\x00\x01\x6c\x40\x26";
	if(dict.is_in_dict(str, wordhead, dwload,
			     sub_addr, num_bytes))      					// if in hc11 dict
	   if(compile)
	      {                                 		  		// if compiling
	      // compile donext inline
	      dict.add_string(10, donxt_code);
	      // compile BNE displacement
	      displ = bstack.pop() - dict.get_hdp()- 1;  // compile displ
	      dict.tccomma(displ);
	      // compile R>DROP
	      if(dict.is_in_dict(str2, wordhead, dwload,
			     sub_addr, num_bytes))      					// if in hc11 dict
		 dict.compile(sub_address(wordhead, dwload,
			     sub_addr, num_bytes));    					//    compile
	      else
		 cout << endl << "Can't find R>DROP in dictionary";
	      }
	   else
	      cout << endl << "FOR must be in colon definition" << endl;
	else
	   cout << endl << "Can't find DONEXT in dictionary" << endl;
	}

//      DO...LOOP
//      DO      ( limit ix -- ) ( R: -- n1 n2 n3 )
void    doo()
	{
	char str[] = "(DO)";                    				// compile (DO)
	if(dict.is_in_dict(str, wordhead, dwload,
			     sub_addr, num_bytes))      					// if in hc11 dict
	   if(compile)
	      {                                 				// if compiling
	      dict.compile(sub_address(wordhead, dwload,
			     sub_addr, num_bytes));    					//    compile
	      bstack.push(dict.get_hdp());            	// push hdp
	      dict.tcomma(0);                         	// 0 T,
	      }
	   else
	      cout << endl << "DO must be in colon definition" << endl;
	else
	   cout << endl << "Can't find (DO) in dictionary" << endl;
	}

//      LOOP    ( -- )
void    loop()
	{
	int displ, xhere1;
	char str[] = "(LOOP)";                  				// compile (LOOP)
	if(dict.is_in_dict(str, wordhead, dwload,
			     sub_addr, num_bytes))      					// if in hc11 dict
	   if(compile)
	      {                                 				// if compiling
	      dict.compile(sub_address(wordhead, dwload,
			     sub_addr, num_bytes));    					//    compile
	      xhere1 = bstack.pop();
	      displ = xhere1 + 2 - dict.get_hdp();
	      dict.tcomma(displ);                    		// complile displ
	      displ = dict.get_hdp() - xhere1;        	// xhere3-xhere1
	      dict.store(displ,xhere1);               	//xhere3->xhere1
	      }
	   else
	      cout << endl << "LOOP must be in colon definition" << endl;
	else
	   cout << endl << "Can't find (LOOP) in dictionary" << endl;
	}


//      IF      ( f -- )
void    iff()
	{
	if(compile)
	  {                                 			// if compiling
	  // compile LDD 2,X+  LBEQ inline
	  dict.add_string(4, LBEQ);
	  bstack.push(dict.get_hdp());            	// push hdp
	  dict.tcomma(0);                         	// displ = 0000
	  }
	else
	      cout << endl << "IF must be in colon definition" << endl;
	}

//      ELSE    ( -- )
void    elsee()
	{
	int displ, xhere1, xhere2;
	if(compile)
	   {                                 // if compiling
	   // compile LBRA inline
	   dict.add_string(2, LBRA);          	// complile LBRA
	   bstack.push(dict.get_hdp());       	// save displ addr
	   dict.tcomma(0);                    	// displ = 0000
	   xhere2 = bstack.pop();               	// ELSE displ addr
	   xhere1 = bstack.pop();               	// IF displ addr
	   displ = xhere2 - xhere1;        	   	// IF displ
	   dict.store(displ,xhere1);          	// store IF displ
	   bstack.push(xhere2);                 	// save ELSE displ addr
	   }
	else
	   cout << endl << "ELSE must be in colon definition" << endl;
	}

//      THEN      ( -- )
void    thenn()
	{
	int displ, xhere12;
	xhere12 = bstack.pop();                  // displ addr
	displ = dict.get_hdp() - xhere12 - 2;  // calc displ
	dict.store(displ,xhere12);             // store displ
	}

//      BEGIN   ( -- )
void    begin()
	{
	bstack.push(dict.get_hdp());          // push hdp
	}

//      AGAIN   ( -- )
void    again()
	{
	int displ, xhere1;
	if(compile)
	   {                                 			// if compiling
	   // compile LBRA inline
	   dict.add_string(2, LBRA);          		// complile LBRA
	   xhere1 = bstack.pop();
	   displ = xhere1 - dict.get_hdp() - 2;
	   dict.tcomma(displ);                    	// complile displ
	   }
	else
	   cout << endl << "AGAIN must be in colon definition" << endl;
	}

void    until()
	{
	int displ, xhere1;
	if(compile)
	      {                                 		// if compiling
	   // compile LDD 2,X+  LBEQ inline
	   dict.add_string(4, LBEQ);
	   xhere1 = bstack.pop();
	   displ = xhere1 - dict.get_hdp() -2;
	   dict.tcomma(displ);                    	// complile displ
	   }
	else
	   cout << endl << "UNTIL must be in colon definition" << endl;
	}

void    whilee()
	{
	int xhere1;
	if(compile)
	   {                                 			// if compiling
	   // compile LDD 2,X+  LBEQ inline
	   dict.add_string(4, LBEQ);
	   xhere1 = bstack.pop();
	   bstack.push(dict.get_hdp());            // push hdp (xhere2)
	   bstack.push(xhere1);                      // SWAP
	   dict.tcomma(0);                         // 0 T,
	   }
	else
	   cout << endl << "WHILE must be in colon definition" << endl;
	}

void    repeat()
	{
	int displ, xhere1, xhere2;
	if(compile)
	   {                                 				// if compiling
	   // compile LBRA inline
	   dict.add_string(2, LBRA);         	    	// complile LBRA
	   xhere1 = bstack.pop();
	   displ = xhere1 - dict.get_hdp() - 2;
	   dict.tcomma(displ);                    		// complile LBRA displ
	   xhere2 = bstack.pop();
	   displ = dict.get_hdp() - xhere2 - 2;   		// displ = xhere3-xhere2
	   dict.store(displ,xhere2);              		// compile LBEQ displ
	   }
	else
	   cout << endl << "REPEAT must be in colon definition" << endl;
	}


//      Change to hex
void    hex()
	{
	base = 16;
	cout << setiosflags(ios::hex);
	}

//      Change to decimal
void    decimal()
	{
	base = 10;
	cout << setiosflags(ios::dec);
	}

//      Colon definition
void    colon()
	{
	int subaddr;
	compile = true;
	// get next word
	here = strtok(NULL, tokensep);
	if(to_tseg)
	   subaddr = dict.get_hdp() + base_addr;
	else
	   subaddr = dict.get_hdp();
	this_addr = subaddr;            // remember for recurse
	strcpy(latest, here);           // remember name
	// make a header
	dict.add_word(here, subaddr);
	}

//      Semi-colon
void    semis()
	{
	compile = false;
	dict.tccomma(0x3d);           // compile RTS
	dict.fix_size();
	download_word();                // download word to HC11
	}

//      CONSTANT        ( n -- )
void    constant()
	{
	char str[] = "(LIT)";
	colon();                                        // make header
	if(dict.is_in_dict(str, wordhead, dwload,     // compile (LIT)
			     sub_addr, num_bytes))      				// if in hc11 dict
	   {
	   dict.compile(sub_address(wordhead, dwload,
			     sub_addr, num_bytes));     				//    compile
	   dict.tcomma(tpop());
	   semis();                                     // RTS
	   }
	else
	   {
	   cout << endl << "Can't find (LIT) in dictionary" << endl;
	   exit(1);
	   }
	}

//      VARIABLE
void    variable()
	{
	char str[] = "(LIT)";
	colon();                                        // make header
	if(dict.is_in_dict(str, wordhead, dwload,     // compile (LIT)
			     sub_addr, num_bytes))      				// if in hc11 dict
	   {
	   dict.compile(sub_address(wordhead, dwload,
			     sub_addr, num_bytes));     				//    compile
	   dict.tcomma(vdp);
	   vdp += 2;                                    // update vdp
	   semis();                                     // RTS
	   }
	else
	   {
	   cout << endl << "Can't find (LIT) in dictionary" << endl;
	   exit(1);
	   }
	}

//      \       Comments
void    backslash()
	{
	do
	   here = strtok(NULL, tokensep);
	while(here != NULL);
	}

//      (       PAREN
void    paren()
	{
	char right_paren[] = ")";
	here = strtok(NULL, right_paren);
	}

//      Load a Forth file
void    load()
	{
	FILE * filename;
	here = strtok(NULL, tokensep);          // get pathname
	if((filename = fopen(here, "r")) == NULL)
	   {
	   cout << "problem opening Forth file\n";
	   exit(1);
	   }
	cout << endl;
	while(fgets(kbuf, 80, filename) != NULL)
	   {
	   cout << kbuf;
	   here = strtok(kbuf, tokensep);
	   while(here != NULL)
	      {
	      doword(here);
	      if(what_flag)
		 {
		 cout << endl << "Press any key to continue ";
		 while( !_kbhit() )
		    { }
		 }
	      here = strtok(NULL, tokensep);
	      }
	   }
	fclose(filename);
	cout << endl << "Current value of target dp is " <<
		hex << tdp << endl << dec;
	}

//      Load a Forth file to tseg
void    load_tseg()
	{
	int baddr;
	FILE * filename;
	here = strtok(NULL, tokensep);          // get pathname
	if((filename = fopen(here, "r")) == NULL)
	   {
	   cout << "problem opening Forth file\n";
	   exit(1);
	   }
	to_tseg = true;
	dict.set_flags();                     // set all flags true
	cout << endl << "Enter base address: ";
	cout << setiosflags(ios::hex);
	cin >> baddr;
	base_addr = baddr;
	dict.set_base_addr(baddr);
	cout << endl;
	while(fgets(kbuf, 80, filename) != NULL)
	   {
	   cout << kbuf;
	   here = strtok(kbuf, tokensep);
	   while(here != NULL)
	      {
	      doword(here);
	      if(what_flag)
		 {
		 cout << endl << "Press any key to continue ";
		 while( !_kbhit() )
		    { }
		 }
	      here = strtok(NULL, tokensep);
	      }
	   }
	to_tseg = false;
	fclose(filename);
	}

//      CREATE
void    create()
	{
	int taddr, nbytes, subaddr;
	char str[] = "(CREATE)";
	char str2[] = "DOVAR";
	if(compile)
	   // compile (CREATE)
	   if(dict.is_in_dict(str, wordhead, dwload,
			     sub_addr, num_bytes))
	      dict.compile(sub_address(wordhead, dwload,
				sub_addr, num_bytes));     //    compile
	   else
	      {
	      cout << endl << "Can't find (CREATE) in dictionary" << endl;
	      exit(1);
	      }
	else
	   {
	   // get next word
	   here = strtok(NULL, tokensep);
	   if(to_tseg)
	      subaddr = dict.get_hdp() + base_addr;
	   else
	      subaddr = dict.get_hdp();
	   // make a header
	   dict.add_word(here, subaddr);
	   // compile DOVAR
	   if(dict.is_in_dict(str2, wordhead, dwload,
			       sub_addr, num_bytes))      // if in hc11 dict
	      {
	      dict.compile(sub_address(wordhead, dwload,
			     sub_addr, num_bytes));       				//    compile
	      dict.fix_size();
	      if(!to_tseg)
		 {
		 taddr = tdp;
		 dict.get_first_string(strbuf);              	// download
		 nbytes = dict.first_size();                 	//  JSR DOVAR
		 tblock_store(nbytes, strbuf, taddr);
		 dict.set_first_address(taddr);
		 tdp += nbytes;
		 }
	      }
	   else
	      {
	      cout << endl << "Can't find (DOVAR) in dictionary" << endl;
	      exit(1);
	      }
	   }
	}

//      DOES>   ( pfa -- )
void    does()
	{
	char str[] = "(;CODE)";
	char str2[] = "DODOES";
	if(compile)
	   {
	   // compile (;CODE)
	   if(dict.is_in_dict(str, wordhead, dwload,
			     sub_addr, num_bytes))
	      dict.compile(sub_address(wordhead, dwload,
				sub_addr, num_bytes));     //    compile
	   else
	      {
	      cout << endl << "Can't find (;CODE) in dictionary" << endl;
	      exit(1);
	      }
	   // compile DODOES
	   if(dict.is_in_dict(str2, wordhead, dwload,
			     sub_addr, num_bytes))
	      dict.compile(sub_address(wordhead, dwload,
				sub_addr, num_bytes));     //    compile
	   else
	      {
	      cout << endl << "Can't find DODOES in dictionary" << endl;
	      exit(1);
	      }
	   }
	else
	      cout << endl << "DOES> must be in colon definition" << endl;
	}

//      '       "tick"
void    tick()
	{
	char str[] = "[']";
	if(compile)
	   {
	   // compile [']
	   if(dict.is_in_dict(str, wordhead, dwload,
			     sub_addr, num_bytes))
	      dict.compile(sub_address(wordhead, dwload,
				sub_addr, num_bytes));     //    compile
	   else
	      {
	      cout << endl << "Can't find ['] in dictionary" << endl;
	      exit(1);
	      }
	   }
	else
	   {
	   // get next word
	   here = strtok(NULL, tokensep);
	   if(dict.is_in_dict(here, wordhead, dwload,
			     sub_addr, num_bytes))
	      tpush(sub_address(wordhead, dwload,
				sub_addr, num_bytes));
	   else
	      cout << " <--What? ";
	   }
	}

//      LITERAL
void    literal()
	{
	char str[] = "(LIT)";
	if(dict.is_in_dict(str, wordhead, dwload,     // compile (LIT)
			     sub_addr, num_bytes))      // if in hc11 dict
	   {
	   dict.compile(sub_address(wordhead, dwload,
			     sub_addr, num_bytes));     //    compile
	   dict.tcomma(tpop());                       // , # on stack
	   }
	else
	   {
	   cout << endl << "Can't find (LIT) in dictionary" << endl;
	   exit(1);
	   }
	}

//      ."	dot_quote
void    dot_quote()
	{
	size_t i;
	size_t len;
	char quote[] = "\"";
	char str[] = "(.\")";
	if(dict.is_in_dict(str, wordhead, dwload,     // compile (.")
			     sub_addr, num_bytes))      				// if in hc12 dict
	   {
	   dict.compile(sub_address(wordhead, dwload,
			     sub_addr, num_bytes));     				//    compile
	   here = strtok(NULL, quote);                  // parse to "
	   len = strlen(here);
	   dict.tccomma((int)len);                         // C, # of char
	   for(i = 0; i < len; i++)
	      dict.tccomma(here[i]);                  // C, chars
	   }
	else
	   {
	   cout << endl << "Can't find (.\") in dictionary" << endl;
	   exit(1);
	   }
	}

   
//      "	quote
void    quote()
	{
	size_t i;
	size_t len;
	char quote[] = "\"";
	if(dict.is_in_dict(quote, wordhead, dwload,     // compile (")
			     sub_addr, num_bytes))      				// if in hc12 dict
	   {
	   dict.compile(sub_address(wordhead, dwload,
			     sub_addr, num_bytes));     				//    compile
	   here = strtok(NULL, quote);                  // parse to "
	   len = strlen(here);
	   dict.tccomma((int)len);                         // C, # of char
	   for(i = 0; i < len; i++)
	      dict.tccomma(here[i]);                  // C, chars
	   }
	else
	   {
	   cout << endl << "Can't find (\") in dictionary" << endl;
	   exit(1);
	   }
	}

//      SEI
void    sei()
	{
	if(compile)
	   dict.tcomma(0x1410);        // compile SEI 1410
	else
	   cout << "SEI must be in colon definition ";
	}

//      CLI
void    cli()
	{
	if(compile)
	   dict.tcomma(0x10ef);        // compile CLI 10ef
	else
	   cout << "CLI must be in colon definition ";
	}

//      INT:
void    int_colon()
	      {
         int int_stack;
         int_stack = RAMBASE + 0x30;
	      colon();               			// make header
			dict.tccomma(0xce);        	// LDX #(RAMBASE+$30)
			dict.tcomma(int_stack);
	      }

//      RTI;
void    rti_semis()
	      {
	      compile = false;
	      dict.tccomma(0x0b);                // compile RTI
	      dict.fix_size();
	      }

//      ,
void    comma()
	{
	char str[] = "(,)";
	if(compile)
	   // compile (,)
	   if(dict.is_in_dict(str, wordhead, dwload,
			     sub_addr, num_bytes))
	      dict.compile(sub_address(wordhead, dwload,
				sub_addr, num_bytes));     //    compile
	   else
	      {
	      cout << endl << "Can't find (,) in dictionary" << endl;
	      exit(1);
	      }
	else
	   if(to_tseg)
	      dict.tcomma(tpop());            		// , in tseg
	   else
	      {
	      if(dict.is_in_dict(str, wordhead, dwload,
			     sub_addr, num_bytes))
		 {
		 send_int(sub_address(wordhead, dwload,
			     sub_addr, num_bytes));     			//    execute sub
		 wait6();                               	// wait for ack
		 }
	      else
		 {
		 cout << endl << "Can't find (,) in dictionary" << endl;
		 exit(1);
		 }
	      }
	}

//      C,
void    ccomma()
	{
	char str[] = "(C,)";
	if(compile)
	   // compile (,)
	   if(dict.is_in_dict(str, wordhead, dwload,
			     sub_addr, num_bytes))
	      dict.compile(sub_address(wordhead, dwload,
				sub_addr, num_bytes));     		//    compile
	   else
	      {
	      cout << endl << "Can't find (C,) in dictionary" << endl;
	      exit(1);
	      }
	else
	   if(to_tseg)
	      dict.tccomma(tpop());           	// , in tseg
	   else
	      {
	      if(dict.is_in_dict(str, wordhead, dwload,
			     sub_addr, num_bytes))
		 {
		 send_int(sub_address(wordhead, dwload,
			     sub_addr, num_bytes));     			//    execute sub
		 wait6();                               	// wait for ack
		 }
	      else
		 {
		 cout << endl << "Can't find (C,) in dictionary" << endl;
		 exit(1);
		 }
	      }
	}

//      Load an .S19 file
void    load_s19_file()
	{
	FILE * filename;
	here = strtok(NULL, tokensep);          		// get pathname
	if((filename = fopen(here, "r")) == NULL)
	   {
	   cout << "problem opening .s19 file\n";
	   exit(1);
	   }
	dict.load(filename);
	}

//      RECURSE
void    recurse()
	{
	int displ;
	int opcode;
	opcode = 0x07;                  					// BSR opcode
	dict.tccomma(opcode);         					// compile BSR
	displ = this_addr - dict.get_hdp() - 1;
	dict.tccomma(displ);          					// compile displacement
	}

//      ASCII
void    ascii()
	{
	char str[] = "(LIT)";
	// get next string
	here = strtok(NULL, tokensep);
	if(compile)
	   {
	   // compile (LIT)
	   if(dict.is_in_dict(str, wordhead, dwload,
			     sub_addr, num_bytes))
	      dict.compile(sub_address(wordhead, dwload,
				sub_addr, num_bytes));     			//    compile
	   else
	      {
	      cout << endl << "Can't find (LIT) in dictionary" << endl;
	      exit(1);
	      }
	   dict.tcomma(here[0]);
	   }
	else
	   tpush(here[0]);
	}

//      CONTROL
void    control()
	{
	char str[] = "(LIT)";
	// get next string
	here = strtok(NULL, tokensep);
	if(compile)
	   {
	   // compile (LIT)
	   if(dict.is_in_dict(str, wordhead, dwload,
			     sub_addr, num_bytes))
	      dict.compile(sub_address(wordhead, dwload,
				sub_addr, num_bytes));     			//    compile
	   else
	      {
	      cout << endl << "Can't find (LIT) in dictionary" << endl;
	      exit(1);
	      }
	   dict.tcomma(here[0] && 31);
	   }
	else
	   tpush(here[0] && 31);
	}

//      SEE <word>    Decompile
void    see()
	{
	int i;
	unsigned int val;
	unsigned int buffer[256];
	// get next word
	here = strtok(NULL, tokensep);
	if(dict.is_in_dict(here, wordhead, dwload,
			sub_addr, num_bytes))
	   {
	   cout << setiosflags(ios::hex | ios::uppercase | ios::right)
		<< setbase(16);
	   tpush(sub_address(wordhead, dwload,
				sub_addr, num_bytes));
	   tpush(num_bytes);
	   send_int(SNDSUB);
	   cout << endl << here << " ";
	   for(i = 0; i < num_bytes; i++)
	      {
	      val = getq();
	      buffer[i] = val;
	      }
	   decompile(buffer);
	   cout << setiosflags(ios::dec | ios::uppercase | ios::right)
		<< setbase(10) << endl;
	   wait6();
	   }
	else
	   cout << " <--What? ";
	}

//      SHOW <word>    show machine code
void    show()
	{
	int i;
	unsigned int val;
	unsigned int buffer[256];
	// get next word
	here = strtok(NULL, tokensep);
	if(dict.is_in_dict(here, wordhead, dwload,
			sub_addr, num_bytes))
	   {
	   cout << setiosflags(ios::hex | ios::uppercase | ios::right)
		<< setbase(16);
	   tpush(sub_address(wordhead, dwload,
				sub_addr, num_bytes));
	   tpush(num_bytes);
	   send_int(SNDSUB);
	   cout << endl << here << " ";
	   for(i = 0; i < num_bytes; i++)
	      {
	      val = getq();
	      buffer[i] = val;
	      if(val == 0x16)
		 cout << endl;
	      cout << setw(3) << buffer[i];
	      }
	   cout << setiosflags(ios::dec | ios::uppercase | ios::right)
		<< setbase(10) << endl;
	   wait6();
	   }
	else
	   cout << " <--What? ";
	}

//      STEP <word>    Single step
void    step()
	{
	int i;
	unsigned int val;
	unsigned int buffer[256];
	// get next word
	here = strtok(NULL, tokensep);
	if(dict.is_in_dict(here, wordhead, dwload,
			sub_addr, num_bytes))
	   {
	   cout << setiosflags(ios::hex | ios::uppercase | ios::right)
		<< setbase(16);
	   tpush(sub_address(wordhead, dwload,
				sub_addr, num_bytes));
	   tpush(num_bytes);
	   send_int(SNDSUB);
	   cout << endl << here << " ";
	   for(i = 0; i < num_bytes; i++)
	      {
	      val = getq();
	      buffer[i] = val;
	      }
	   wait6();
	   decompile(buffer);
	   single_step(buffer);
	   if(base == 10)
	      cout << setiosflags(ios::dec | ios::uppercase | ios::right)
		   << setbase(10);
	   cout << endl;
	   }
	else
	   cout << " <--What? ";
	}

//      *********************************************************
//      Process receive value
//      *********************************************************
void    do_recv(char recv_val, bool &ack)
	{
	switch(recv_val)
	   {
	   case 1:                              // .    dott
         {
	      int val1 = getqword();
	      cout << val1 << " ";              // add blank
	      break;
         }
	   case 2:                              // U.    udott
         {
	      unsigned int val2 = getqword();
	      cout << val2 << " ";              // add blank
	      break;
         }
	   case 3:                              // D.    ddott
         {
	      long val3 = getqword();           // high word
	      long val3l = getqword();          // low word
	      val3l = val3l & 0x0000ffff;		 // mask sign ext
	      val3 = val3 << 16;                // high word
	      val3 = val3 | val3l;         		 // plus low word
	      cout << val3 << " ";              // add blank
	      break;
         }
	   case 4:                              // UD.    uddott
         {
	      unsigned long val4 = getqword();  // high word
	      unsigned long val4l = getqword(); // low word
	      val4l = val4l & 0x0000ffff;		 // mask sign ext
	      val4 = val4 << 16;                // high word
	      val4 = val4 | val4l;         		 // plus low word
	      cout << val4 << " ";              // add blank
	      break;
         }
	   case 5:              				    // EMIT
         {
	      char val5 = getqword();
	      cout << val5;
	      break;
         }
	   case 6:                              // ACK
	      ack = true;
	      break;
	   case 7:                              // .S
	      _dots();                          // display data stack
	      break;
	   case 8:                              // .R
	      _dotr();                          // display return stack
	      break;
	   case 9:                              // ."
	      _dot_quote();                     // display string
	      break;
	   case 10:                             // linefeed
	      break;                            //   let CR do it
	   case 11:                             // ,
	      send_int(tdp);
	      tdp +=2;
	      break;
	   case 12:                             // C,
	      send_int(tdp++);
	      break;
	   case 13:                             // CR
	      cout << endl;
	      break;
	   case 14:                             // .REG
	      _dot_regs();                      // display registers
	      break;
	   case 15:                             // CREATE
	      _create();
	      break;
	   case 16:                             // HERE  ( -- n )
	      _here();
	      break;
	   case 17:                             // '     ( -- cfa )
	      _tick();
	      break;
	   case 18:                             // '     ( -- cfa )
	      _sfile();
	      break;
	   case 19:                             // VHERE  ( -- n )
	      _vhere();
	      break;
	   case 20:                             // ALLOT  ( -- n )
	      _allot();
	      break;
	   case 21:                             // VALLOT  ( -- n )
	      _vallot();
	      break;
 	   case 22:                             // DARK ( -- )
         //clrscr();                         // clear screen
	      break;
 	   case 23:                             // AT ( col row-- )
         {
         int row = getqword();
         int col = getqword();
	      //gotoxy(col, row);                 // set cursor
	      break;
         }
      default:
	      cout << setw(2) << recv_val;
	   }
	}

//      .S      code 7
void    _dots()                 						// display data stack
	{
	int i, highbyte;
	int count = getq();
	highbyte = 0xff00;
	if(count > 127)
	   count = count | highbyte;
	cout << endl << "S:[" << count << "] ";   	// display count
	if(count > 0)
	   {
	   for(i = 0; i < count; i++)
	      cout << getqword() << " ";           	// display stack values
	   }
	}

//      .R      code 8
void    _dotr()          		// display return stack
	{
	int i, highbyte;
	int count = getq();
	highbyte = 0xff00;
	if(count > 127)
	   count = count | highbyte;
	cout << endl << "R:[" << count << "] ";   	// display count
	if(count > 0)
	   {
	   for(i = 0; i < count; i++)
	      cout << getqword() << " ";           	// display stack values
	   }
	}

//      ."	code 9
void    _dot_quote()       	// display string
	{
	int i;
	char ch;
	int count = getq();
	for(i = 0; i < count; i++)
	   {
	   ch = getq();
	   cout << ch;          	// display characters
	   }
	}

//      .REGS   code 14
void    _dot_regs()          	// display registers
	{
	unsigned int i, val;
	cout << setiosflags(ios::hex | ios::uppercase | ios::right)
	     << setbase(16);
	cout << endl << "            SXHINZVC" << endl;
	val = getqword();                       // get SP
	cout << "SP:" << setw(4) << val;
	val = getq();                           // get CCR
	cout << " CCR:";
	for(i = 0; i < 8; i++)
	   {
	   cout << val%2;                       // print CCR in binary
	   val /=2;
	   }
	val = getq();                           // get A
	cout << " A:" << setw(2) << val;
	val = getq();                           // get B
	cout << " B:" << setw(2) << val;
	val = getqword();                       // get X
	cout << " X:" << setw(4) << val;
	val = getqword();                       // get Y
	cout << " Y:" << setw(4) << val << endl;
	val = getqword();                       // get PC
	cout << "PC:" << setw(4) << val;
	val = getq();                           // get code
	cout << "  code:" << setw(2) << val;
	val = getq();                           // get code
	cout << " " << setw(2) << val;
	cout << setiosflags(ios::dec | ios::uppercase | ios::right)
	     << setbase(10) << endl;
	}

//      CREATE
void    _create()
	{
	int taddr, nbytes, last_val;
	char last_str[3];
	char str[] = "DOVAR";
	// get next word
	here = strtok(NULL, tokensep);
	// make a header
	dict.add_word(here, dict.get_hdp());
	// compile DOVAR
	if(dict.is_in_dict(str, wordhead, dwload,
			       sub_addr, num_bytes))      			// if in hc11 dict
	   {
	   dict.compile(sub_address(wordhead, dwload,
			     sub_addr, num_bytes));       			//    compile
	   dict.fix_size();
	   taddr = tdp;
	   dict.get_first_string(strbuf);            	// download
	   nbytes = dict.first_size();            		//  JSR DOVAR
	   tblock_store(nbytes, strbuf, taddr);
	   dict.set_first_address(taddr);
	   last_val = tdp +1;                           // ^DOVAR
	   last_str[0] = last_val >> 8;                 // high byte
	   last_str[1] = last_val & 0xff;               // low byte
	   tblock_store(2, last_str, last);             // set last
	   tdp += nbytes;                               // update tdp
	   send_int(6);                                 // send ack to target
	   }
	else
	      {
	      cout << endl << "Can't find DOVAR in dictionary" << endl;
	      exit(1);
	      }
	}

//      HERE  ( -- n )
void    _here()
	{
	send_int(tdp);
	}

//      VHERE  ( -- n )
void    _vhere()
	{
	send_int(vdp);
	}

//      ALLOT  ( n -- )
void    _allot()
	{
	tdp += getqword();
	}

//      VALLOT  ( n -- )
void    _vallot()
	{
	vdp += getqword();
	}

//      '  ( -- cfa )           "tick"
void    _tick()
	{
	// get next word
	here = strtok(NULL, tokensep);
	if(dict.is_in_dict(here, wordhead, dwload,
			     sub_addr, num_bytes))
	      {
	      tpush(sub_address(wordhead, dwload,
				sub_addr, num_bytes));
	      send_int(6);
	      }
	else
	   cout << " <--What? ";
	}

//      s.file  ( start_addr end_addr -- )
void    _sfile()
	{
	int dest_addr, nbytes, i;
	char filename[80];
	char buff[8192];
	FILE * datfile;
	srecord srec;
	// open file
	cout << endl << "Enter an s19 filename: ";
	cin >> filename;
	if((datfile = fopen(filename,"w")) == NULL)
	   {
	   cout << "problem opening file\n";
	   exit(1);
	   }
  //    cout << endl << "Enter destination hex address for data: ";
  //    cout << setiosflags(ios::hex);
  //    cin >> dest_addr;
	uart_8250.transmit_byte(6);             // send ack
	dest_addr = getqword();
	nbytes = getqword();
	for(i = 0; i < nbytes; i++)
	   {
	   buff[i] = getq();
    //     cout << hex << (int) buff[i] << " ";
	   }
	srec.make_srecord_file(datfile, buff, 0, nbytes-1, dest_addr);
	cout << setiosflags(ios::dec);
	fclose(datfile);
	}

//      read cfg file
void    read_cfg_file(FILE* cfgfile)
	{
	int addr;
	char int_name[16];
	fscanf(cfgfile,"%s", NAME_FILE);
	cout << "Using " << NAME_FILE;
	while(fscanf(cfgfile,"%s%x", int_name, &addr) != EOF)
	   set_int(int_name, addr);
	}

//      set integer values from .cfg file
void    set_int(char* name, int val)
	{
	if(strcmp(name,"TORG") == 0)
	   TORG = val;
	else if(strcmp(name,"HEDBASE") == 0)
	   HEDBASE = val;
	else if(strcmp(name,"RAMBASE") == 0)
	   RAMBASE = val;
	else if(strcmp(name,"INCHAR") == 0)
	   INCHAR = val;
	else if(strcmp(name,"OUTPUT") == 0)
	   OUTPUT = val;
	else if(strcmp(name,"TPUSH") == 0)
	   TPUSH = val;
	else if(strcmp(name,"TPOP") == 0)
	   TPOP = val;
	else if(strcmp(name,"INWDY") == 0)
	   INWDY = val;
	else if(strcmp(name,"INWDX") == 0)
	   INWDX = val;
	else if(strcmp(name,"STOREW") == 0)
	   STOREW = val;
	else if(strcmp(name,"TBLKST") == 0)
	   TBLKST = val;
	else if(strcmp(name,"SNDSUB") == 0)
	   SNDSUB = TORG+val;
	else if(strcmp(name,"EESTART") == 0)
	   EESTART = val;
	else if(strcmp(name,"EESTOP") == 0)
	   EESTOP = val;
	else if(strcmp(name,"tdp") == 0)
	   tdp = val;
	else if(strcmp(name,"vdp") == 0)
	   vdp = val;
	else if(strcmp(name,"last") == 0)
	   last = RAMBASE+val;
	else if(strcmp(name,"rts_code") == 0)
	   rts_code = TORG+val;
	else if(strcmp(name,"set_flags") == 0)
	   set_flags = val;
	else
	   cout << endl << "Error in .CFG file";
	}

//      Decompile subroutine code in buffer
void    decompile(unsigned int* buffer)
	{
	int i, k, addr;
	unsigned int hi_byte;
	char name[33];
	k=0;
	i=0;
	while(buffer[i] != 0x3d)
	   {
	   if(buffer[i] == 0x16)
	      {
	      hi_byte = buffer[i+1] << 8;
	      addr = hi_byte | buffer[i+2];
	      if(dict.find_name(addr, name))
		 {
		 if(k%4 == 0)
		    cout << endl << "\t";
		 cout << " " << name;
		 k++;
		 }
	      else
		 cout << endl << "no name for address " << addr;
	      i += 3;
	      }
	   else if(buffer[i] == 0xb7 && buffer[i+1] == 0x76
			&& buffer[i+2] == 0xec)
		 {
		 cout << " DONEXT";
		 i +=11;
		 }
	   else if(buffer[i] == 0xec && buffer[i+1] == 0x31
		  && buffer[i+2] == 0x18 && buffer[i+3] == 0x27)
		 {
		 cout << " LBEQ";
		 i +=4;
		 cout << setw(3) << buffer[i] <<
			 setw(3) << buffer[i+1];
		 i +=2;
		 }
	   else if(buffer[i] == 0x18 && buffer[i+1] == 0x20)
		 {
		 cout << " LBRA";
		 i +=2;
		 cout << setw(3) << buffer[i] <<
			 setw(3) << buffer[i+1];
		 i +=2;
		 }
	   else if(buffer[i] == 0x07)
		 {
		 cout << " BSR";
		 i +=1;
		 cout << setw(3) << buffer[i];
		 i +=1;
		 }
      else
	      {
	      cout << setw(3) << buffer[i];
	      i++;
	      }
	   }
	cout << " ;";
	}

//      Single step subroutine code in buffer
void    single_step(unsigned int* buffer)
	{
	int i, addr, val, displ;
	int keyval, ascii_code;
	int dots, dotr, plit;
	unsigned int hi_byte;
	char name[33];
	char str[] = ".S";
	if(dict.is_in_dict(str, wordhead, dwload,
			     sub_addr, num_bytes))
	   {
	   dots = sub_address(wordhead, dwload,
				sub_addr, num_bytes);
	   }
	else
	   {
	   cout << endl << ".S not in dictionary ";
	   exit(1);
	   }
	char str0[] = ".R";
	if(dict.is_in_dict(str0, wordhead, dwload,
			     sub_addr, num_bytes))
	   {
	   dotr = sub_address(wordhead, dwload,
				sub_addr, num_bytes);
	   }
	else
	   {
	   cout << endl << ".R not in dictionary ";
	   exit(1);
	   }
	char str1[] = "(LIT)";
	if(dict.is_in_dict(str1, wordhead, dwload,
			     sub_addr, num_bytes))
	   {
	   plit = sub_address(wordhead, dwload,
				sub_addr, num_bytes);
	   }
	else
	   {
	   cout << endl << "(LIT) not in dictionary ";
	   exit(1);
	   }
 	send_int(dots);
	wait6();
	send_int(dotr);
	wait6();
	i=0;
	while(buffer[i] != 0x3d)
	   {
	   if(buffer[i] == 0x16)
	      {
	      hi_byte = buffer[i+1] << 8;
	      addr = hi_byte | buffer[i+2];	// address of sub
	      if(dict.find_name(addr, name))
		     {
		     cout << endl << " " << name << "  ";
		     keyval = _getch();		// wait for key
		     ascii_code = keyval & 0xff;
		 	    if(ascii_code == 0x1b || ascii_code == 'q' || ascii_code == 'Q')
		         break;
		 	  if(addr == plit)			// (LIT)
		    	 {
		       i += 3;
		       hi_byte = buffer[i] << 8;
		       val = hi_byte | buffer[i+1];	// value of lit
		       tpush(val);
		       i += 2;
		       }
           else
		       {
		       send_int(addr);		// execute word
		       wait6();
             i += 3;
		       }
           send_int(dots);
           wait6();
           send_int(dotr);
           wait6();
           }
	      else
		 	  cout << endl << "no name for address " << addr;
	      }
	   else if(buffer[i] == 0x07)		// recurse/BSR
	      {
	      i++;
	      displ = 0xff00 | buffer[i];
	      i++;
	      i += displ;
	      }
	   else if(buffer[i] == 0xec && buffer[i+1] == 0x31
		  		&& buffer[i+2] == 0x18 && buffer[i+3] == 0x27)
      		{
		 		i +=4;
		 		hi_byte = buffer[i] << 8;
		 		val = hi_byte | buffer[i+1];	// value of displ
		 		i += 2;
		 		if(tpop() == 0)
              i += val;
		 		}
	   else if(buffer[i] == 0x18 && buffer[i+1] == 0x20)
		 		{
		 		cout << " LBRA";
		 		i +=2;
		 		hi_byte = buffer[i] << 8;
		 		val = hi_byte | buffer[i+1];	// value of displ
		 		i += 2;
		 		i += val;
		 		}
	   else if(buffer[i] == 0xb7 && buffer[i+1] == 0x76
					&& buffer[i+2] == 0xec)
		 		{
		 		i +=10;
		 		displ = 0xff00 | buffer[i];
		 		i++;
		 		i += displ;
		 		}
	   else
	      i++;
	   }
	cout << endl << "Exit single step";
	}

