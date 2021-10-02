
#include<iostream>
#include<sstream>
#include<fstream>
#include<vector>
#include<cstring>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<arpa/inet.h>

void Abort (std::string a) {

	std::cerr << a << std::endl;

	exit(-1);
}

int main (int argc, char** argv) {

	std::cout << "Welcome to Joe-Bob's Media Server..." << std::endl;
	std::cout << "Starting up! ..." << std::endl;

	int listSock = socket(AF_INET, SOCK_STREAM, 0);
	if (listSock == -1)
		Abort("Failed to define listSock!");

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(80);
	inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);
	if (bind(listSock, (sockaddr*)&hint, sizeof(hint)) == -1)
		Abort("Failed to bind listSock!");

	if (listen(listSock, SOMAXCONN) == -1)
		Abort("Failed to mark listSock for listening!");

	while (true) {

		sockaddr_in cli;
		socklen_t cliSize = sizeof(cli);
		int cliSock = accept(listSock, (sockaddr*)&cli, &cliSize);
		if (cliSock == -1)
			continue;

		char buf[8192];
		memset(buf, 0, 8192);
		int rbc = recv(cliSock, buf, 8192, 0);
		std::string raw = std::string(buf, 0, rbc);

		if (raw.find(" ") != std::string::npos) {

			std::vector<std::string> vls;
			std::stringstream ss(raw);
			std::string s;
			while (std::getline(ss, s, ' '))
				vls.push_back(s);
			std::string page = vls[1];

			std::cout << "Request for page: " << page << std::endl;

			
		}
	}

	return 0;
}

