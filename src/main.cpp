
#include<iostream>
#include<sstream>
#include<fstream>
#include<vector>
#include<cstring>
#include<chrono>
#include<thread>

#include<glob.h>
#include<sys/stat.h>
#include<unistd.h>
#include<pwd.h>

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
	while (bind(listSock, (sockaddr*)&hint, sizeof(hint)) == -1) {
	
		std::cout << "Failed to bind listSock! Attempting again in 5 seconds.." << std::endl;

		std::this_thread::sleep_for(std::chrono::seconds(5));
	}

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

			if (page == "/favicon.ico") {

				const char* homedir;
				if ((homedir = getenv("HOME")) == NULL)
					homedir = getpwuid(getuid())->pw_dir;

				std::string fn = homedir;
				fn += "/.jbms.favicon.png";

				std::cout << fn << std::endl;

				if (access(fn.c_str(), F_OK) == -1) {

					std::cout << "NOT FOUND!!" << std::endl;

					close(cliSock);
					continue;
				}

				std::ifstream ifs(fn.c_str(), std::ios::binary|std::ios::ate);
				std::ifstream::pos_type pos = ifs.tellg();
				int length = pos;
				char *pChars = new char[length];
				ifs.seekg(0, std::ios::beg);
				ifs.read(pChars, length);
				ifs.close();

				send(cliSock, pChars, length, 0);
				close(cliSock);
				continue;
			}

			std::string dn;
			if (page == "/") dn = "." + page + "*";
			else dn = "." + page + "/*";

			glob_t gr;
			glob(dn.c_str(), GLOB_TILDE, NULL, &gr);
			std::vector<std::string> fls;
			for (unsigned int i = 0; i < gr.gl_pathc; ++i)
				fls.push_back(std::string(gr.gl_pathv[i]));
			globfree(&gr);

			std::vector<std::string> resl;
			resl.push_back("<!DOCTYPE html>");
			resl.push_back("<meta charset=\"UTF-8\">");
			resl.push_back("<html><head>");
			resl.push_back("<title>JBMS</title></head>");
			resl.push_back("<body style=\"background-color:Black;color:White;\">");

			for (auto &s : fls) {

				std::cout << s << std::endl;

				struct stat st;
				if (stat(s.c_str(), &st) == 0)
					if (st.st_mode & S_IFDIR) {

						resl.push_back("<a href=\""+s+"\">"+s+"</a><br>");
						continue;
					}
				
				resl.push_back(s + "<br>");
			}

			resl.push_back("</body></html>");

			std::string rs;
			for (auto &s : resl)
				rs += s;

			send(cliSock, rs.c_str(), rs.size(), 0);

			close(cliSock);
		}
	}

	return 0;
}

