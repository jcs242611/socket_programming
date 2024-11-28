#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "json.hpp"
using namespace std;

using json = nlohmann::json;

struct sockaddr_in serverAddr;

int main()
{
    char buffer[1024] = {0};

    ifstream config_file("config.json");
    if (!config_file.is_open())
    {
        cerr << "[ERROR] Failed to open config.json" << endl;
        return 1;
    }

    json config;
    try
    {
        config_file >> config;
    }
    catch (const json::parse_error &e)
    {
        cerr << "[ERROR] Error parsing JSON: " << e.what() << endl;
        return 1;
    }

    int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket < 0)
    {
        cout << "[ERROR] Socket not opened" << endl;
        return 1;
    }
    else
        cout << "[INFO] Socket opened with id=" << serverSocket << endl;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(config["server_port"]);
    serverAddr.sin_addr.s_addr = inet_addr(string(config["server_ip"]).c_str());
    memset(&(serverAddr.sin_zero), 0, 8);
    int serverAddrlen = sizeof(serverAddr);

    int bindResult = ::bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (bindResult < 0)
    {
        cout << "[ERROR] Failed to bind" << endl;
        return 1;
    }
    else
        cout << "[INFO] Binding Successfull" << endl;

    int listenResult = listen(serverSocket, 5);
    if (listenResult < 0)
    {
        cout << "[ERROR] Failed to start listening" << endl;
        return 1;
    }
    else
        cout << "[INFO] Listening... on IP:" << string(config["server_ip"]) << " and PORT:" << config["server_port"] << endl;

    int connectionWithClient = accept(serverSocket, (struct sockaddr *)&serverAddr, (socklen_t *)&serverAddrlen);
    if (connectionWithClient < 0)
    {
        perror("[ERROR] Accept failed");
        return -1;
    }
    cout << "[INFO] Connection established with client" << endl;

    int msgVal = recv(connectionWithClient, buffer, 1024, 0);
    if (msgVal < 0)
    {
        perror("[ERROR] No message received");
    }

    cout << "Message from client: " << buffer << endl;

    close(connectionWithClient);
    close(serverSocket);

    return 0;
}