#include "stdafx.h"
#include "1712173_1712202_1712292.h"

#include <algorithm>

//define 403
struct Forbidden403
{
	char *header =
	"HTTP/1.1 403 Forbidden\r\n"
	"Content-Type: text/html; charset utf=8\r\n"
	"Server: Microsoft-IIS/7.0\r\n"
	"Date: Sat, 14 Jan 2012 04:00:08 GMT\r\n"
	"Content-Length: 251\r\n"
	"\r\n"
	"<!DOCTYPE>"
	"<html>"
	"<body>\r\n"
	"<h1>Forbidden</h1>\r\n"
	"<hr>"
	"<p><i>You don't have permission to access on this server!</i></p>"
	"</body>\r\n"
	"</html>\r\n";
};
//------------------------------------------Refer function-----------------------------------------
/*
Get ip from domain name
*/
char *get_ip(char* host)
{
	struct hostent *hent;
	int iplen = 15; //XXX.XXX.XXX.XXX
	char *ip = (char *)malloc(iplen + 1);
	memset(ip, 0, iplen + 1);
	if ((hent = gethostbyname(host)) == NULL)
	{
		perror("Can't get IP");
		exit(1);
	}
	if (inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, iplen) == NULL)
	{
		perror("Can't resolve host");
		exit(1);
	}
	return ip;
}

//Ref: http://stackoverflow.com/questions/19715144/how-to-convert-char-to-lpcwstr
wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

//---------------------------------------------OUR FUNCTION---------------------------------------------

//get domain from host
char *get_host(char *request)
{
	char *str = strstr(request, "Host: ");
	str += 6;
	int host_size = 0;
	while (*str != '\r'&& *str != '\n')
	{
		host_size++;
		++str;
	}
	char *host = (char*)malloc(host_size + 1);
	memset(host, 0, host_size + 1);
	str -= host_size;
	strncpy(host, str, host_size);

	return host;
}

//get header from request
std::string get_header(char* buff, std::string header)
{
	header += ": ";
	std::string request(buff);

	size_t pBegin = request.find(header);
	size_t pEnd = request.find("\r\n", pBegin);

	pBegin += header.size();
	return request.substr(pBegin, pEnd - pBegin);
}

//Socket.Send()
DWORD socketSend(CSocket* socket, char* buffer, int size)
{
	int bytesSent = 0;
	int bytes;

	while (bytesSent < size)
	{
		if ((bytes = socket->Send(&buffer[bytesSent],size - bytesSent)) == SOCKET_ERROR)
		{
			if (GetLastError() == WSAEWOULDBLOCK)
			{
				break;
			}
			else
			{
				return GetLastError();
			}
		}
		else
		{
			bytesSent += bytes;
		}
	}

	return 0;
}

//Check domain in blacklist file
int isBlacklisted(std::string domain)
{
	std::string data;
	std::ifstream file;
	file.open("blacklist.conf", std::ios::in);

	if (!file.is_open())
	{
		std::cout << "Open black list file failed!\n";
		return -1;
	}

	while (!file.eof())
	{
		file >> data;
		if (domain == data)
		{
			return 1;
		}
	}

	file.close();
	return 0;
}
//--------------------------------------------CACHING------------------------------------------------
//Proxy Cache
bool exists_cache(std::string domain) //check doamin is exists
{
	std::string fileCache = domain + ".txt";
	std::ifstream f;
	f.open(fileCache, std::ios::in);
	if (f.fail())
	{
		return false;
	}
	else return true;

	f.close();
}

char *response_cache(char * domain) //get response
{
	char *buff = new char();
	char *filecache = domain + 'txt';
	std::ifstream f;
	f.open(filecache, std::ios::in);
	
	while (!f.eof())
	{
		f >> buff;
	}

	f.close();
	return buff;
}

void caching(char *domain, char *response) // caching conten of website
{
	char *filecache = domain + 'txt';
	std::ofstream fout(filecache,std::ios::out);

	int buff_size = strlen(response);
	while (buff_size != 0)
	{
		fout << response;
		buff_size--;
	}
}

//------------------------------------------LOAD WEBSITE---------------------------------------------
//Load website
DWORD WINAPI Load_WebSite(LPVOID arg)
{
	if (!AfxSocketInit())
	{
		std::cout << "Failed!";
	}
	SOCKET* hConnected = (SOCKET*)arg;
	CSocket WebClient, Proxy;
	WebClient.Attach(*hConnected);

	char buff[MAX_SIZE_BUFFER];
	int cli_recevie = WebClient.Receive(buff, MAX_SIZE_BUFFER);
	
	//Remove protocol that's diferent GET and POST
	std::string temp(buff);
	if (temp.substr(0, 3) != "GET" && temp.substr(0, 4) != "POST")
	{
		return 0;
	}

	//Recevie request
	if (cli_recevie < 0)
	{
		std::cout << "Can't recevie request from client! ERROR:" << GetLastError() << std::endl;
	}

	std::cout << "===>Recevied request! Request's info:" << std::endl;
	std::cout << buff << "\n";

	//Get doamin from Host in request:
	std::string domain = get_header(buff, "Host");
	std::cout << "_Domain name: " << domain << std::endl;
	char *host = get_host(buff);
	char *ip = get_ip(host);
	if (ip == NULL)
	{
		std::cout << "Can't get IP from: " << domain << "\n";
		return 0;
	}
	std::cout << "_IP: " << ip << "\n";

	//check doamin name
	if (isBlacklisted(domain) == -1)
	{
		return 0;
	}
	else if (isBlacklisted(domain)==1)
	{
		//return 403 FORBIDDEN
		Forbidden403 response;
		WebClient.Send(response.header, strlen(response.header));
		std::cout << "403 FORBIDDEN" << std::endl;
	}
	else if (isBlacklisted(domain) == 0)
	{
		//Create Proxy Server
		if (!Proxy.Create())
		{
			std::cout << "Can't create Server! ERROR:" << GetLastError() << std::endl;
			return 0;
		}

		//Connect to web server at port 80
		if (!Proxy.Connect(convertCharArrayToLPCWSTR(ip), PORT_SERVER))
		{
			std::cout << "Can't connect to WEB SERVER! ERROR: " << GetLastError() << std::endl;
			return 0;
		}

		//Send request to WEB SERVER				
		int err = socketSend(&Proxy, buff, cli_recevie);
		if (err < 0)
		{
			std::cout << "Can't send request to WEB SERVER! ERROR:" << err << std::endl;
			return 0;
		}
		//cout << "===>Sent to WEB SERVER successfully!" << endl;

		////////////////////////////////////////////////////
		//Recive response from WEB SERVER

		int proxy_recieved = 0;
		int proxy_size;

		while (1)
		{
			int proxy_recevie = Proxy.Receive(buff, MAX_SIZE_BUFFER);
			//Proxy recevie response to WEB SERVER
			if (proxy_recevie < 0)
			{
				std::cout << "Server send response to Proxy failed! ERROR:" << GetLastError() << std::endl;
			}

			//Web client send content of website for user

			int err = socketSend(&WebClient, buff, proxy_recevie);
			if (err < 0)
			{
				std::cout << "Response to Client failed! ERROR:" << err << std::endl;
			}
			if (proxy_recevie==0)
			{
					break;
			}
		}
		std::cout << "_____LOAD_____WEBSITE______SUCCESSFULLY_____\n";
	}
	
	WebClient.Close();
	Proxy.Close();

	return 0;
}

