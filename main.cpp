#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <fstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "threadpool/ThreadPool.hpp"

namespace skyfall
{
	namespace net
	{
		class server
		{
		private:
			int socket_acceptor;

			skyfall::ThreadPool pool;

		public:
			server(uint16_t port) : pool(8)
			{
				struct sockaddr_in address;

				address.sin_family = AF_INET;
				address.sin_addr.s_addr = htonl(INADDR_ANY);
				address.sin_port = htons(port);

				this->socket_acceptor = socket(PF_INET, SOCK_STREAM, 0);

				int enable = 1;
				if (setsockopt(this->socket_acceptor, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    				perror("setsockopt(SO_REUSEADDR) failed");

				if (bind(this->socket_acceptor, (sockaddr*)&address, sizeof(address)) < 0)
				{
					perror("bind failed");
					exit(EXIT_FAILURE);
				}
			}

			~server()
			{
				close(this->socket_acceptor);
			}

		public:
			void execute()
			{
				if (listen(this->socket_acceptor, 128) < 0)
				{
					perror("listen");
					exit(EXIT_FAILURE);
				}

				while (true)
				{
					sockaddr_in address;
					std::size_t address_length;

					int new_connection = accept(this->socket_acceptor, (sockaddr*)&address, (socklen_t*)&address_length);

					this->pool.execute([&](){
						this->handle_connection(new_connection);
					});
				}
			}

		private:
			void handle_connection(int socket)
			{
				std::string request;
				request.resize(4096);

				recv(socket, request.data(), request.size(), 0);

				std::string filename = request.substr(request.find("/")+1, request.find("HTTP")-6);

				std::string file;
				std::ifstream f(filename);
					
				if (f.is_open())
				{
					std::string result, buffer;

					while (std::getline(f, buffer))
						result += buffer + '\n';

					file = result;
				

					std::string answer = 
									"HTTP/1.1 200 OK\r\n"
									"Content-Length: " + std::to_string(file.size()) + "\r\n" +
									"Connection: close\r\n\r\n"
									+ file;

					send(socket, answer.data(), answer.size(), 0);
				}

				close(socket);
			}
		};
	}
}
int main()
{
	skyfall::net::server server(65535);

	server.execute();
}