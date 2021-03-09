//
// Created by youh on 2/12/21.
//

#include <cassert>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <random>

#include "../../include/io/Timer.h"

Timer::Timer(EventLoop &eventLoop,
             int intervalSec, int intervalNanosec,
             std::function<void(void)> callback)
        : EventListener(eventLoop, timerfd_create(CLOCK_MONOTONIC, 0)),
          callback(std::move(callback)),
          intervalSec(intervalSec), intervalNanosec(intervalNanosec) {

    assert(Fd != -1 && "error in timerfd_create()");

    registerEvents(EPOLLIN);

    Reset();
}

void Timer::onEvents(uint32_t events) {
    assert(events == EPOLLIN && "unexpected event for timerfd");
    char buf[8];
    int n = read(Fd, buf, 8);
    assert(n == 8 && "timerfd not returning 8 bytes");

    callback();
}

void Timer::Reset() {
    itimerspec timerSpec = {
            {intervalSec, intervalNanosec},
            {intervalSec, intervalNanosec}
    };
    int status = timerfd_settime(Fd, 0, &timerSpec, nullptr);
    assert(status != -1 && "error in timerfd_settime()");
}

HeartbeatMonitored::HeartbeatMonitored(EventLoop &eventLoop,
                                       int intervalSec,
                                       int deathThresholdSec)
        : myHeartbeatTimer(eventLoop,
                           intervalSec, random() % 1000000000,
                           [this]() { this->sendHeartbeat(); }),
          clientHeartbeatTimer(eventLoop,
                               deathThresholdSec, random() % 1000000000,
                               [this]() { this->onClientTimeout(); }) {}

void HeartbeatMonitored::onClientActivity() {
    clientHeartbeatTimer.Reset();
}
