// uart.cpp
// member functions for 8250 uart
// Written by Richard E. Haskell -- Nov. 1994/Dec. 1996

#include <vector>

#include "uart.h"
#include "enumser.h"

#include <iostream>
#include <windows.h>

#include <locale>
#include <codecvt>

using namespace std;

// *********************
// uart member functions
// *********************

//	constructor
Uart::Uart()
{
	commHandle = nullptr;
	portnum = 0;

	CEnumerateSerial::CPortsArray ports;
	CEnumerateSerial::UsingQueryDosDevice(ports);

	if ((ports.size() > 0) && (set_com(ports[0])))		// default
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
bool Uart::set_com(int port)
{
	portnum = port;
	return true;
}

//	Get COM port
int Uart::get_com()
{
	return portnum;
}

void Uart::get_available_coms(std::vector<unsigned int> &list)
{
	CEnumerateSerial::UsingQueryDosDevice(list);
}

//	Initialize terminal
void Uart::init_term()
{
}

//	Transmit character
void Uart::transmit_byte(char val)
{
	if (!commHandle)
		return;
	DWORD numWritten;
	WriteFile(commHandle, &val, 1, &numWritten, NULL); 
}

//	Receive byte
bool Uart::check_recv(char& ch)
{
	if (!commHandle)
		return false;

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
	string commPortName = "\\\\.\\COM";
	char buffer[10];
	commPortName += string(_itoa(portnum, buffer, 10));
	wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	wstring wideCommPortName = converter.from_bytes(commPortName);
	int bitRate = 9600;
	commHandle = CreateFile(wideCommPortName.c_str(), GENERIC_READ|GENERIC_WRITE, 0,NULL, OPEN_EXISTING, 
		0, NULL);

	if(commHandle == INVALID_HANDLE_VALUE) 
	{
		portnum = 0;
		commHandle = nullptr;
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
	commHandle = nullptr;
}

//	change COM value
void Uart::change_com(int comval)
{
	disable_com();					// disable current com
	set_com(comval);        		// set new com
	enable_com();           		// enable it
}

