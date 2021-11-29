#pragma once

#include "resource.h"

#ifndef MAIN_H_
#define MAIN_H_

#include "stdafx.h"
#include "afxsock.h"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>

//define
#define PORT_PROXY 8888
#define PORT_SERVER 80

#define MAX_SIZE_BUFFER 100000
#define MAX_SIZE_NAME 255

//Get Response from WEB Server _ use thread to handle multiple requests simultaneously
DWORD WINAPI Load_WebSite(LPVOID arg);

#endif // MAIN_H_

