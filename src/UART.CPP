// uart.cpp
// member functions for 8250 uart
// Written by Richard E. Haskell -- Nov. 1994/Dec. 1996

#include "uart.h"

#include <iostream>
#include <windows.h>

using namespace std;

// *********************
// uart member functions
// *********************

//	constructor
Uart::Uart()
{
	if(set_com(COM1))		// COM1 default
		enable_com();
	else
		cout << endl << "Unable to set COM " << endl;
}

//	destructor
Uart::~Uart()
{
	disable_com();
}

//	Set baud rate
bool Uart::setbaud(long int baud)
{
	// TBD
	return false;
}

//	Set COM port
bool Uart::set_com(Com_port Com_x)
{
	// TBD
	return true;
}

//	Initialize terminal
void Uart::init_term()
{
}

//	Transmit character
void Uart::transmit_byte(char val)
{
	DWORD numWritten;
	WriteFile(commHandle, &val, 1, &numWritten, NULL); 
}

//	Receive byte
bool Uart::check_recv(char& ch)
{
	DWORD numRead;

	BOOL ret = ReadFile(commHandle, &ch, 1, &numRead, NULL);

	if(!numRead)
	{
		return false;
	}

	return true;
}

//	enable new com
void Uart::enable_com()
{
	// TBD - this is hardcoded for now!
	wstring commPortName = L"COM3";
	int bitRate = 9600;
	commHandle = CreateFile(commPortName.c_str(), GENERIC_READ|GENERIC_WRITE, 0,NULL, OPEN_EXISTING, 
		0, NULL);

	if(commHandle == INVALID_HANDLE_VALUE) 
	{
		// TBD - FIXME
		//throw("ERROR: Could not open com port");
	}
	else 
	{
		// set timeouts
		COMMTIMEOUTS cto = { MAXDWORD, 0, 0, 0, 0};
		DCB dcb;
		if(!SetCommTimeouts(commHandle,&cto))
		{
			throw("ERROR: Could not set com port time-outs");
		}

		// set DCB
		memset(&dcb,0,sizeof(dcb));
		dcb.DCBlength = sizeof(dcb);
		dcb.BaudRate = bitRate;
		dcb.fBinary = 1;
		dcb.fDtrControl = DTR_CONTROL_ENABLE;
		dcb.fRtsControl = RTS_CONTROL_ENABLE;

		dcb.Parity = NOPARITY;
		dcb.StopBits = ONESTOPBIT;
		dcb.ByteSize = 8;

		if(!SetCommState(commHandle,&dcb))
		{
			throw("ERROR: Could not set com port parameters");
		}
	}
}

//	disable current com
void  Uart::disable_com()
{
	CloseHandle(commHandle);
}

//	change COM value
void Uart::change_com(int comval)
{
	disable_com();					// disable current com
	set_com((enum Com_port)comval);        		// set new com
	enable_com();           		// enable it
}

