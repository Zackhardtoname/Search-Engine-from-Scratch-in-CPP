//
// Created by youh on 2/12/21.
//

#ifndef MQ_CONNECTION_H
#define MQ_CONNECTION_H

#include "Event.h"
#include "Timer.h"
#include "String.h"

class Connection : public EventListener {
    String sendBuf;
    // If I can write without being blocked.
    bool writable = false;

    // Handler for receiving new data.
    // Return false to indicate that the connection has been closed.
    virtual void onRecv() = 0;

    void onEvents(uint32_t events) final;

protected:
    String recvBuf;

    // Send some data to the client.
    void sendData(const void *data, size_t size);

    Connection(EventLoop &eventLoop, int conn);
};

class ConnectionWithSimpleHandshake : public Connection {
    bool handshaken = false;

    virtual const String & getExpectedHandshake() = 0;

    virtual const String & getMyHandshake() = 0;

    // Return false to signal that
    void onRecv() final;

    virtual void onHandshakeComplete() {}

    virtual void onPostHandshakeRecv() = 0;

protected:

    bool handshakeComplete() { return handshaken; }

public:
    ConnectionWithSimpleHandshake(EventLoop &eventLoop, int conn);
};


#endif //MQ_CONNECTION_H
