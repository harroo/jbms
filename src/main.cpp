
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

bool isdir (std::string s) {

	struct stat st;
	if (stat(s.c_str(), &st) == 0)
		if (st.st_mode & S_IFDIR)
			return true;
	return false;
}

std::string urlDecode(std::string str){
    std::string ret;
    char ch;
    int i, ii, len = str.length();

    for (i=0; i < len; i++){
        if(str[i] != '%'){
            if(str[i] == '+')
                ret += ' ';
            else
                ret += str[i];
        }else{
            sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            ret += ch;
            i = i + 2;
        }
    }
    return ret;
}

std::vector<std::string> split (std::string s, char c) {

	std::vector<std::string> v;
	std::stringstream ss(s);
	std::string t;

	while (std::getline(ss, t, c))
		v.push_back(t);

	return v;
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

		std::cout << "Failed to bind listSock! Attempting again in 2 seconds.." << std::endl;

		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
	std::cout << "Bound successfully!" << std::endl;

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

			page = urlDecode(page);

			std::cout << "Decoded: " << page << std::endl;

			std::string dn;
			if (page == "/") dn = "." + page + "*";
			else {
				page = "." + page;
				dn = page;

				if (!isdir(page)) {

					if (page == "./favicon.ico") {

						const char* homedir;
						if ((homedir = getenv("HOME")) == NULL)
							homedir = getpwuid(getuid())->pw_dir;

						page = homedir;
						page += "/.jbms.favicon.png";
					}

					std::cout << "GETTING: " << page << std::endl;

					if (access(page.c_str(), F_OK) == -1) {

						std::cout << "NOT FOUND!!" << std::endl;

						close(cliSock);
						continue;
					}

					std::ifstream ifs(page.c_str(), std::ios::binary|std::ios::ate);
					std::ifstream::pos_type pos = ifs.tellg();
					int length = pos;
					char *pChars = new char[length];
					ifs.seekg(0, std::ios::beg);
					ifs.read(pChars, length);
					ifs.close();

					send(cliSock, pChars, length, 0);
					close(cliSock);
					continue;

				} else dn += "/*";
			}

			std::cout << dn << ":" << page << std::endl;

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

			resl.push_back("<a href=\""+page+"/..\">..</a><br>");

			for (auto &s : fls) {

				std::cout << s << std::endl;

				//if (isdir(s)) {

					if (s.find(".png") != std::string::npos ||
						s.find(".jpg") != std::string::npos ||
						s.find(".jpeg") != std::string::npos ||
						s.find(".gif") != std::string::npos ||
						s.find(".webp") != std::string::npos
					) {

						resl.push_back("<img src=\""+s+"\" href=\""+s+"\"><br>");

					} else
						resl.push_back("<a href=\""+s+"\">"+s+"</a><br>");
				//	continue;
				//}

				//resl.push_back(s + "<br>");
			}

			resl.push_back("</body></html>");

			std::string rs;
			for (auto &s : resl)
				rs += s;

			send(cliSock, rs.c_str(), rs.size(), 0);

			close(cliSock);

		} else
			close(cliSock);
	}

	return 0;
}
