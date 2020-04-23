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
#include <cstdlib>
#include <pthread.h>


using namespace std;

const int BUFFSIZE = 1500;
const int NUM_CONNECTIONS = 5;

void* servicingThread(void* args);

int main(int argc, char *argv[]) {
    
    if (argc != 2) {   
        cerr << "Usage: " << argv[0] << " port" << endl;
        return -1;
    }

    //build address
    int port = atoi(argv[1]);
    cout << "Port is " << port << endl;  
    if (port < 1024 || port > 49151 ) {
        cerr << "Please type serverPort in range of 1024-49151" << endl;
        return -1;
    }

    sockaddr_in acceptSocketAddress;
    bzero((char *)&acceptSocketAddress, sizeof(acceptSocketAddress));
    acceptSocketAddress.sin_family = AF_INET; 
    acceptSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY); 
    acceptSocketAddress.sin_port = htons(port);

    //Open socket and bind
    int serverSD = socket(AF_INET, SOCK_STREAM, 0); //using TCP
    const int on = 1;
    setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));
    cout << "Socket #: " << serverSD << endl;
    int rc = bind(serverSD, (sockaddr *)&acceptSocketAddress, sizeof(acceptSocketAddress));
    if (rc < 0){
        cerr << "Bind Failed" << endl;
    }
   
    //listen and accept
    listen(serverSD, NUM_CONNECTIONS); 
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    //build pthread
    pthread_attr_t attr;
    pthread_t threads;
    pthread_attr_init(&attr);

    //accept new connection requests
    while(true) {
        int newSD = accept(serverSD, (sockaddr *) &newSockAddr, &newSockAddrSize);
        cout << "Accepted Socket #: " << newSD <<endl;       
        pthread_create(&threads, &attr, servicingThread, &newSD);  
    }
    pthread_join(threads, NULL);  
    close(serverSD);
    return 0;
}


void* servicingThread(void* args) {
    cout << "start new thread" << endl;
    int& newSD = *((int*)args);
    char databuf[BUFFSIZE];
    bzero(databuf, BUFFSIZE);

    //read iteration 
    int bytesRead;
    int inRead = 0;
    while (inRead < BUFFSIZE){
        bytesRead = read(newSD, databuf, BUFFSIZE - inRead);
        inRead += bytesRead;
    }
    int iteration = atoi(databuf);
    cout << iteration<< " times iteration" << endl;    

    int count = 0;   
    //read data 
    for (int i=0; i < iteration -1; i++) {
        inRead = 0;
        while (inRead < BUFFSIZE){ 
            bytesRead = read(newSD, databuf, BUFFSIZE - inRead);
            inRead += bytesRead;
            count++;
        }
    }
    //printf(to_string(count), databuf); 
    bzero(databuf, BUFFSIZE);
    int b = 0;
    for (auto a: to_string(count)) {
        databuf[b] = a;
        b++;
    }
    write(newSD, databuf, BUFFSIZE);

    //Close this connection.
    close(newSD);
    //Terminate the thread
    return 0;
}

