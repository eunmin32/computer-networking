#include <iostream>
#include "UdpSocket.h"
#include "Timer.h"
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>
using namespace std;

const int PORT = 30205;       // my UDP port
const int MAX = 20000;        // times of message transfer
const int MAX_WIN = 30;       // maximum window size
const bool verbose = false;   //use verbose mode for more information during run
const bool excelFile = true;  //Create output for excel file 
ofstream outfile1("slidingWindowResult.csv");

// client packet sending functions
void ClientUnreliable(UdpSocket &sock, int max, int message[]);
int ClientStopWait(UdpSocket &sock, int max, int message[]);
int ClientSlidingWindow(UdpSocket &sock, int max, int message[],int windowSize);

// server packet receiving functions
void ServerUnreliable(UdpSocket &sock, int max, int message[]);
void ServerReliable(UdpSocket &sock, int max, int message[]);
void ServerEarlyRetrans(UdpSocket &sock, int max, int message[],int windowSize );

enum myPartType {CLIENT, SERVER} myPart;

int main( int argc, char *argv[] ) 
{
    int message[MSGSIZE/4]; 	  // prepare a 1460-byte message: 1460/4 = 365 ints;

    // Parse arguments
    if (argc == 1) 
    {
        myPart = SERVER;
    }
    else if (argc == 2)
    {
        myPart = CLIENT;
    }
    else
    {
        cerr << "usage: " << argv[0] << " [serverIpName]" << endl;
        return -1;
    }

    // Set up communication
    // Use different initial ports for client server to allow same box testing
    UdpSocket sock( PORT + myPart );  
    if (myPart == CLIENT)
    {
        if (! sock.setDestAddress(argv[1], PORT + SERVER)) 
        {
            cerr << "cannot find the destination IP name: " << argv[1] << endl;
            return -1;
        }
    }

    int testNumber;
    cerr << "Choose a testcase" << endl;
    cerr << "   1: unreliable test" << endl;
    cerr << "   2: stop-and-wait test" << endl;
    cerr << "   3: sliding windows" << endl;
    cerr << "--> ";
    cin >> testNumber;

    if (myPart == CLIENT) 
    {
        Timer timer;           
        int retransmits = 0;   

        switch(testNumber) 
        {
        case 1:
            timer.Start();
            ClientUnreliable(sock, MAX, message); 
            cout << "Elasped time = ";  
            cout << timer.End( ) << endl;
            break;
        case 2:
            timer.Start();   
            retransmits = ClientStopWait(sock, MAX, message); 
            cout << "Elasped time = "; 
            cout << timer.End( ) << endl;
            cout << "retransmits = " << retransmits << endl;
            break;
        case 3:            
            for (int windowSize = 1; windowSize <= MAX_WIN; windowSize++ ) {
                timer.Start( );
                retransmits = ClientSlidingWindow(sock, MAX, message, windowSize);
                cout << "Window size = ";  
                cout << windowSize << " ";
                cout << "Elasped time = "; 
                int end = timer.End();
                cout << end << endl;
                cout << "retransmits = " << retransmits << endl;
                if (excelFile) {
                    outfile1 << windowSize << ", " << end << ", " << retransmits << endl;
                }
            }
            break;
        default:
            cerr << "no such test case" << endl;
            break;
        }
    }
    if (myPart == SERVER) 
    {
        switch(testNumber) 
        {
            case 1:
                ServerUnreliable(sock, MAX, message);
                break;
            case 2:
                ServerReliable(sock, MAX, message);
                break;
            case 3:
               for (int windowSize = 1; windowSize <= MAX_WIN; windowSize++) {
	                ServerEarlyRetrans( sock, MAX, message, windowSize );
                }
                break;
            default:
                cerr << "no such test case" << endl;
                break;
        }

        // The server should make sure that the last ack has been delivered to client.
        
        if (testNumber != 1)
        {
            if (verbose)
            {
                cerr << "server ending..." << endl;
            }
            for ( int i = 0; i < 10; i++ ) 
            {
                sleep( 1 );
                int ack = MAX - 1;
                sock.ackTo( (char *)&ack, sizeof( ack ) );
            }
        }
    }
    cout << "finished" << endl;
    return 0;
}
// Test 1 Client
void ClientUnreliable(UdpSocket &sock, int max, int message[]) 
{
    // transfer message[] max times; message contains sequences number i
    for ( int i = 0; i < max; i++ ) 
    {
        message[0] = i;                            
        sock.sendTo( ( char * )message, MSGSIZE ); 
        if (verbose)
        {
            cerr << "message = " << message[0] << endl;
        }
    }
    cout << max << " messages sent." << endl;
}

// Test1 Server
void ServerUnreliable(UdpSocket &sock, int max, int message[]) 
{
    // receive message[] max times and do not send ack
    for (int i = 0; i < max; i++) 
    {
        sock.recvFrom( ( char * ) message, MSGSIZE );
        if (verbose)
        {  
            cerr << message[0] << endl;
        }                    
    }
    cout << max << " messages received" << endl;
}

//return retransmits #
int ClientStopWait(UdpSocket &sock, int max, int message[])
{
    int retransmit = 0; 
    //Implement this function
    int i = 0;
    Timer timeoutTimer;
    bool timeout;
    bool serverClosed = false;
    while(i < max) {
        message[0] = i;                            
        sock.sendTo( ( char * )message, MSGSIZE ); //send pocket
        if (verbose) {
            cerr << i << " messages sent." << endl;
        }
        timeout = false;
        timeoutTimer.Start();  
        while(sock.pollRecvFrom() <= 0) {
             if (timeoutTimer.End() > 1500) {
                timeout =true;
                break;
             }
        }
        if(timeout){
            retransmit++;
        } else {
            sock.recvFrom( ( char * ) message, MSGSIZE );
            if (verbose) {
                cout << "ACK " << message[0] << " received." << endl;
            }
            if(message[0] == i) {
                i++;
            }          
        }     
    } 
    return retransmit;
}

void ServerReliable(UdpSocket &sock, int max, int message[])
{
    int i = 0;
    while(i < max - 1) {
        while(sock.pollRecvFrom() <=  0) {}
        sock.recvFrom( ( char * ) message, MSGSIZE );
        i = message[0];        
        if (verbose) {  
            cout << "Sever received "<< i << endl;
        } 
        sock.ackTo(( char * ) message, MSGSIZE);
    }

    //to cover edge case when the last packet is dropped 
    //10 extra check
    for (int j = 0; j < 10; j++) {
        if(sock.pollRecvFrom() > 0) {
            sock.recvFrom( ( char * ) message, MSGSIZE );
            sock.ackTo(( char * ) message, MSGSIZE);
        }
    }
    cout << "closing server" << endl;
    return;
}

int ClientSlidingWindow(UdpSocket &sock, int max, int message[], int windowSize) {
    for (int i = 0; i < windowSize; i++) {
        message[0] = i;                            
        sock.sendTo( ( char * )message, MSGSIZE ); 
        if (verbose)
        {
            cerr << "message = " << message[0] << endl;
        }
    }
    int LAR = 0;
    int LFS = windowSize;
    Timer timeoutTimer;
    bool timeout = false;
    int retransmit = 0;
    while (LAR < max) {
        timeoutTimer.Start();  
        while(sock.pollRecvFrom() <= 0) {
             if (timeoutTimer.End() > 1500) {
                timeout = true;
                break;
             }
        }
        if (timeout) { //send packet again
            for (int i = LAR; i < LFS; i++) {
                retransmit++;
                message[0] = i;                        
                sock.sendTo( ( char * )message, MSGSIZE );
                if (verbose) {
                    cerr << "Timeout. resend message = " << message[0] << endl;
                }
            }
            timeout = false; 
        } else {
            
            sock.recvFrom( ( char * ) message, MSGSIZE );
            if (verbose) {
                    cout << "ACK recieved: " << message[0] << endl;
            }
            if (message[0] >= LAR) {
                LAR = message[0] + 1;
                message[0] = message[0] + windowSize;
                sock.sendTo( ( char * )message, MSGSIZE );  
                if (verbose) {
                    cout << "Client Send: " << message[0] << endl;
                }
                LFS = LAR + windowSize;
                if (LFS > max)
                    LFS = max;
            }
        }      
    }
    return retransmit;
}

void ServerEarlyRetrans(UdpSocket &sock, int max, int message[],int windowSize ) {
    int LFR = 0;
    int LAF = LFR + windowSize;
    bool* buffer = new bool[max] {false};
   // bool buffer[max] {false};

    while (LFR < max) {
        while(sock.pollRecvFrom() <=  0) {}
        sock.recvFrom( ( char * ) message, MSGSIZE );
        if (verbose) {
                    cout << "Server recieved: " << message[0] << endl;
        }
        if(message[0] < max)
            buffer[message[0]] = true;
        if (message[0] < LFR) {
            message[0] = LFR - 1;
            sock.ackTo(( char * ) message, MSGSIZE);
            if (verbose) {
                    cout << "ACKed: " << message[0] << endl;
            }
        } else if (message[0] >= LFR && message[0] < LAF) {
            if (message[0] == LFR) {
                int i = 0;
                while (i < windowSize) {
                    if (!buffer[message[0] + i])
                        break;
                    i++;
                }
                LFR += i;
                LAF += i;
                if (LAF > max)
                    LAF = max;
            }
            message[0] = LFR - 1;
            sock.ackTo(( char * ) message, MSGSIZE);
            if (verbose) {
                    cout << "ACKed: " << message[0] << endl;
            }
        } 
    }
    delete buffer;
    cout << "Closing Server" << endl;
}
