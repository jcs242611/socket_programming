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

void closeConnection(int caller, int error = 0)
{
    close(caller);
    cout << (error ? "[CLIENT | ERROR] " : "[CLIENT | END] ") << "Connection ended with client" << endl;
}

int main()
{
    ifstream config_file("config.json");
    if (!config_file.is_open())
    {
        cerr << "[CLIENT | ERROR] Failed to open config.json" << endl;
        return 1;
    }

    json config;
    try
    {
        config_file >> config;
    }
    catch (const json::parse_error &e)
    {
        cerr << "[CLIENT | ERROR] Error parsing JSON: " << e.what() << endl;
        return 1;
    }

    int clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket < 0)
    {
        cout << "[CLIENT | ERROR] Socket not opened" << endl;
        close(clientSocket);
        return 1;
    }
    else
        cout << "[CLIENT | INFO] Socket opened with id=" << clientSocket << endl;

    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        cerr << "Error setting socket timeout" << endl;
        close(clientSocket);
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(config["server_port"]);
    serverAddr.sin_addr.s_addr = inet_addr(string(config["server_ip"]).c_str());

    int connectionWithServer = connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (connectionWithServer < 0)
    {
        perror("[CLIENT | ERROR] Connection failed");
        close(clientSocket);
        return -1;
    }
    cout << "[CLIENT | INFO] Connection established with server" << endl;

    // Word Counting
    int k = config["k"];

    unordered_map<string, int> wordCount;
    int totalWordCount = 0;

    bool EOFWord = false;
    while (!EOFWord)
    {
        try
        {
            if (totalWordCount % k == 0)
            {
                cout << "[CLIENT | SEND] offset=" << totalWordCount + 1 << endl;
                string wordOffsetInString = to_string(totalWordCount + 1) + "\n";
                send(clientSocket, wordOffsetInString.c_str(), wordOffsetInString.length() + 2, 0);
            }
            else
                throw "words-received=" + to_string(totalWordCount);

            char buffer[1024] = {0};
            int msgVal = 0;
            while ((msgVal = recv(clientSocket, buffer, 1024, 0)) > 0)
            {
                string msg(buffer);
                stringstream pktStream(msg);
                string pkt;

                while (getline(pktStream, pkt))
                {
                    cout << "[CLIENT | RECEIVE] data=" << pkt << endl;
                    if (pkt == "$$")
                        throw "Invalid offset (could be EOF sometimes)";

                    stringstream wordStream(pkt);
                    string word;

                    while (getline(wordStream, word, ','))
                    {
                        if (word == "EOF")
                        {
                            cout << "[CLIENT | SUCCESS] Download complete" << endl;
                            EOFWord = true;
                            break;
                        }

                        totalWordCount++;
                        wordCount[word]++;
                    }
                }

                if (totalWordCount % k == 0)
                    break;

                memset(buffer, 0, sizeof(buffer));
            }
        }
        catch (const exception &e)
        {
            cerr << "[CLIENT | ERROR] " << e.what() << '\n';
            closeConnection(clientSocket, 1);
            return 1;
        }
        catch (const char *e)
        {
            cout << "[CLIENT | ERROR] " << e << endl;
            if (string(e).compare("Invalid offset (could be EOF sometimes)") == 0)
                break;

            closeConnection(clientSocket, 1);
            return 1;
        }
    }

    // Writing to file
    ofstream outputFile("output_1.txt");
    if (!outputFile)
    {
        cerr << "[CLIENT | ERROR] Error opening file for writing!" << endl;
        closeConnection(clientSocket, 1);
        return 1;
    }

    map<string, int> wordCountInDict(wordCount.begin(), wordCount.end());
    for (auto it = wordCountInDict.begin(); it != wordCountInDict.end(); ++it)
    {
        outputFile << it->first << ", " << to_string(it->second);
        if (next(it) != wordCountInDict.end())
            outputFile << endl;
    }

    closeConnection(clientSocket);

    return 0;
}