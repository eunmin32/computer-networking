/*
 * Sockets-server - 
 * Simple server program to demonstrate sockets usage
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

const int BUFFSIZE = 1500;
const int NUM_CONNECTIONS = 5;

int main(int argc, char *argv[])
{
    int serverPort;
    char *serverName;
    char databuf[BUFFSIZE];
    bzero(databuf, BUFFSIZE);

    /* 
     * Build address
     */
    int port = 3072;
    sockaddr_in acceptSocketAddress;
    bzero((char *)&acceptSocketAddress, sizeof(acceptSocketAddress));
    acceptSocketAddress.sin_family = AF_INET; //using the internet
    acceptSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY); //accept any IP address
    acceptSocketAddress.sin_port = htons(port);

    /*
     *  Open socket and bind
     */
    int serverSD = socket(AF_INET, SOCK_STREAM, 0); //using TCP
    const int on = 1;
    setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));
    cout << "Socket #: " << serverSD << endl;
   
    int rc = bind(serverSD, (sockaddr *)&acceptSocketAddress, sizeof(acceptSocketAddress));
    if (rc < 0)
    {
        cerr << "Bind Failed" << endl;
    }

    /*
     *  listen and accept
     */
    listen(serverSD, NUM_CONNECTIONS);       //setting number of pending connections
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    int newSD = accept(serverSD, (sockaddr *) &newSockAddr, &newSockAddrSize);
    cout << "Accepted Socket #: " << newSD <<endl;

    /*
     *  read from the socket
     */
    int bytesRead = read(newSD, databuf, BUFFSIZE);
    cout << "Bytes Read: " << bytesRead << endl;
    cout << databuf[0] << endl;

    /*
     *  write to socker
    
    databuf[13] = 'R';
    int bytesWritten = write(newSD, databuf, BUFFSIZE);
    cout << "Bytes Written: " << bytesWritten << endl;
*/
    close(newSD);
    close(serverSD);

    return 0;
}
