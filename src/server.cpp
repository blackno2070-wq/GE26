#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <mutex>
#include <string>
#include <sstream>
#include <chrono>
#include "Entity.h"
#include "EntityManager.h"
#include "PhysicsEngine.h"
#include "EntityFactory.h"
#include "Player.h"
#include "Enemy.h"
#include <cstdlib>
#include <ctime>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define PORT 5555
#define MAX_CLIENTS 2
#define GRID_WIDTH  40
#define GRID_HEIGHT 20

struct ClientInfo {
    SOCKET socket;
    int    id;
    string role;
    bool   connected;
};

ClientInfo clients[2];
int        clientCount = 0;
mutex      clientMutex;
int        score = 0;

// ── Send / Broadcast ───────────────────────────────────────────
void sendToClient(SOCKET sock, const string& msg) {
    string packet = msg + "\n";
    send(sock, packet.c_str(), packet.size(), 0);
}

void broadcast(const string& msg) {
    for (int i = 0; i < 2; i++)
        if (clients[i].connected)
            sendToClient(clients[i].socket, msg);
}

// ── Game State ─────────────────────────────────────────────────
EntityManager* entityManager = nullptr;
PhysicsEngine* physicsEngine = nullptr;
bool           gameRunning   = false;

void initGame() {
    entityManager = new EntityManager();
    physicsEngine = new PhysicsEngine();

    auto p1 = EntityFactory::createEntity("Player", 1, {20, 0}, "Top");
    if (p1) {
        static_cast<Player*>(p1.get())->setAsTopPlayer(true);
        entityManager->addEntity(std::move(p1));
    }

    auto p2 = EntityFactory::createEntity("Player", 2, {20, 19}, "Bottom");
    if (p2) {
        static_cast<Player*>(p2.get())->setAsTopPlayer(false);
        entityManager->addEntity(std::move(p2));
    }

    srand(time(nullptr));
    entityManager->addEntity(EntityFactory::createEntity("Enemy", 3, {rand() % 38 + 1, 1}));
}

// Builds the STATE packet the client's applyServerState() expects:
// STATE:id:symbol:x:y:1;id:symbol:x:y:1:SCORE:N
// Locks clientMutex internally — must be called OUTSIDE any clientMutex lock.
string serializeWorld() {
    string state = "STATE:";
    bool first = true;
    lock_guard<mutex> lock(clientMutex);
    for (auto& e : entityManager->getEntities()) {
        if (!e || !e->isActive()) continue;
        if (!first) state += ";";
        first = false;
        Vec2 pos = e->getPosition();
        state += to_string(e->getId()) + ":"
               + e->display()          + ":"
               + to_string(pos.x)      + ":"
               + to_string(pos.y)      + ":"
               + "1";
    }
    state += ":SCORE:" + to_string(score);
    return state;
}

// ── Input Handler ──────────────────────────────────────────────
// Called from receive threads while clientMutex is already held.
void handleInput(const string& packet) {
    stringstream ss(packet);
    string who, action;
    getline(ss, who,    ':');
    getline(ss, action, ':');

    Player* top    = nullptr;
    Player* bottom = nullptr;
    for (auto& e : entityManager->getEntities()) {
        if (auto* p = dynamic_cast<Player*>(e.get())) {
            if (p->isTop()) top    = p;
            else            bottom = p;
        }
    }

    if (who == "P1" && top) {
        if (action == "MOVE_LEFT")  top->move({-1, 0});
        if (action == "MOVE_RIGHT") top->move({ 1, 0});
        if (action == "SHOOT") {
            auto proj = top->shoot();
            if (proj) entityManager->addEntity(std::move(proj));
        }
    }
    if (who == "P2" && bottom) {
        if (action == "MOVE_LEFT")  bottom->move({-1, 0});
        if (action == "MOVE_RIGHT") bottom->move({ 1, 0});
        if (action == "JUMP")       bottom->setVelocity({0, -5});
    }
}

// ── Per-client receive thread ──────────────────────────────────
void receiveFromClient(int idx) {
    char buffer[1024];
    while (gameRunning && clients[idx].connected) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(clients[idx].socket, buffer, sizeof(buffer), 0);
        if (bytes <= 0) {
            cout << "Player " << clients[idx].id << " disconnected.\n";
            clients[idx].connected = false;
            broadcast("GAMEOVER:DISCONNECT");
            gameRunning = false;
            break;
        }
        string packet(buffer, bytes);
        if (!packet.empty() && packet.back() == '\n')
            packet.pop_back();
        cout << "[P" << clients[idx].id << "] " << packet << "\n";
        lock_guard<mutex> lock(clientMutex);
        handleInput(packet);
    }
}

// ── Server game loop ───────────────────────────────────────────
void gameLoop() {
    initGame();
    gameRunning = true;

    thread t1(receiveFromClient, 0);
    thread t2(receiveFromClient, 1);

    while (gameRunning) {
        {
            lock_guard<mutex> lock(clientMutex);

            // Move all entities + detect/resolve collisions + remove inactive
            entityManager->updateAll(*physicsEngine);

            // Check if any player died
            for (auto& e : entityManager->getEntities()) {
                if (auto* p = dynamic_cast<Player*>(e.get())) {
                    if (!p->isActive()) {
                        broadcast("GAMEOVER:LOSE");
                        gameRunning = false;
                    }
                }
            }

            // Respawn an enemy when all insects are dead — increment score
            bool hasEnemy = false;
            for (auto& e : entityManager->getEntities())
                if (e && e->isActive() && e->getType() == EntityType::INSECT)
                    hasEnemy = true;

            if (!hasEnemy) {
                score++;
                entityManager->addEntity(EntityFactory::createEntity(
                    "Enemy", rand() % 1000 + 10, {rand() % 38 + 1, 1}));
            }
        }
        // Broadcast OUTSIDE the lock — serializeWorld() takes its own lock
        broadcast(serializeWorld());

        this_thread::sleep_for(chrono::milliseconds(150));
    }

    if (t1.joinable()) t1.join();
    if (t2.joinable()) t2.join();

    delete entityManager;
    delete physicsEngine;
}

// ── Main ───────────────────────────────────────────────────────
int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_port        = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));

    listen(serverSocket, MAX_CLIENTS);
    cout << "Server listening on port " << PORT << "...\n";
    cout << "Waiting for 2 players...\n";

    while (clientCount < 2) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) continue;

        clients[clientCount].socket    = clientSocket;
        clients[clientCount].id        = clientCount + 1;
        clients[clientCount].connected = true;
        clients[clientCount].role      = (clientCount == 0) ? "P1" : "P2";

        if (clientCount == 0) {
            sendToClient(clientSocket, "ROLE:P1");
            cout << "Player 1 connected -> P1\n";
        } else {
            sendToClient(clientSocket, "ROLE:P2");
            cout << "Player 2 connected -> P2\n";
            sendToClient(clients[0].socket, "START");
            sendToClient(clients[1].socket, "START");
            cout << "Both connected! Starting game...\n";
        }

        clientCount++;
    }

    gameLoop();

    WSACleanup();
    return 0;
}