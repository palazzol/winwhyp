// uart.h
// 8250 UART of PC
// Written by Richard E. Haskell -- October 1994
// Modified December 1996
// Recoded internals to use win32 API, Frank Palazzolo 2021

#ifndef UART_H
#define UART_H

enum Com_port {COM1, COM2};

typedef void *HANDLE;

class   Uart
	{
	private:
		HANDLE commHandle;
	public:
	   Uart();				// constructor
	   ~Uart();				// destructor
	   void transmit_byte(char);			// transmit character
	   bool check_recv(char&);				// get receive char
	   void change_com(int);
	private:
	   void init_term();               	// initialize terminal
	   bool set_com(Com_port);        	// set COM - true if ok
	   bool setbaud(long int);				// set baud rate - true if ok
	   void enable_com();
	   void disable_com();
	};

#endif  // UART_H
