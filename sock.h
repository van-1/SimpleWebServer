#ifndef SOCK_H
#define SOCK_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/utsname.h>
#include <sys/ioctl.h>

class Sock
{
  public:
    Sock(int dom, int type, int protocol=0, bool closeOnDestroy = false)
        : m_domain(dom)
        , m_socktype(type)
        , m_closeOnDestroy(closeOnDestroy)
    {
        if ((m_sid = socket(m_domain, m_socktype, protocol)) < 0)
            perror("socket");
    }

    Sock(int fd, bool closeOnDestroy = false)
      : m_sid(fd)
      , m_domain(AF_INET)
      , m_socktype(SOCK_STREAM)
      , m_closeOnDestroy(closeOnDestroy)
    {}

    ~Sock()
    {
      if (m_closeOnDestroy)
      {
        shutdown();
        ::close(m_sid);
      }
    }

    int fd() {return m_sid;} // return the socket file descriptor
    int good() {return m_sid >= 0;} // check sock object status

    int bind(const char* name, int port=-1)
    {
        struct sockaddr_in addr;
        socklen_t len = constr_name(addr, name, port);
        if ((m_rc = ::bind(m_sid, reinterpret_cast<sockaddr*>(&addr), len)) < 0  ||
                (m_rc = ::getsockname(m_sid, reinterpret_cast<sockaddr*>(&addr), &len)) < 0)
            perror("bind or getsockname");
        else
          std::cout << "Socket port: " << ntohs(addr.sin_port) << std::endl;

        if (m_rc != -1 && m_socktype != SOCK_DGRAM && (m_rc = listen(m_sid, m_BACKLOG_NUM)) < 0)
            perror("listen");
        return m_rc;
    }

    int accept (char* name, int* port_p)
    {
        if (!name) return ::accept(m_sid, nullptr, nullptr);
        struct sockaddr_in addr;
        socklen_t size = sizeof (addr);
        if ((m_rc = ::accept(m_sid, reinterpret_cast<sockaddr*>(&addr), &size)) > -1)
        {
            if (name) strcpy(name, ip2name(addr.sin_addr));
            if (port_p) *port_p = ntohs(addr.sin_port);
        }

        return m_rc;
    }

    int connect(const char* hostnm, int port = -1)
    {
        struct sockaddr_in addr;
        int len = constr_name(addr, hostnm, port);
        if ((m_rc = ::connect(m_sid, reinterpret_cast<sockaddr*>(&addr), len)) < 0) perror("bind");

        return m_rc;
    }

    int write(const char* buf, int len, int flag = MSG_NOSIGNAL)
    {
        return ::send(m_sid, buf, len, flag);
    }

    int read(char* buf, int len, int flag = MSG_NOSIGNAL)
    {
        return ::recv(m_sid, buf, len, flag);
    }

    int sendAll(const char *buf, int len, int flags = MSG_NOSIGNAL)
    {
        int total = 0;
        int sent_bytes = 0;

        while (total < len)
        {
            sent_bytes = write(buf + total, len - total, flags);
            if (sent_bytes == -1) break;
            total += sent_bytes;
        }

        return sent_bytes == -1 ? -1 : total;
    }

    int shutdown(int mode = SHUT_RDWR)
    {
        return ::shutdown(m_sid, mode);
    }

    int close()
    {
      return ::close(m_sid);
    }

    int setNonblock()
    {
        int flags;
    #ifdef O_NONBLOCK
        if( -1 == (flags = fcntl(m_sid, F_GETFL, 0 ))) {
            flags = 0;
        }
        return fcntl(m_sid, F_SETFL, flags | O_NONBLOCK );
    #else
        flags = 1;
        return ioctl(m_sid, FIONBIO, &flags);
    #endif
    }

  private:

    /* Build a socket name based on the given hostname and port number */
    int constr_name(sockaddr_in& addr, const char* hostnm, int port)
    {
        addr.sin_family = m_domain;
        if (!hostnm)
        {
            addr.sin_addr.s_addr = INADDR_ANY;
        }
        else
        {
            hostent *hp = gethostbyname(hostnm);
            memcpy(reinterpret_cast<char*>(&addr.sin_addr), reinterpret_cast<char*>(hp->h_addr), hp->h_length);
        }
        addr.sin_port = htons(port);

        return sizeof(addr);
    }

    /* Convert an IP address to a host name */
    char* ip2name(const in_addr in)
    {
        u_long laddr = 0;
        if ((laddr = inet_addr(inet_ntoa(in))) == -1) return nullptr;
        hostent *hp = gethostbyaddr(reinterpret_cast<char*>(&laddr), sizeof(laddr), AF_INET);
        if (hp == nullptr) return nullptr;
        for (char **p = hp->h_addr_list; *p != nullptr; p++) {
            memcpy((char*)&in.s_addr, *p, sizeof(in.s_addr));
            if (hp->h_name) return hp->h_name;
        }

        return nullptr;
    }

  private:
    //! socket descriptor
    int m_sid;
    //! socket domain
    int m_domain;
    //! socket type
    int m_socktype;
    //! member function return status code
    int m_rc;
    //! close coonection in destructor?
    bool m_closeOnDestroy;
    const int m_BACKLOG_NUM = 10;
};

#endif
