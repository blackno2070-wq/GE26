#pragma once
#include <vector>
#include <string>
#include "Entity.h"

// Task 2 - Eman
// Event System using Observer Pattern

// Base Event class
class Event {
public:
    std::string type;
    Event(std::string t) : type(t) {}
    virtual ~Event() {}
};

// Player Events
class PlayerJoinedEvent : public Event {
public:
    std::string username;
    PlayerJoinedEvent(std::string u) : Event("PlayerJoined"), username(u) {}
};

class PlayerLeftEvent : public Event {
public:
    std::string username;
    PlayerLeftEvent(std::string u) : Event("PlayerLeft"), username(u) {}
};

// Collision Event
class CollisionEvent : public Event {
public:
    Entity* a;
    Entity* b;
    CollisionEvent(Entity* a, Entity* b)
        : Event("Collision"), a(a), b(b) {}
};

// Score Event
class ScoreUpdateEvent : public Event {
public:
    int playerID;
    int newScore;
    ScoreUpdateEvent(int id, int s)
        : Event("ScoreUpdate"), playerID(id), newScore(s) {}
};

// Listener interface
class EventListener {
public:
    virtual void onEvent(Event* e) = 0;
    virtual ~EventListener() {}
};

// EventManager
class EventManager {
private:
    std::vector<EventListener*> listeners;
public:
    void subscribe(EventListener* l);
    void unsubscribe(EventListener* l);
    void notify(Event* e);
};