#pragma once
#ifndef _GUARD_TALK2RELAY_H
#define _GUARD_TALK2RELAY_H
#include <process.h>  
#include <Windows.h>
#include "../SerialCom/SerialPort.h"



void give_shock(CSerialPort mySerialPort, double duration);

HANDLE initialize_relay(int com_num);
BOOL write_data(HANDLE hCom, unsigned char* pData, int length);
void clear_relay_memory(HANDLE hCom);

#endif // !_GUARD_TALK2RELAY_H