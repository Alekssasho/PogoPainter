//
//  Socket.cpp
//  PogoPainter
//
//  Created by Aleksandar Angelov on 12/19/14.
//
//

#include "Socket.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

void SocketStream::Close()
{
    if(mSocket != -1) {
        close(mSocket);
        mSocket = -1;
    }
}

SocketStream::SocketStream(SocketStream&& o)
{
    //transfer file descriptor
    mSocket = o.mSocket;
    o.mSocket = -1;
}

SocketStream::~SocketStream()
{
    this->Close();
}

long SocketStream::SendBytes(void* byteArray, size_t count)
{
    //TODO: check with select for availability to send and some error detection would be good
    if(mSocket == -1)
        return -1;
    return send(mSocket, byteArray, count, 0);

}

long SocketStream::ReceiveBytes(void* byteArray, size_t count)
{
    if (mSocket == -1)
        return -1;
    return recv(mSocket, byteArray, count, 0);
}

int SocketStream::Available()
{
    int result;
    if(ioctl(mSocket, FIONREAD, &result) < 0) {
        return -1;
    }
    return result;
}

bool ClientSocket::Connect(const std::string& ipaddress, int port)
{
    mSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (mSocket == -1) {
        return false;
    }
    
    struct sockaddr_in desc;
    memset(&desc, 0, sizeof(desc));
    desc.sin_family = AF_INET;
    desc.sin_port = htons(port);
    inet_pton(AF_INET, ipaddress.c_str(), &desc.sin_addr);
    
    if(connect(mSocket, (struct sockaddr*)&desc, sizeof(desc)) != 0) {
        return false;
    }
    
    return true;
}

bool ServerSocket::Listen(int port)
{
    mSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (mSocket == -1) {
        return false;
    }
    
    struct sockaddr_in desc;
    memset(&desc, 0, sizeof(desc));
    
    desc.sin_family = AF_INET;
    desc.sin_addr.s_addr = htonl(INADDR_ANY);
    desc.sin_port = htons(port);
    
    int status = bind(mSocket, (struct sockaddr*)&desc, sizeof(desc));
    if (status < 0) {
        return false;
    }
    
    status = listen(mSocket, 4);
    if (status < 0) {
        return false;
    }
    
    return true;
}

SocketStream ServerSocket::Accept()
{
    if (mSocket == -1) {
        return SocketStream();
    }
    
    int sockfd = accept(mSocket, nullptr, nullptr);
    if (sockfd == -1) {
        return SocketStream();
    }
    
    return SocketStream(sockfd);
}

ServerSocket::~ServerSocket()
{
    close(mSocket);
}
