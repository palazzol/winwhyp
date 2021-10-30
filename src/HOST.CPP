// HOST.CPP
// Copyright 1998 by Richard E. Haskell
// Terminal host program for downloading .s19 files to EEPROM memory
// Modified 2000 to fix * handshaking (F4) for Motorola M68EVB912B32 board.

#include <vector>

#include "uart.h"

#include <iostream>
#include <conio.h>

// Function prototypes
void    check_COM_port();
void    download_s19();
void    download_s19_star();
bool    send_1_line(FILE * );
int     wait_for_star();

using namespace std;

// create a uart object
Uart uart_8250;

// Global variable
int     COMnum = uart_8250.get_com();

int    main()
	{
	int keyval, ascii_code, scan_code;
	char recv_val;
	unsigned char out_byte;
	bool done = false;

	cout << endl << "Terminal host...9600 baud" << endl;
	if (COMnum == 0)
		cout << "Current COM port is Invalid" << endl;
	else
		cout << "Current COM port is COM" << COMnum << endl;
	cout << "(Press F10 to check/change COM port)" << endl;
	cout << "(Press F6 to download .S19 file)" << endl;
	cout << "(Press F4 to download .S19 file with * handshake)" << endl;
	cout << "(Press <Esc> to exit HOST)" << endl << endl;

	//uart_8250.change_com(COMnum);          // default is COM1
	while(!done)
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
	      if(ascii_code == 0x1b)    		// esc key
		 done = true;
	      else if(ascii_code == 0xbe)   // F4 key
		 download_s19_star();
	      else if(ascii_code == 0xc0)   // F6 key
		 download_s19();
	      else if(ascii_code == 0xc4)   // F10 key
		 check_COM_port();
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
	      if((recv_val >= 0x20) | (recv_val == 0x08))  // BS
		 cout << recv_val;
	      }
	   }
	cout << endl << "Exiting HOST" << endl;
	}

//  Check for COM1 or COM2
void    check_COM_port()
	{
	int COMnum = uart_8250.get_com();
	if (COMnum == 0)
		cout << endl << "Current COM port is Invalid";
	else
		cout << endl << "Current COM port is COM" << COMnum;
	cout << endl << "Available COMS are: ";
	vector<unsigned int> list;
	uart_8250.get_available_coms(list);
	for ( unsigned int elem : list )
	{
		cout << elem << " ";
	}
	cout << endl << "Enter new COM port number: ";
	cin >> COMnum;
	uart_8250.change_com(COMnum); // set new COM port
	COMnum = uart_8250.get_com();
	if (COMnum == 0)
		cout << endl << "Current COM port is Invalid";
	else
		cout << endl << "Current COM port is COM" << COMnum;
	}

void    download_s19_star()
	{
	char filename[80];
	bool eof = false;
	FILE * s19file;
	cout << endl << "Enter .s19 filename to download: ";
	cin >> filename;
	// open file
	if((s19file = fopen(filename,"r")) == NULL)
	   {
	   cout << "problem opening .s19 file\n";
	   return;
	   }
	eof = send_1_line(s19file);             // send 1 line
  	while(!eof)
	   {
	   if(wait_for_star() != 0)             // wait for *
	      break;                            // (exit with key press)
	   else
	      eof = send_1_line(s19file);       // then send 1 line
	   }
 	fclose(s19file);
	cout << endl << "Finished downloading...";     
	}


//  Wait for key press and return generalized ASCII code
//  Wait for * to be received in receive queue
int     wait_for_star()
	{
	int keyval, ascii_code, scan_code;
	char recv_val;
	bool star = false;
	while(!star)
	   {
	   if(_kbhit())
	      {
	      keyval = _getch();
	      scan_code = keyval >> 8;
	      ascii_code = keyval & 0xff;
	      if(ascii_code == 0)
		 return scan_code | 0x80;
	      else
		 return ascii_code;
	      }
	   if(uart_8250.check_recv(recv_val))
	      if(recv_val == 0x2a)      // if *
		 star = true;
	   }
	return 0;
	}

//      send s19 file
bool send_1_line(FILE * s19file)
	{
	char ch;
	int c;
   bool s9 = false;
	// look for 'S'
	do
      {
	   c = getc(s19file);
      ch = c;
      cout << ch ;
      }
	while(ch != 'S' && c != EOF);
	if(c == EOF)
	   {
	   cout << endl << "End of file ";
	   return true;
	   }
	uart_8250.transmit_byte(ch);    	// send S
   c = getc(s19file);           		// get next char
   ch = c;
   cout << ch;
   uart_8250.transmit_byte(ch); 		// send it
   if(ch == '9') 					  		// check to see if it is S9
   	s9 = true;
	do
	   {
	   c = getc(s19file);           	// get next char
	   ch = c;
	   cout << ch;
	   uart_8250.transmit_byte(ch); 	// send it
	   }
	while(ch != '\n' && c != EOF);                       // do entire line
	return s9;
	}

void    download_s19()
	{
	char ch;
	int c;
	char filename[80];
	FILE * s19file;
	cout << endl << "Enter .s19 filename to download: ";
	cin >> filename;
	// open file
	if((s19file = fopen(filename,"r")) == NULL)
	   {
	   cout << "problem opening .s19 file\n";
	   return;
	   }
	cout << endl << "Getting data..." << endl;
	do
	   {
	   c = getc(s19file);           				// get next char
	   ch = c;
	   cout << ch;
	   uart_8250.transmit_byte(ch); 				// send it
	   }
	while(c != EOF);                        	// do entire file
	fclose(s19file);
	cout << endl << "Finished downloading...";
	}
