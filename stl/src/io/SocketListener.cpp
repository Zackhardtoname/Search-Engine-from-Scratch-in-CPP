//
// Created by youh on 2/12/21.
//

#include <sys/socket.h>
#include <netdb.h>
#include <cassert>
#include <unistd.h>
#include <cstdio>
#include <sys/epoll.h>
#include <memory>
#include "../../include/io/SocketListener.h"
#include "../../include/io/utils.h"

SocketListener::SocketListener(EventLoop &eventLoop, const char *hostname, int port)
        : EventListener(eventLoop, createSocket(hostname, port)) {
    registerEvents(EPOLLIN);
}

void SocketListener::onEvents(uint32_t events) {
    // We only listen for EPOLLIN event.
    sockaddr_storage addr{};
    socklen_t addrLen = sizeof(sockaddr_storage);
    int conn = accept(Fd, reinterpret_cast<sockaddr *>(&addr), &addrLen);
    assert(conn != -1 && "accept() failed");

    SetTCPNoDelay(conn);
    SetNonBlocking(conn);

    onNewConnection(conn);
}

int SocketListener::createSocket(const char *hostname, int port) {
    char portStr[20];
    sprintf(portStr, "%d", port);
    addrinfo *addrResult = QueryHost(hostname, portStr);

    int sock = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    assert(sock != 0 && "error in socket()");

    int yes = 1;
    int status = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    assert(status == 0 && "error in setsockopt()");

    status = bind(sock, addrResult->ai_addr, addrResult->ai_addrlen);
    assert(status == 0 && "error in bind()");

    status = listen(sock, SOCK_QUEUE_SIZE);
    assert(status == 0 && "error in listen()");

    freeaddrinfo(addrResult);

    return sock;
}

