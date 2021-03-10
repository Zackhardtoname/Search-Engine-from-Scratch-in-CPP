//
// Created by youh on 2/20/21.
//

#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>

#include "../String.h"

constexpr size_t MAX_HOSTNAME_LEN = 255;

agr::String AddrToString( const sockaddr_storage &addr, socklen_t addrLen );

void SetTCPNoDelay( int fd );

void SetNonBlocking( int fd );

addrinfo *QueryHost( const char *host, const char *service );

#endif // IO_UTILS_H
