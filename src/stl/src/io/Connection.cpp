//
// Created by youh on 2/12/21.
//

#include <sys/epoll.h>
#include <sys/socket.h>
#include <cassert>
#include <sys/ioctl.h>
#include "../../include/io/Connection.h"

Connection::Connection(EventLoop &eventLoop, int conn)
        : EventListener(eventLoop, conn) {
    registerEvents(EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET);
}

void Connection::onEvents(uint32_t events) {
    if (events & (EPOLLRDHUP | EPOLLERR | EPOLLHUP)) {
        die();
        return;
    }

    if (events & EPOLLIN) {
        while (true) {
            // Read as many bytes as possible.
            int maxRecvBytes = -1;
            int status = ioctl(Fd, FIONREAD, &maxRecvBytes);
            assert(status != -1 && "error in ioctl()");
            maxRecvBytes = std::max(maxRecvBytes, 0) + 1;

            int start = recvBuf.size();
            recvBuf.resize(start + maxRecvBytes, '\0');
            int n = recv(Fd, recvBuf.data() + start, maxRecvBytes, MSG_DONTWAIT | MSG_NOSIGNAL);
            if (n == -1 && errno != EAGAIN) {
                die();
                return;
            }
            recvBuf.resize(start + n);
            if (n < maxRecvBytes)
                // We have exhausted the buffer.
                break;
        }
        onRecv();
    }

    if (events & EPOLLOUT) {
        writable = true;
        if (!sendBuf.empty()) {
            // Send as many bytes as possible.
            int n = send(Fd, sendBuf.data(), sendBuf.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
            if (n < 0) {
                if (errno == EAGAIN)
                    writable = false;
                else {
                    die();
                    return;
                }
            }
            sendBuf = sendBuf.substr(n);
        }
    }
}

void Connection::sendData(const void *data, size_t size) {
    auto *cdata = static_cast<const char *>(data);
    for (int i = 0; i < size; i++)
        sendBuf.pushBack(cdata[i]);
    if (writable)
        onEvents(EPOLLOUT);
}

ConnectionWithSimpleHandshake::ConnectionWithSimpleHandshake(EventLoop &eventLoop, int conn)
        : Connection(eventLoop, conn) {}

void ConnectionWithSimpleHandshake::onRecv() {
    if (!handshaken) {
        const auto &expected = getExpectedHandshake();
        if (recvBuf.length() < expected.length())
            return;

        if (recvBuf.substr(0, expected.size()) != expected) {
            printf("\nCONN failed.");
            fflush(stdout);
            die();
            return;
        }

        handshaken = true;
        printf("\nCONN ok.");

        const auto &mine = getMyHandshake();
        sendData(mine.data(), mine.size());

        recvBuf = recvBuf.substr(expected.size());

        onHandshakeComplete();
    }

    onPostHandshakeRecv();
}