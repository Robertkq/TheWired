#include "server.h"

int main()
{
    server Server(host_port, &scramble);
    Server.Start();

    while (true)
    {
        Server.Update();
    }

    Server.Stop();
    return 0;
}