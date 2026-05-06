#pragma once
#include "Vec2.h"
#include <vector>
#include <string>
#include <atomic>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

class Entity;
class EntityManager;
class PhysicsEngine;

constexpr int  GRID_WIDTH  = 40;
constexpr int  GRID_HEIGHT = 20;
constexpr char EMPTY_CELL  = '.';
constexpr char BORDER_H    = '-';
constexpr char BORDER_V    = '|';

#define SERVER_IP "127.0.0.1"
#define PORT 5555

class Engine {
public:
    static Engine& getInstance();

    Engine(const Engine&)            = delete;
    Engine& operator=(const Engine&) = delete;

    void run();
    void processInput();
    void render();
    void drawGrid(const std::vector<Entity*>& entities);
    void drawHUD();
    void clearScreen();

    bool isRunning() const { return running; }
    void stop()            { running = false; }

    int  getScore()  const { return score; }
    void setScore(int s)   { score = s;    }

private:
    Engine();
    ~Engine();

    void connectToServer();
    void receiveLoop();
    void sendInput(const std::string& action);
    void applyServerState(const std::string& state);

    SOCKET sock;
    std::thread recvThread;
    std::atomic<bool> connected;
    std::atomic<bool> gameStarted;
    std::string myRole = "";
    int score = 0;

    bool running;

    EntityManager* entityManager;

    static Engine* instance;
};

// 6