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
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <ratio>

using namespace std;

int main(int argc, char* argv[])
{
    char* serverName; //1
    char* serverPort; //2
    int repition; //3
    int nbufs; //4
    int bufsize; //5
    int type;  //6

    struct addrinfo hints;
    struct addrinfo* result, * rp;
    int clientSD = -1;


    // Argument numbers validation
    if (argc != 7) {   //./client csslab4.uwb.edu 3072 1 3 1500 1
        cerr << "Usage: " << argv[0] << " serverName port reptition nbufs bufsize type" << endl;
        return -1;
    }

    serverName = argv[1];
    serverPort = argv[2];
    repition = atoi(argv[3]);
    nbufs = atoi(argv[4]);
    bufsize = atoi(argv[5]);;
    type = atoi(argv[6]);

    //valdiation 
    if (atoi(serverPort) < 1024 || atoi(serverPort) > 49151 ) {
        cerr << "Please type serverPort in range of 1024-49151" << endl;
        return -1;
    }
    if(type > 3 || type < 1) {
        cerr << "Please type type in range of 1-3" << endl;
        return -1;
    }
    if (nbufs * bufsize != 1500) {
        cerr << "nbufs * bufsize has to be 1500" << endl;
        return -1;
    }
    
    char databuf[nbufs][bufsize];

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;					/* Allow IPv4 or IPv6*/
    hints.ai_socktype = SOCK_STREAM;					/* TCP */
    hints.ai_flags = 0;							/* Optional Options*/
    hints.ai_protocol = 0;						/* Allow any protocol*/
    int rc = getaddrinfo(serverName, serverPort, &hints, &result);
    if (rc != 0) {
        cerr << "ERROR: " << gai_strerror(rc) << endl;
        exit(EXIT_FAILURE);
    }

    // Iterate through addresses and connect  
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        clientSD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (clientSD == -1){
            continue;
        }
        
        //A socket has been successfully created 
        rc = connect(clientSD, rp->ai_addr, rp->ai_addrlen);
        if (rc < 0){
            cerr << "Connection Failed" << endl;
            close(clientSD);
            return -1;
        } else {
            break;
        }
    }

    if (rp == NULL) {
        cerr << "No valid address" << endl;
        exit(EXIT_FAILURE);
    } else {
        cout << "Client Socket: " << clientSD << endl;
    }
    freeaddrinfo(result);

    //client sends a message to the server which contains the number of iterations it will perform
    int index = 0;
    for (auto c : to_string(repition)) {
        databuf[0][index] = c;
        index++;
    }
    write(clientSD, databuf[0], bufsize);
    
    //test
    cout << "Start test" << endl;
    chrono::steady_clock::time_point t1;
    if (type == 1) {
        t1 = chrono::steady_clock::now();
        for (int i = 0; i < repition; i++) {
            for (int j = 0; j < nbufs; j++) {
                write(clientSD, databuf[j], bufsize);
            }
        }

    } else if (type == 2) {
        t1 = chrono::steady_clock::now();
        cout << "test 2" << endl;
        for (int i = 0; i < repition; i++) {
            struct iovec vector[nbufs];
            for (int j = 0; j < nbufs; j++){
                vector[j].iov_base = databuf[j];
                vector[j].iov_len = bufsize;
            }
            writev(clientSD, vector, nbufs);
        }

    } else if(type == 3) {
        t1 = chrono::steady_clock::now();
        cout << "test 3" << endl;
        for (int i = 0; i < repition; i++) {
            write(clientSD, databuf, bufsize * nbufs);
        }
    }
    cout << "All buffer sent" << endl;

    //Reading count
    int bytesRead;
    int inRead = 0;
    while (inRead < bufsize){
        bytesRead = read(clientSD, databuf, bufsize);
        inRead += bytesRead;
    }
    chrono::steady_clock::time_point t2 = chrono::steady_clock::now();
    
    int count = atoi(databuf[0]);
    int etime = chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    cout << "Test " << type << ": time = " << etime
    << " usec, #reads = " << count << ", throughput " << (repition * 1500 * 8) / (etime) << " Mbps" << endl << endl; 
    close(clientSD);
    return 0;
}
