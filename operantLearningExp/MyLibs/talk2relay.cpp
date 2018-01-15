#include "talk2relay.h"


// Include standard libraries
#include <iostream>
#include <cstdio>
#include <ctime>
#include <string>
#include <process.h>


using namespace std;
// control relay to give electric shock
void give_shock(CSerialPort mySerialPort, double duration)
{
	unsigned char pullUp_command = 0x11;
	unsigned char release_command = 0x21;
	//Pull up

	mySerialPort.WriteData(&pullUp_command, 1);
	clock_t startTime = clock();
	clock_t endTime = clock();
	while ((endTime - startTime) / CLOCKS_PER_SEC < duration)
	{
		// hold pulling up
		endTime = clock();
	}
	// release
	mySerialPort.WriteData(&release_command, 1);

}


HANDLE initialize_relay(int com_num)
{
	DWORD dwError;
	wchar_t comName[50];
	char ss[50];
	sprintf_s(ss, "COM%d");
	memset(comName, 0, sizeof(wchar_t)*50);
	MultiByteToWideChar(CP_ACP, 0, ss,-1,comName, 50);
	HANDLE hCom = CreateFile(comName,
		GENERIC_READ | GENERIC_WRITE, 
		0, // not sharing
		NULL, // safety
		OPEN_EXISTING, // the equip has to exist
		0,
		0);
	if (hCom == INVALID_HANDLE_VALUE)
	{
		dwError = GetLastError(); // get the error code
	}
	return hCom;
}

BOOL write_data(HANDLE hCom, unsigned char* pData, int length)
{
	BOOL res = TRUE;
	DWORD  BytesToSend = 0;
	if (hCom == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	// write data
	res = WriteFile(hCom, pData, length, &BytesToSend, NULL);
	if (!res)
	{
		DWORD dwError = GetLastError();
		/** clear serial cache */
		PurgeComm(hCom, PURGE_RXCLEAR | PURGE_RXABORT);
		return false;
	}
	return true;
}

void clear_relay_memory(HANDLE hCom)
{
	PurgeComm(hCom, PURGE_RXCLEAR | PURGE_RXABORT);
}

