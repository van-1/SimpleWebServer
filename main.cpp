// STL
#include <iostream>
#include <thread>

// Local
#include "threadpool.h"
#include "sock.h"
#include "epoll.h"
#include "daemonize.h"

// C
#include <getopt.h>
#include <string.h>
#include <stdlib.h>


void clientRead(void* ptr)
{
  int serverFd = *((int*)ptr);
  ServerEpoll::serverEpoll(serverFd);
}


struct globalArgs_t
{
  char* m_host;
  char* m_directory;
  int m_port;
  int daemonize;
};

int main(int argc, char *argv[])
{
  if (argc < 6)
  {
    std::cerr << "usage: " << argv[0] << " -h <ip> -p <port> -d <directory>" << std::endl;
    return 0;
  }

  globalArgs_t globalArgs {nullptr, nullptr, 0, 1};
  static const char* optString = "h:p:d:D:";
  int opt = 0;

  opt = getopt( argc, argv, optString );
  while(opt != -1)
  {
    switch(opt)
    {
      case 'h':
        globalArgs.m_host = optarg;
        break;

      case 'p':
        globalArgs.m_port = atoi(optarg);
        break;

      case 'd':
        globalArgs.m_directory = optarg;
        break;

      case 'D':
      globalArgs.daemonize = atoi(optarg);
      break;

      default:
        break;
    }

    opt = getopt(argc, argv, optString);
  }

  if (globalArgs.daemonize)
    daemonize("/tmp");

  ThreadPool threadsPool (std::thread::hardware_concurrency(), 256);

  /* create a server socket */
  Sock server(AF_INET, SOCK_STREAM);
  if (!server.good())
    return 0;

  if (server.bind(globalArgs.m_host, globalArgs.m_port) < 0)
    return 0;

  if (server.setNonblock() != 0)
  {
    std::cerr << "cannot set for server socket nonblock mode" << std::endl;
    return 0;
  }

  std::cout << "created server socket for: "
            << "host = " << globalArgs.m_host
            << " port = " << globalArgs.m_port
            << " directory = " << globalArgs.m_directory
            << std::endl;

  int serverFd = server.fd();
  for (int i = 0; i < std::thread::hardware_concurrency(); ++i)
    threadsPool.addJob(clientRead, &serverFd);

  threadsPool.wait();
  return 0;
}
