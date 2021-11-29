
#include "stdafx.h"
#include "1712173_1712202_1712292.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		// TODO: code your application's behavior here.
		if (!AfxSocketInit())
		{
			_tprintf(_T("Failed!"));
		}
		else
		{			
			CSocket ProxyServer;
			if (!ProxyServer.Create(PORT_PROXY, SOCK_STREAM, NULL))
			{
				cout << "Can't create PROXY SERVER!" << endl;
			}
			else
			{
				cout << "Create PROXY SERVER succesfully!" << endl;
			}

			//Listen connection from Client
			if (!ProxyServer.Listen())
			{
				cout << "Can't listen on this port!" << endl;
			}
			else
			{
				cout << "Listening on port " << PORT_PROXY << endl;
			}
			while (1)
			{
				CSocket WebClient;
				// Accept Conncetion
				if (!ProxyServer.Accept(WebClient))
					cout << "PROXY Server can't accept connection!" << endl;
				else 
				{
					SOCKET* hConnected = new SOCKET();
					*hConnected = WebClient.Detach();

					DWORD threadID;
					HANDLE threadStatus;
					threadStatus = CreateThread(NULL, 0, Load_WebSite, hConnected, 0, &threadID);
				}
			}
		}
	}

	return nRetCode;
}
