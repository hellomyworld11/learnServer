#include "xserver.h"
#include <iostream>


int main(int argc, char *argv[])
{
    int port = -1;
    char *ip = nullptr;
    if (argc <= 1)
    {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }else if(argc == 2)
    {
        printf("listen any address\n");
        port = atoi(argv[1]);
    }else if (argc >= 3)
    {
        ip = argv[1];
        port = atoi(argv[2]);
    }

    XServer server(ip, port);
    server.start();
    return 0;
}