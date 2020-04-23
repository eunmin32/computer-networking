/*
 * Sockets-client - client program to demonstrate sockets usage
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

const int BUFFSIZE=1500;

int main(int argc, char *argv[])
{
    char *serverName;
    char serverPort[6] = "3072";
    char *databuf;
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int clientSD = -1;


    /*
     *  Argument validation
     */
    if (argc != 2)
    {
       cerr << "Usage: " << argv[0] << " serverName" << endl;
       return -1;
    }

    /*
     * Use getaddrinfo() to get addrinfo structure corresponding to serverName / Port
	 * This addrinfo structure has internet address which can be used to create a socket too
     */
    serverName = argv[1];
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;					/* Allow IPv4 or IPv6*/
    hints.ai_socktype = SOCK_STREAM;					/* TCP */
    hints.ai_flags = 0;							/* Optional Options*/
    hints.ai_protocol = 0;						/* Allow any protocol*/
    int rc = getaddrinfo(serverName, serverPort, &hints, &result);
    if (rc != 0)
    {
       cerr << "ERROR: " << gai_strerror(rc) << endl;
       exit(EXIT_FAILURE);
    }

    /*
     * Iterate through addresses and connect
     */
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        clientSD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (clientSD == -1)
		{
            continue;
        }
		/*
		* A socket has been successfully created
		*/
        rc = connect(clientSD, rp->ai_addr, rp->ai_addrlen);
        if (rc < 0)
        {
            cerr << "Connection Failed" << endl;
            close(clientSD);
            return -1;
        }
        else	//success
        {
            break;
        }
    }

    if (rp == NULL)
    {
        cerr << "No valid address" << endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "Client Socket: " << clientSD << endl;
    }
    freeaddrinfo(result);

    /*
     *  Write and read data over network
     */
    databuf = new char[BUFFSIZE];
    for (int i = 0; i < BUFFSIZE; i++)
    {
        databuf[i] = 'z';
    }
 
    int bytesWritten = write(clientSD, databuf, BUFFSIZE);
    cout << "Bytes Written: " << bytesWritten << endl;

    int bytesRead = read(clientSD, databuf, BUFFSIZE);
    cout << "Bytes Read: " << bytesRead << endl;
    cout << databuf[13] << endl;

    close(clientSD);
    return 0;
}
