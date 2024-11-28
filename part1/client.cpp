#include <iostream>
#include <cstring>
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

    int clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket < 0)
    {
        cout << "[ERROR] Socket not opened" << endl;
        return 1;
    }
    else
        cout << "[INFO] Socket opened with id=" << clientSocket << endl;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(config["server_port"]);
    serverAddr.sin_addr.s_addr = inet_addr(string(config["server_ip"]).c_str());

    int connectionWithServer = connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (connectionWithServer < 0)
    {
        perror("[ERROR] Connection failed");
        return -1;
    }
    cout << "[INFO] Connection established with server" << endl;

    send(clientSocket, "YOWZA", 5, 0);

    close(clientSocket);

    return 0;
}