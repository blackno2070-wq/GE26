#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <sstream>
#include <atomic>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define PORT 5555
#define SERVER_IP "127.0.0.1"  // Change to server's IP for LAN

atomic<bool> running(true);
int myPlayerID = 0;
string myRole  = "";

// ─── Task 2: Local mirror of the authoritative world state ────────
// These are updated every time a WORLDSTATE packet arrives.
// The client never modifies score or alive directly — it only reads
// what the server sends.
struct PlayerState {
    int id;
    string role;
    int x, y;
    bool alive;
};

int    worldScore = 0;
PlayerState worldPlayers[2];

// ─── Task 2: Parse a WORLDSTATE packet ───────────────────────────
// Format: WORLDSTATE:score|P1:id:role:x:y:alive|P2:id:role:x:y:alive
//
// Splits on '|' first, then on ':' for each player block.
// After parsing, worldScore and worldPlayers[] hold the server's truth.
void applyWorldState(const string& msg) {
    // Strip the "WORLDSTATE:" prefix
    string body = msg.substr(11);   // length of "WORLDSTATE:" == 11

    stringstream topLevel(body);
    string scoreStr, p1Block, p2Block;
    getline(topLevel, scoreStr, '|');
    getline(topLevel, p1Block,  '|');
    getline(topLevel, p2Block,  '|');

    worldScore = stoi(scoreStr);

    // Parse a single player block: "P1:id:role:x:y:alive"
    auto parsePlayer = [](const string& block, PlayerState& out) {
        // Skip the "P1" or "P2" label token
        stringstream ss(block);
        string label, id, role, x, y, alive;
        getline(ss, label, ':');   // "P1" / "P2" — discarded
        getline(ss, id,    ':');
        getline(ss, role,  ':');
        getline(ss, x,     ':');
        getline(ss, y,     ':');
        getline(ss, alive, ':');

        out.id    = stoi(id);
        out.role  = role;
        out.x     = stoi(x);
        out.y     = stoi(y);
        out.alive = (alive == "1");
    };

    parsePlayer(p1Block, worldPlayers[0]);
    parsePlayer(p2Block, worldPlayers[1]);

    // Display the updated snapshot
    cout << "\n[WORLD] Score: " << worldScore
         << " | P1(" << worldPlayers[0].role << ") x=" << worldPlayers[0].x
         << " y=" << worldPlayers[0].y
         << " alive=" << worldPlayers[0].alive
         << " | P2(" << worldPlayers[1].role << ") x=" << worldPlayers[1].x
         << " y=" << worldPlayers[1].y
         << " alive=" << worldPlayers[1].alive
         << "\n> ";
}

// ─── Handle incoming messages from server ────────────────────────
void receiveMessages(SOCKET sock) {
    char buffer[2048];   // bumped up for WORLDSTATE packets
    while (running) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            cout << "\n[Disconnected from server.]\n";
            running = false;
            break;
        }

        string msg(buffer, bytesReceived);

        // Role assignment from server
        if (msg.substr(0, 5) == "ROLE:") {
            myPlayerID = msg[5] - '0';
            myRole     = msg.substr(7);
            cout << "\n You are Player " << myPlayerID << " | Role: " << myRole << "\n";
            if (myRole == "TOP")
                cout << "Controls: A/D to slide | F to shoot\n";
            else
                cout << "Controls: SPACE to jump | K to report kill\n";
        }
        // Game start signal
        else if (msg == "SERVER:START") {
            cout << "\n Both players connected! Game is starting...\n> ";
        }
        // ── Task 2: Full world-state snapshot from server ──────────────
        // This is the primary sync mechanism.  Every action the server
        // processes triggers a WORLDSTATE broadcast so both clients
        // always hold the same authoritative picture.
        else if (msg.substr(0, 11) == "WORLDSTATE:") {
            applyWorldState(msg);
        }
        // Legacy per-event score update (kept for compatibility)
        else if (msg.substr(0, 6) == "SCORE:") {
            worldScore = stoi(msg.substr(6));
            cout << "\n Score: " << worldScore << "\n> ";
        }
        // Other player left
        else if (msg.substr(0, 19) == "SERVER:PLAYER_LEFT:") {
            cout << "\n Other player disconnected.\n";
            running = false;
        }
        // Per-action state event (used to trigger animations / local effects)
        else if (msg.substr(0, 6) == "STATE:") {
            cout << "\n[ACTION] " << msg << "\n> ";
        }

        cout.flush();
    }
}

// ─── Send packet to server ────────────────────────────────────────
void sendPacket(SOCKET sock, const string& action, int x, int y) {
    // Format: PlayerID:Action:X:Y
    string packet = to_string(myPlayerID) + ":" + action + ":"
                  + to_string(x) + ":" + to_string(y);
    send(sock, packet.c_str(), packet.size(), 0);
}

// ─── Main ─────────────────────────────────────────────────────────
int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port   = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Could not connect to server.\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    cout << "Connected to server at " << SERVER_IP << ":" << PORT << "\n";
    cout << "Waiting for role assignment...\n";

    thread recvThread(receiveMessages, sock);

    // Input loop
    while (running) {
        cout << "> ";
        string input;
        getline(cin, input);

        if (input == "QUIT") {
            running = false;
            break;
        }
        if (input.empty()) continue;

        // Player 1 (TOP) controls
        if (myRole == "TOP") {
            if (input == "A")       sendPacket(sock, "SLIDE", -1, 0);
            else if (input == "D")  sendPacket(sock, "SLIDE",  1, 0);
            else if (input == "F")  sendPacket(sock, "SHOOT",  0, 0);
        }
        // Player 2 (BOTTOM) controls
        else if (myRole == "BOTTOM") {
            if (input == "SPACE")   sendPacket(sock, "JUMP", 0, 1);
        }

        // Score a point (generic, server will authorise)
        if (input == "SCORED")  sendPacket(sock, "SCORED", 0, 0);

        // ── Task 2: DEATH report ──────────────────────────────────────
        // Client detects a kill and sends DEATH:<victimID> to the server.
        // The server validates it, marks the victim dead, increments the
        // authoritative score, and broadcasts a fresh WORLDSTATE to both
        // clients.  The client never marks anyone dead on its own.
        //
        // Usage: type  KILL1  to report Player 1 died
        //              KILL2  to report Player 2 died
        if (input == "KILL1")   sendPacket(sock, "DEATH", 1, 0);
        if (input == "KILL2")   sendPacket(sock, "DEATH", 2, 0);
    }

    closesocket(sock);
    WSACleanup();
    if (recvThread.joinable())
        recvThread.join();

    cout << "Disconnected.\n";
    return 0;
}
