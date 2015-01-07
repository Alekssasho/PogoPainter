//
//  Socket.h
//  PogoPainter
//
//  Created by Aleksandar Angelov on 12/19/14.
//
//

#ifndef __PogoPainter__Socket__
#define __PogoPainter__Socket__

#include <string>

class SocketStream
{
public:
    SocketStream(const SocketStream&) = delete;
    SocketStream& operator=(const SocketStream&) = delete;
    SocketStream(SocketStream&& o);
    ~SocketStream();
    
    void Close();
    
    long SendBytes(const void* byteArray, size_t count);
    long ReceiveBytes(void* byteArray, size_t count);
    
    //Check how many bytes are available to read
    unsigned long Available();
    
    std::string GetPeerName();
    
protected:
    friend class ServerSocket;
    SocketStream(int sockfd = -1) :mSocket(sockfd) {}
    int mSocket;
};

class ClientSocket : public SocketStream
{
public:
    ClientSocket();
    ~ClientSocket();
    bool Connect(const std::string& ipaddress, int port);
};

class ServerSocket
{
public:
    ServerSocket();
    ~ServerSocket();
    bool Listen(int port);
    SocketStream Accept();
private:
    int mSocket = -1;
};

#endif /* defined(__PogoPainter__Socket__) */
