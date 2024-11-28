#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "json.hpp"
using namespace std;

using json = nlohmann::json;

struct sockaddr_in addr;
fd_set fr, fw, fe;

int main()
{
    ifstream config_file("config.json");
    if (!config_file.is_open())
    {
        cerr << "Failed to open config.json" << endl;
        return 1;
    }

    json config;
    try
    {
        config_file >> config;
    }
    catch (const json::parse_error &e)
    {
        cerr << "Error parsing JSON: " << e.what() << endl;
        return 1;
    }

    int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket < 0)
    {
        cout << "Socket not opened" << endl;
        return 1;
    }
    else
        cout << "Socket opened with id=" << serverSocket << endl;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(config["server_port"]);
    addr.sin_addr.s_addr = inet_addr(string(config["server_ip"]).c_str());
    memset(&(addr.sin_zero), 0, 8);
    int addrlen = sizeof(addr);

    int bindResult = ::bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr));
    if (bindResult < 0)
    {
        cout << "Failed to bind" << endl;
        return 1;
    }
    else
        cout << "Binding Successfull" << endl;

    int listenResult = listen(serverSocket, 5);
    if (listenResult < 0)
    {
        cout << "Failed to start listening" << endl;
        return 1;
    }
    else
        cout << "Listening..." << endl;

    int connectionWithClient = accept(serverSocket, (struct sockaddr *)&addr, (socklen_t *)&addrlen);
    if (connectionWithClient < 0)
    {
        perror("Accept failed");
        return -1;
    }

    std::cout << "Connection established!" << std::endl;

    close(connectionWithClient);
    close(serverSocket);

    return 0;
}