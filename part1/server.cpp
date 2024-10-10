#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
using namespace std;

#define PORT 7012

struct sockaddr_in addr;

int main()
{
    int serverSocketID = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocketID < 0)
        cout << "Socket not opened" << endl;
    else
        cout << "Socket opened with id=" << serverSocketID << endl;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(addr.sin_zero), 0, 8);

    int bindResult = ::bind(serverSocketID, (struct sockaddr *)&addr, sizeof(addr));
    if (bindResult < 0)
        cout << "Failed to bind" << endl;
    else
        cout << "Binding Successfull" << endl;

    return 0;
}