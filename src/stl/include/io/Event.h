//
// Created by youh on 2/12/21.
//

#ifndef MQ_EVENT_H
#define MQ_EVENT_H

#include <cstdint>
#include <unordered_set>

#include "Vector.h"

class EventListener;

class EventLoop {
    static constexpr int EPOLL_MAX_EVENTS = 10;

    int epollFd;

    std::unordered_set<EventListener*> toDelete;

    void registerEvent(EventListener *listener, uint32_t ev) const;

    void onListenerDeath(EventListener *listener);

    friend class EventListener;

public:
    EventLoop();

    void RunForever();
};

// Something that listen to epoll events.
class EventListener {
    virtual void onEvents(uint32_t events) = 0;

    friend class EventLoop;


protected:
    EventLoop &eventLoop;

    void registerEvents(uint32_t events);

    void die();

    EventListener(EventLoop &eventLoop, int fd);

    virtual ~EventListener();

public:
    const int Fd;
};


#endif //MQ_EVENT_H
