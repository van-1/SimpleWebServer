#ifndef EPOLLSERVER_H
#define EPOLLSERVER_H

// C
#include <unistd.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

// STL
#include <unordered_map>

// Local
#include "sock.h"
#include "handleHttpRequest.h"

#define MAX_EVENTS 32


namespace ServerEpoll
{

    void serverEpoll(int serverSocket)
    {
        int epoll = epoll_create1(0);
        epoll_event event;
        event.data.fd = serverSocket;
        event.events = EPOLLIN;
        epoll_ctl(epoll, EPOLL_CTL_ADD, serverSocket, &event);

        while (true)
        {
            epoll_event events[MAX_EVENTS];
            int n = epoll_wait(epoll, events, MAX_EVENTS, -1);

            for (int i = 0; i < n; i++)
            {
                if (events[i].data.fd == serverSocket)
                {
                    sockaddr addr;
                    socklen_t len = sizeof(addr);
                    char ipstr[INET6_ADDRSTRLEN];
                    memset(ipstr, 0, sizeof(ipstr));

                    int clientSocket = accept(serverSocket, &addr, &len);
                    Sock sTmp(clientSocket);
                    sTmp.setNonblock();
                    epoll_event event;
                    event.data.fd = clientSocket;
                    event.events = EPOLLIN;
                    epoll_ctl(epoll, EPOLL_CTL_ADD, clientSocket, &event);
                    sockaddr_in* s = reinterpret_cast<sockaddr_in*>(&addr);
                    inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
                }
                else
                {
                    char buffer[1024];
                    memset(buffer, 0, sizeof(buffer));
                    Sock client(events[i].data.fd);
                    int recvResult = client.read(buffer, sizeof(buffer));
                    if (recvResult == 0 && errno != EAGAIN)
                    {
                        client.shutdown();
                        client.close();
                    }
                    else if (recvResult > 0)
                    {
                      char* findIndex = strstr(buffer, "GET /index.html");
                      char* findRoot = strstr(buffer, "GET / HTTP/1.0");
                      std::cout << buffer;
                      if (findIndex == nullptr)
                      {
                        if (findRoot != nullptr)
                        {
                          std::string dataToSend(HTTP::response200);
                          dataToSend += HTTP::indexHtml;
                          client.write(dataToSend.c_str(), dataToSend.length());
                        }
                        else
                        {
                          client.write(HTTP::response404.c_str(), HTTP::response404.length());
                        }
                      }
                      else
                      {
                        std::string dataToSend(HTTP::response200);
                        dataToSend += HTTP::indexHtml;
                        client.write(dataToSend.c_str(), dataToSend.length());
                      }
                      client.shutdown();
                      client.close();
                    }
                }
            }
        }
    }
}


#endif // EPOLLSERVER_H
