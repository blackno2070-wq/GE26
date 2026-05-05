#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <mutex>
#include <string>
#include <sstream>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define PORT 5555
#define MAX_CLIENTS 2

struct Player {
    SOCKET socket;
    int id;
    string role;
    int x, y;
    bool alive;
    bool connected;
};

int score = 0;
Player players[2];
int playerCount = 0;
mutex clientMutex;

void sendToClient(SOCKET sock, const string& msg) {
    send(sock, msg.c_str(), msg.size(), 0);
}

void broadcast(const string& msg) {
    lock_guard<mutex> lock(clientMutex);
    for (int i = 0; i < 2; i++)
        if (players[i].connected)
            sendToClient(players[i].socket, msg);
}

string serializeAll() {
    string state = "WORLDSTATE:" + to_string(score);
    for (int i = 0; i < 2; i++) {
        state += "|P" + to_string(i + 1) + ":"
               + to_string(players[i].id)   + ":"
               + players[i].role            + ":"
               + to_string(players[i].x)   + ":"
               + to_string(players[i].y)   + ":"
               + (players[i].alive ? "1" : "0");
    }
    return state;
}

void broadcastWorldState() {
    string snapshot = serializeAll();
    for (int i = 0; i < 2; i++)
        if (players[i].connected)
            sendToClient(players[i].socket, snapshot);
}

void handlePacket(const string& packet, int playerIndex) {
    stringstream ss(packet);
    string idStr, action, xStr, yStr;

    getline(ss, idStr, ':');
    getline(ss, action, ':');
    getline(ss, xStr, ':');
    getline(ss, yStr, ':');

    int x = xStr.empty() ? 0 : stoi(xStr);
    int y = yStr.empty() ? 0 : stoi(yStr);

    {
        lock_guard<mutex> lock(clientMutex);

        if (action == "SLIDE") {
            players[playerIndex].x += x;
            if (players[playerIndex].x < 0)  players[playerIndex].x = 0;
            if (players[playerIndex].x > 39) players[playerIndex].x = 39;
        }
        else if (action == "JUMP") {
            players[playerIndex].y -= 3;
            if (players[playerIndex].y < 0) players[playerIndex].y = 0;
        }
        else if (action == "SHOOT") {
            cout << "[P" << playerIndex + 1 << "] SHOOT!\n";
        }
        else if (action == "SCORED") {
            score++;
        }
        else if (action == "DEATH") {
            int victimID = x;
            if (victimID >= 1 && victimID <= 2) {
                players[victimID - 1].alive = false;
                score++;
                cout << "[AUTH] Player " << victimID << " died. Score: " << score << "\n";
            }
        }

        broadcastWorldState();
    }
}

void handleClient(int playerIndex) {
    SOCKET sock = players[playerIndex].socket;
    int playerID = players[playerIndex].id;
    char buffer[1024];

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            cout << "Player " << playerID << " disconnected.\n";
            {
                lock_guard<mutex> lock(clientMutex);
                players[playerIndex].connected = false;
            }
            broadcast("SERVER:PLAYER_LEFT:" + to_string(playerID));
            closesocket(sock);
            break;
        }
        string packet(buffer, bytesReceived);
        cout << "[P" << playerID << "] " << packet << "\n";
        handlePacket(packet, playerIndex);
    }
}

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

    while (playerCount < 2) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) continue;

        lock_guard<mutex> lock(clientMutex);

        players[playerCount].socket    = clientSocket;
        players[playerCount].id        = playerCount + 1;
        players[playerCount].connected = true;
        players[playerCount].alive     = true;
        players[playerCount].x         = 20;
        players[playerCount].y         = (playerCount == 0) ? 0 : 19;

        if (playerCount == 0) {
            players[playerCount].role = "TOP";
            sendToClient(clientSocket, "ROLE:1:TOP");
            cout << "Player 1 connected -> Role: TOP\n";
        } else {
            players[playerCount].role = "BOTTOM";
            sendToClient(clientSocket, "ROLE:2:BOTTOM");
            cout << "Player 2 connected -> Role: BOTTOM\n";
        }

        playerCount++;

        if (playerCount == 2) {
            for (int i = 0; i < 2; i++)
                sendToClient(players[i].socket, "SERVER:START");
            broadcastWorldState();
            cout << "Both players connected! Game starting...\n";
        }
    }

    thread t1(handleClient, 0);
    thread t2(handleClient, 1);

    t1.join();
    t2.join();

    WSACleanup();
    return 0;
}