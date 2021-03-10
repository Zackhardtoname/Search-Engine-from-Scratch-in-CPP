//
// Created by youh on 2/12/21.
//

#ifndef MQ_SOCKETLISTENER_H
#define MQ_SOCKETLISTENER_H

#include <string>
#include "Event.h"
#include "Connection.h"

// Something that listen() and accept() on sockets.
class SocketListener : public EventListener {
    static constexpr int SOCK_QUEUE_SIZE = 20;

    void onEvents(uint32_t events) final;

    static int createSocket(const char *hostname, int port);

    virtual void onNewConnection(int conn) = 0;

protected:
    SocketListener(EventLoop &eventLoop, const char *hostname, int port);
};

#endif //MQ_SOCKETLISTENER_H
