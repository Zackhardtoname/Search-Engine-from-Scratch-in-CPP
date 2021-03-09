//
// Created by youh on 2/12/21.
//

#include <unistd.h>
#include <sys/epoll.h>
#include <cassert>

#include "../../include/io/Event.h"

EventLoop::EventLoop() : epollFd(epoll_create1(0)) {
    assert(epollFd != -1 && "error in epoll_create1()");
}

void EventLoop::registerEvent(EventListener *listener, uint32_t ev) const {
    // Register epoll event.
    epoll_event eventSettings{};
    eventSettings.events = ev;
    eventSettings.data.ptr = listener;
    int status = epoll_ctl(epollFd, EPOLL_CTL_ADD, listener->Fd, &eventSettings);
    assert(status != -1 && "error in epoll_ctl()");
}

void EventLoop::RunForever() {
    epoll_event outEvent{};
    while (epoll_wait(epollFd, &outEvent, 1, -1) != -1) {
        static_cast<EventListener *>(outEvent.data.ptr)->onEvents(outEvent.events);

        for (EventListener *listener: toDelete) {
            delete listener;
        }
        toDelete.clear();
    }

    assert(false && "epoll_wait()");
}

void EventLoop::onListenerDeath(EventListener *listener) {
    int status = epoll_ctl(epollFd, EPOLL_CTL_DEL, listener->Fd, nullptr);
    assert((status != -1 || errno != EBADF) && "error in epoll_ctl()");
    toDelete.insert(listener);
}

EventListener::EventListener(EventLoop &eventLoop, int fd)
        : eventLoop(eventLoop), Fd(fd) {}

EventListener::~EventListener() {
    close(Fd);
}

void EventListener::registerEvents(uint32_t events) {
    eventLoop.registerEvent(this, events);
}

void EventListener::die() {
    eventLoop.onListenerDeath(this);
}


