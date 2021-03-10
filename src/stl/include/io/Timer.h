//
// Created by youh on 2/12/21.
//

#ifndef MQ_TIMER_H
#define MQ_TIMER_H

#include <functional>
#include "Event.h"

class Timer : public EventListener {
    const std::function<void(void)> callback;
    const int intervalSec, intervalNanosec;

    void onEvents(uint32_t events) override;

public:
    Timer(EventLoop &eventLoop, int intervalSec, int intervalNanosec, std::function<void(void)> callback);

    void Reset();
};

class HeartbeatMonitored {
    Timer myHeartbeatTimer, clientHeartbeatTimer;

    virtual void onClientTimeout() = 0;

    virtual void sendHeartbeat() = 0;

protected:
    void onClientActivity();

public:
    HeartbeatMonitored(EventLoop &eventLoop, int intervalSec, int deathThresholdSec);
};

#endif //MQ_TIMER_H
