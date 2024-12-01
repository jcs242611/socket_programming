#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "json.hpp"
using namespace std;

using json = nlohmann::json;

struct sockaddr_in serverAddr;

void closeConnection(int room, int reception, int error = 0)
{
    close(room);
    close(reception);
    cout << (error ? "[SERVER | ERROR] " : "[SERVER | END] ") << "Connection ended with client" << endl;
}

int main(int argc, char *argv[])
{
    ifstream config_file("config.json");
    if (!config_file.is_open())
    {
        cerr << "[SERVER | ERROR] Failed to open config.json" << endl;
        return 1;
    }

    json config;
    try
    {
        config_file >> config;
    }
    catch (const json::parse_error &e)
    {
        cerr << "[SERVER | ERROR] Error parsing JSON: " << e.what() << endl;
        return 1;
    }

    int p = config["p"];
    if (argc == 2)
    {
        try
        {
            p = stoi(argv[1]);
        }
        catch (const invalid_argument &e)
        {
            cerr << "[SERVER | ERROR] Invalid value for p provided as argument" << endl;
            return 1;
        }
        catch (const out_of_range &e)
        {
            cerr << "[SERVER | ERROR] Value for p is out of range" << endl;
            return 1;
        }
    }

    string serverIP = config["server_ip"];
    int PORT = config["server_port"];

    int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket < 0)
    {
        cout << "[SERVER | ERROR] Socket not opened" << endl;
        close(serverSocket);
        return 1;
    }
    else
        cout << "[SERVER | INFO] Socket opened with id=" << serverSocket << endl;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP.c_str());
    memset(&(serverAddr.sin_zero), 0, 8);
    int serverAddrlen = sizeof(serverAddr);

    int bindResult = ::bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (bindResult < 0)
    {
        cout << "[SERVER | ERROR] Failed to bind" << endl;
        close(serverSocket);
        return 1;
    }
    else
        cout << "[SERVER | INFO] Binding Successfull" << endl;

    int listenResult = listen(serverSocket, 5);
    if (listenResult < 0)
    {
        cout << "[SERVER | ERROR] Failed to start listening" << endl;
        close(serverSocket);
        return 1;
    }
    else
        cout << "[SERVER | INFO] Listening... on IP:" << serverIP << " and PORT:" << PORT << endl;

    int connectionWithClient = accept(serverSocket, (struct sockaddr *)&serverAddr, (socklen_t *)&serverAddrlen);
    if (connectionWithClient < 0)
    {
        perror("[SERVER | ERROR] Accept failed");
        close(serverSocket);
        return 1;
    }
    cout << "[SERVER | INFO] Connection established with client" << endl;

    // Word Counting
    int k = config["k"];

    string fileName = config["input_file"];
    cout << "[SERVER | INFO] input-file=" << fileName << endl;

    char buffer[1024] = {0};
    int msgVal = 0;
    while ((msgVal = recv(connectionWithClient, buffer, 1024, 0)) > 0)
    {
        try
        {
            string msg(buffer);
            msg.erase(msg.find("\n"));
            int offset = stoi(msg);
            cout << "[SERVER | RECEIEVE] offset=" << offset << endl;

            ifstream inputFile(fileName);
            if (!inputFile.is_open())
                throw "File not open";

            string line;
            while (getline(inputFile, line))
            {
                stringstream ss(line);
                string word;
                string packet;
                int wordDescriptor = 0;
                int totalWordCounter = 0;
                int pktWordCount = 0;

                while (getline(ss, word, ','))
                {
                    wordDescriptor++;

                    if (wordDescriptor >= offset)
                    {
                        totalWordCounter++;
                        pktWordCount++;

                        if (totalWordCounter > k)
                            break;
                        if (pktWordCount <= p && totalWordCounter <= k)
                            packet += word + ",";
                        if (pktWordCount == p || totalWordCounter == k)
                        {
                            cout << "[SERVER | SEND] data=" << packet << endl;
                            packet += "\n";
                            send(connectionWithClient, packet.c_str(), packet.length(), 0);
                            packet = "";
                            pktWordCount = 0;
                        }
                    }
                }

                if (wordDescriptor < offset)
                {
                    cout << "[SERVER | SEND] out-of-bound offset" << endl;
                    send(connectionWithClient, "$$\n", 4, 0);
                }
                else if (totalWordCounter < k)
                {
                    packet += "EOF";
                    cout << "[SERVER | SEND] data=" << packet << endl;
                    packet += "\n";
                    send(connectionWithClient, packet.c_str(), packet.length(), 0);
                    break;
                }
            }
            inputFile.close();
        }
        catch (const exception &e)
        {
            cerr << "[SERVER | ERROR] " << e.what() << "\n";
            closeConnection(connectionWithClient, serverSocket, 1);
            return 1;
        }
        catch (const char *e)
        {
            cout << "[SERVER | ERROR] " << e << endl;
            closeConnection(connectionWithClient, serverSocket, 1);
            return 1;
        }
    }

    closeConnection(connectionWithClient, serverSocket);

    return 0;
}