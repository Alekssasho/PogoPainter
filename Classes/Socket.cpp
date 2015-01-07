//
//  Socket.cpp
//  PogoPainter
//
//  Created by Aleksandar Angelov on 12/19/14.
//
//

#include "Socket.h"
#include <string.h>

#include "cocos2d.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT) || (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    #define SOCKET_WIN
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

void SocketStream::Close()
{
    if(mSocket != -1) {
#ifdef SOCKET_WIN
		closesocket(mSocket);
#else
        close(mSocket);
#endif
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

long SocketStream::SendBytes(const void* byteArray, size_t count)
{
    //TODO: check with select for availability to send and some error detection would be good
    if(mSocket == -1)
        return -1;
    return send(mSocket, reinterpret_cast<const char*>(byteArray), count, 0);

}

long SocketStream::ReceiveBytes(void* byteArray, size_t count)
{
    if (mSocket == -1)
        return -1;
    return recv(mSocket, reinterpret_cast<char*>(byteArray), count, 0);
}

unsigned long SocketStream::Available()
{
    unsigned long result = 0;
#ifdef SOCKET_WIN
    if(ioctlsocket(mSocket, FIONREAD, &result) < 0) {
#else
    if(ioctl(mSocket, FIONREAD, &result) < 0) {
#endif

        return 0;
    }
    return result;
}
    
std::string SocketStream::GetPeerName()
{
    if(mSocket == -1)
        return "";
    
    struct sockaddr_in clientSocket;
    socklen_t size = sizeof(struct sockaddr_in);
    getpeername(mSocket,(struct sockaddr*) &clientSocket, &size);
    
    return std::string(inet_ntoa(clientSocket.sin_addr));
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
	desc.sin_addr.s_addr = inet_addr(ipaddress.c_str());
    
    if(connect(mSocket, (struct sockaddr*)&desc, sizeof(desc)) != 0) {
        return false;
    }
    
    return true;
}

ClientSocket::ClientSocket()
{
#ifdef SOCKET_WIN
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2),&wsaData);
#endif
}

ClientSocket::~ClientSocket()
{
#ifdef SOCKET_WIN
    WSACleanup();
#endif
}

ServerSocket::ServerSocket()
{
#ifdef SOCKET_WIN
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2),&wsaData);
#endif
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
#ifdef SOCKET_WIN
    closesocket(mSocket);
    WSACleanup();
#else
    close(mSocket);
#endif
    
}
