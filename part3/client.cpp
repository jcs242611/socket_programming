#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include "json.hpp"
using namespace std;
using namespace std::chrono;

using json = nlohmann::json;
json config;

struct sockaddr_in serverAddr;

void *downloadFileFromServer(void *threadID)
{
    auto start = high_resolution_clock::now();

    int clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket < 0)
    {
        cout << "[CLIENT | ERROR] Socket not opened" << endl;
        close(clientSocket);
        return nullptr;
    }
    else
        cout << "[CLIENT | INFO] Socket opened with id=" << clientSocket << endl;

    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        cerr << "[CLIENT | ERROR | " << clientSocket << "] Error setting socket timeout" << endl;
        close(clientSocket);
        return nullptr;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(config["server_port"]);
    serverAddr.sin_addr.s_addr = inet_addr(string(config["server_ip"]).c_str());

    int connectionWithServer = connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (connectionWithServer < 0)
    {
        cerr << "[CLIENT | ERROR | " << clientSocket << "] Connection failed" << endl;
        close(clientSocket);
        return nullptr;
    }
    cout << "[CLIENT | INFO | " << clientSocket << "] Connection established with server" << endl;

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
                cout << "[CLIENT | SEND | " << clientSocket << "] offset=" << totalWordCount + 1 << endl;
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
                    cout << "[CLIENT | RECEIVE | " << clientSocket << "] data=" << pkt << endl;
                    if (pkt == "$$")
                        throw "Invalid offset (could be EOF sometimes)";

                    stringstream wordStream(pkt);
                    string word;

                    while (getline(wordStream, word, ','))
                    {
                        if (word == "EOF")
                        {
                            cout << "[CLIENT | SUCCESS | " << clientSocket << "] Download complete" << endl;
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
            cerr << "[CLIENT | ERROR | " << clientSocket << "] " << e.what() << '\n';
            close(clientSocket);
            return nullptr;
        }
        catch (const char *e)
        {
            cout << "[CLIENT | ERROR | " << clientSocket << "] " << e << endl;
            if (string(e).compare("Invalid offset (could be EOF sometimes)") == 0)
                break;

            close(clientSocket);
            return nullptr;
        }
    }

    // Writing to file
    string filename = "output_" + to_string(*(int *)threadID) + ".txt";
    cout << "[CLIENT | WRITE | " << clientSocket << "] output-file=" << filename << endl;
    ofstream outputFile(filename);
    if (!outputFile)
    {
        cerr << "[CLIENT | ERROR | " << clientSocket << "] Error opening file for writing!" << endl;
        close(clientSocket);
        return nullptr;
    }

    map<string, int> wordCountInDict(wordCount.begin(), wordCount.end());
    for (auto it = wordCountInDict.begin(); it != wordCountInDict.end(); ++it)
    {
        outputFile << it->first << ", " << to_string(it->second);
        if (next(it) != wordCountInDict.end())
            outputFile << endl;
    }

    close(clientSocket);

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    cout << "[CLIENT | TIME | " << clientSocket << "] time-taken=" << duration.count() << " ms" << endl;

    return nullptr;
}

int main(int argc, char *argv[])
{
    ifstream config_file("config.json");
    if (!config_file.is_open())
    {
        cerr << "[CLIENT | ERROR] Failed to open config.json" << endl;
        return 1;
    }

    try
    {
        config_file >> config;
    }
    catch (const json::parse_error &e)
    {
        cerr << "[CLIENT | ERROR] Error parsing JSON: " << e.what() << endl;
        return 1;
    }

    int num_clients = config["num_clients"];
    if (argc == 2)
    {
        try
        {
            num_clients = stoi(argv[1]);
        }
        catch (const invalid_argument &e)
        {
            cerr << "[CLIENT | ERROR] Invalid value for num_clients provided as argument" << endl;
            return 1;
        }
        catch (const out_of_range &e)
        {
            cerr << "[CLIENT | ERROR] Value for num_clients is out of range" << endl;
            return 1;
        }
    }

    pthread_t threads[num_clients + 1];

    for (int i = 1; i <= num_clients; i++)
    {
        int *threadID = new int(i);
        int result = pthread_create(&threads[i], nullptr, downloadFileFromServer, threadID);
        if (result != 0)
            cerr << "[CLIENT | ERROR] Error creating thread " << i << endl;
        else
            cout << "[CLIENT | THREAD] threadID=" << i << " created" << endl;
    }

    for (int i = 1; i <= num_clients; i++)
        pthread_join(threads[i], nullptr);

    cout << "[CLIENT | SUCCESS] All client threads finished executing" << endl;
    return 0;
}