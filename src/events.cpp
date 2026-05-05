#include "events.h"
#include <algorithm>

// Task 2 - Eman
// EventManager implementation

void EventManager::subscribe(EventListener* l) {
    listeners.push_back(l);
}

void EventManager::unsubscribe(EventListener* l) {
    listeners.erase(
        std::remove(listeners.begin(), listeners.end(), l),
        listeners.end()
    );
}

void EventManager::notify(Event* e) {
    for (auto l : listeners) {
        l->onEvent(e);
    }
}