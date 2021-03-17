//
// Created by youh on 3/10/21.
//

#ifndef SCOOBLE_SEARCH_RAII_H
#define SCOOBLE_SEARCH_RAII_H

#include <functional>

class Defer {
    std::function<void(void)> deferredAction;
public:
    explicit Defer(std::function<void(void)> action) : deferredAction(std::move(action)) {}

    ~Defer() { deferredAction(); }
};

#endif //SCOOBLE_SEARCH_RAII_H
