
// Socket_WebClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Socket_WebClient.h"
#include "ProxyServer.h"
#include <thread>
#include <fstream>
#include <string>
#include <vector>
#include <map> /* cache */

// #include "afxsock.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define PROXY_PORT 8888

int main(int argc, char* argv[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			ProxyServer proxy_server(PROXY_PORT);
			proxy_server.run();
			system("pause");
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}

	system("pause");
	return nRetCode;
}



