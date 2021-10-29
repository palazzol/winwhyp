// uart.h
// 8250 UART of PC
// Written by Richard E. Haskell -- October 1994
// Modified December 1996
// Recoded internals to use win32 API, Frank Palazzolo 2021

#ifndef UART_H
#define UART_H

typedef void *HANDLE;

class   Uart
	{
	private:
		HANDLE commHandle;
		int portnum;
	public:
	   Uart();				// constructor
	   ~Uart();				// destructor
	   void transmit_byte(char);			// transmit character
	   bool check_recv(char&);				// get receive char
	   void change_com(int);
	   int get_com();
	   void get_available_coms(std::vector<unsigned int> &list);
	private:
	   void init_term();               	// initialize terminal
	   bool set_com(int port);        	// set COM - true if ok
	   bool setbaud(long int);				// set baud rate - true if ok
	   void enable_com();
	   void disable_com();
	};

#endif  // UART_H
