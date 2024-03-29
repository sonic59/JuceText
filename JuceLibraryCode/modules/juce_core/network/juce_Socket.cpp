/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#if JUCE_WINDOWS
 #include <winsock2.h>
 #include <ws2tcpip.h>

 #if JUCE_MSVC
  #pragma warning (push)
  #pragma warning (disable : 4127 4389 4018)
 #endif

#else
 #if JUCE_LINUX || JUCE_ANDROID
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <sys/errno.h>
  #include <unistd.h>
  #include <netinet/in.h>
 #endif

 #include <fcntl.h>
 #include <netdb.h>
 #include <arpa/inet.h>
 #include <netinet/tcp.h>
#endif

#ifndef AI_NUMERICSERV  // (missing in older Mac SDKs)
 #define AI_NUMERICSERV 0x1000
#endif

BEGIN_JUCE_NAMESPACE

#if JUCE_WINDOWS
 typedef int       juce_socklen_t;
#else
 typedef socklen_t juce_socklen_t;
#endif

//==============================================================================
namespace SocketHelpers
{
    void initSockets()
    {
       #if JUCE_WINDOWS
        static bool socketsStarted = false;

        if (! socketsStarted)
        {
            socketsStarted = true;

            WSADATA wsaData;
            const WORD wVersionRequested = MAKEWORD (1, 1);
            WSAStartup (wVersionRequested, &wsaData);
        }
       #endif
    }

    bool resetSocketOptions (const int handle, const bool isDatagram, const bool allowBroadcast) noexcept
    {
        const int sndBufSize = 65536;
        const int rcvBufSize = 65536;
        const int one = 1;

        return handle > 0
                && setsockopt (handle, SOL_SOCKET, SO_RCVBUF, (const char*) &rcvBufSize, sizeof (rcvBufSize)) == 0
                && setsockopt (handle, SOL_SOCKET, SO_SNDBUF, (const char*) &sndBufSize, sizeof (sndBufSize)) == 0
                && (isDatagram ? ((! allowBroadcast) || setsockopt (handle, SOL_SOCKET, SO_BROADCAST, (const char*) &one, sizeof (one)) == 0)
                               : (setsockopt (handle, IPPROTO_TCP, TCP_NODELAY, (const char*) &one, sizeof (one)) == 0));
    }

    bool bindSocketToPort (const int handle, const int port) noexcept
    {
        if (handle <= 0 || port <= 0)
            return false;

        struct sockaddr_in servTmpAddr = { 0 };
        servTmpAddr.sin_family = PF_INET;
        servTmpAddr.sin_addr.s_addr = htonl (INADDR_ANY);
        servTmpAddr.sin_port = htons ((uint16) port);

        return bind (handle, (struct sockaddr*) &servTmpAddr, sizeof (struct sockaddr_in)) >= 0;
    }

    int readSocket (const int handle,
                    void* const destBuffer, const int maxBytesToRead,
                    bool volatile& connected,
                    const bool blockUntilSpecifiedAmountHasArrived) noexcept
    {
        int bytesRead = 0;

        while (bytesRead < maxBytesToRead)
        {
            int bytesThisTime;

           #if JUCE_WINDOWS
            bytesThisTime = recv (handle, static_cast<char*> (destBuffer) + bytesRead, maxBytesToRead - bytesRead, 0);
           #else
            while ((bytesThisTime = (int) ::read (handle, addBytesToPointer (destBuffer, bytesRead), maxBytesToRead - bytesRead)) < 0
                     && errno == EINTR
                     && connected)
            {
            }
           #endif

            if (bytesThisTime <= 0 || ! connected)
            {
                if (bytesRead == 0)
                    bytesRead = -1;

                break;
            }

            bytesRead += bytesThisTime;

            if (! blockUntilSpecifiedAmountHasArrived)
                break;
        }

        return bytesRead;
    }

    int waitForReadiness (const int handle, const bool forReading, const int timeoutMsecs) noexcept
    {
        struct timeval timeout;
        struct timeval* timeoutp;

        if (timeoutMsecs >= 0)
        {
            timeout.tv_sec = timeoutMsecs / 1000;
            timeout.tv_usec = (timeoutMsecs % 1000) * 1000;
            timeoutp = &timeout;
        }
        else
        {
            timeoutp = 0;
        }

        fd_set rset, wset;
        FD_ZERO (&rset);
        FD_SET (handle, &rset);
        FD_ZERO (&wset);
        FD_SET (handle, &wset);

        fd_set* const prset = forReading ? &rset : nullptr;
        fd_set* const pwset = forReading ? nullptr : &wset;

       #if JUCE_WINDOWS
        if (select (handle + 1, prset, pwset, 0, timeoutp) < 0)
            return -1;
       #else
        {
            int result;
            while ((result = select (handle + 1, prset, pwset, 0, timeoutp)) < 0
                    && errno == EINTR)
            {
            }

            if (result < 0)
                return -1;
        }
       #endif

        {
            int opt;
            juce_socklen_t len = sizeof (opt);

            if (getsockopt (handle, SOL_SOCKET, SO_ERROR, (char*) &opt, &len) < 0
                 || opt != 0)
                return -1;
        }

        return FD_ISSET (handle, forReading ? &rset : &wset) ? 1 : 0;
    }

    bool setSocketBlockingState (const int handle, const bool shouldBlock) noexcept
    {
       #if JUCE_WINDOWS
        u_long nonBlocking = shouldBlock ? 0 : (u_long) 1;
        return ioctlsocket (handle, FIONBIO, &nonBlocking) == 0;
       #else
        int socketFlags = fcntl (handle, F_GETFL, 0);

        if (socketFlags == -1)
            return false;

        if (shouldBlock)
            socketFlags &= ~O_NONBLOCK;
        else
            socketFlags |= O_NONBLOCK;

        return fcntl (handle, F_SETFL, socketFlags) == 0;
       #endif
    }

    bool connectSocket (int volatile& handle,
                        const bool isDatagram,
                        void** serverAddress,
                        const String& hostName,
                        const int portNumber,
                        const int timeOutMillisecs) noexcept
    {
        struct addrinfo hints = { 0 };
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = isDatagram ? SOCK_DGRAM : SOCK_STREAM;
        hints.ai_flags = AI_NUMERICSERV;

        struct addrinfo* info = nullptr;
        if (getaddrinfo (hostName.toUTF8(), String (portNumber).toUTF8(), &hints, &info) != 0 || info == 0)
            return false;

        if (handle < 0)
            handle = (int) socket (info->ai_family, info->ai_socktype, 0);

        if (handle < 0)
        {
            freeaddrinfo (info);
            return false;
        }

        if (isDatagram)
        {
            struct sockaddr* s = new struct sockaddr();
            *s = *(info->ai_addr);
            *serverAddress = s;

            freeaddrinfo (info);
            return true;
        }

        setSocketBlockingState (handle, false);
        const int result = ::connect (handle, info->ai_addr, (int) info->ai_addrlen);
        freeaddrinfo (info);

        if (result < 0)
        {
           #if JUCE_WINDOWS
            if (result == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
           #else
            if (errno == EINPROGRESS)
           #endif
            {
                if (waitForReadiness (handle, false, timeOutMillisecs) != 1)
                {
                    setSocketBlockingState (handle, true);
                    return false;
                }
            }
        }

        setSocketBlockingState (handle, true);
        resetSocketOptions (handle, false, false);

        return true;
    }
}

//==============================================================================
StreamingSocket::StreamingSocket()
    : portNumber (0),
      handle (-1),
      connected (false),
      isListener (false)
{
    SocketHelpers::initSockets();
}

StreamingSocket::StreamingSocket (const String& hostName_,
                                  const int portNumber_,
                                  const int handle_)
    : hostName (hostName_),
      portNumber (portNumber_),
      handle (handle_),
      connected (true),
      isListener (false)
{
    SocketHelpers::initSockets();
    SocketHelpers::resetSocketOptions (handle_, false, false);
}

StreamingSocket::~StreamingSocket()
{
    close();
}

//==============================================================================
int StreamingSocket::read (void* destBuffer, const int maxBytesToRead, const bool blockUntilSpecifiedAmountHasArrived)
{
    return (connected && ! isListener) ? SocketHelpers::readSocket (handle, destBuffer, maxBytesToRead, connected, blockUntilSpecifiedAmountHasArrived)
                                       : -1;
}

int StreamingSocket::write (const void* sourceBuffer, const int numBytesToWrite)
{
    if (isListener || ! connected)
        return -1;

   #if JUCE_WINDOWS
    return send (handle, (const char*) sourceBuffer, numBytesToWrite, 0);
   #else
    int result;

    while ((result = (int) ::write (handle, sourceBuffer, numBytesToWrite)) < 0
            && errno == EINTR)
    {
    }

    return result;
   #endif
}

//==============================================================================
int StreamingSocket::waitUntilReady (const bool readyForReading,
                                     const int timeoutMsecs) const
{
    return connected ? SocketHelpers::waitForReadiness (handle, readyForReading, timeoutMsecs)
                     : -1;
}

//==============================================================================
bool StreamingSocket::bindToPort (const int port)
{
    return SocketHelpers::bindSocketToPort (handle, port);
}

bool StreamingSocket::connect (const String& remoteHostName,
                               const int remotePortNumber,
                               const int timeOutMillisecs)
{
    if (isListener)
    {
        jassertfalse;    // a listener socket can't connect to another one!
        return false;
    }

    if (connected)
        close();

    hostName = remoteHostName;
    portNumber = remotePortNumber;
    isListener = false;

    connected = SocketHelpers::connectSocket (handle, false, 0, remoteHostName,
                                              remotePortNumber, timeOutMillisecs);

    if (! (connected && SocketHelpers::resetSocketOptions (handle, false, false)))
    {
        close();
        return false;
    }

    return true;
}

void StreamingSocket::close()
{
   #if JUCE_WINDOWS
    if (handle != SOCKET_ERROR || connected)
        closesocket (handle);

    connected = false;
   #else
    if (connected)
    {
        connected = false;

        if (isListener)
        {
            // need to do this to interrupt the accept() function..
            StreamingSocket temp;
            temp.connect ("localhost", portNumber, 1000);
        }
    }

    if (handle != -1)
        ::close (handle);
   #endif

    hostName = String::empty;
    portNumber = 0;
    handle = -1;
    isListener = false;
}

//==============================================================================
bool StreamingSocket::createListener (const int newPortNumber, const String& localHostName)
{
    if (connected)
        close();

    hostName = "listener";
    portNumber = newPortNumber;
    isListener = true;

    struct sockaddr_in servTmpAddr = { 0 };
    servTmpAddr.sin_family = PF_INET;
    servTmpAddr.sin_addr.s_addr = htonl (INADDR_ANY);

    if (localHostName.isNotEmpty())
        servTmpAddr.sin_addr.s_addr = ::inet_addr (localHostName.toUTF8());

    servTmpAddr.sin_port = htons ((uint16) portNumber);

    handle = (int) socket (AF_INET, SOCK_STREAM, 0);

    if (handle < 0)
        return false;

    const int reuse = 1;
    setsockopt (handle, SOL_SOCKET, SO_REUSEADDR, (const char*) &reuse, sizeof (reuse));

    if (bind (handle, (struct sockaddr*) &servTmpAddr, sizeof (struct sockaddr_in)) < 0
         || listen (handle, SOMAXCONN) < 0)
    {
        close();
        return false;
    }

    connected = true;
    return true;
}

StreamingSocket* StreamingSocket::waitForNextConnection() const
{
    jassert (isListener || ! connected); // to call this method, you first have to use createListener() to
                                         // prepare this socket as a listener.

    if (connected && isListener)
    {
        struct sockaddr address;
        juce_socklen_t len = sizeof (sockaddr);
        const int newSocket = (int) accept (handle, &address, &len);

        if (newSocket >= 0 && connected)
            return new StreamingSocket (inet_ntoa (((struct sockaddr_in*) &address)->sin_addr),
                                        portNumber, newSocket);
    }

    return nullptr;
}

bool StreamingSocket::isLocal() const noexcept
{
    return hostName == "127.0.0.1";
}


//==============================================================================
//==============================================================================
DatagramSocket::DatagramSocket (const int localPortNumber, const bool allowBroadcast_)
    : portNumber (0),
      handle (-1),
      connected (true),
      allowBroadcast (allowBroadcast_),
      serverAddress (0)
{
    SocketHelpers::initSockets();

    handle = (int) socket (AF_INET, SOCK_DGRAM, 0);
    bindToPort (localPortNumber);
}

DatagramSocket::DatagramSocket (const String& hostName_, const int portNumber_,
                                const int handle_, const int localPortNumber)
    : hostName (hostName_),
      portNumber (portNumber_),
      handle (handle_),
      connected (true),
      allowBroadcast (false),
      serverAddress (0)
{
    SocketHelpers::initSockets();

    SocketHelpers::resetSocketOptions (handle_, true, allowBroadcast);
    bindToPort (localPortNumber);
}

DatagramSocket::~DatagramSocket()
{
    close();

    delete static_cast <struct sockaddr*> (serverAddress);
    serverAddress = 0;
}

void DatagramSocket::close()
{
   #if JUCE_WINDOWS
    closesocket (handle);
    connected = false;
   #else
    connected = false;
    ::close (handle);
   #endif

    hostName = String::empty;
    portNumber = 0;
    handle = -1;
}

bool DatagramSocket::bindToPort (const int port)
{
    return SocketHelpers::bindSocketToPort (handle, port);
}

bool DatagramSocket::connect (const String& remoteHostName,
                              const int remotePortNumber,
                              const int timeOutMillisecs)
{
    if (connected)
        close();

    hostName = remoteHostName;
    portNumber = remotePortNumber;

    connected = SocketHelpers::connectSocket (handle, true, &serverAddress,
                                              remoteHostName, remotePortNumber,
                                              timeOutMillisecs);

    if (! (connected && SocketHelpers::resetSocketOptions (handle, true, allowBroadcast)))
    {
        close();
        return false;
    }

    return true;
}

DatagramSocket* DatagramSocket::waitForNextConnection() const
{
    struct sockaddr address;
    juce_socklen_t len = sizeof (sockaddr);

    while (waitUntilReady (true, -1) == 1)
    {
        char buf[1];

        if (recvfrom (handle, buf, 0, 0, &address, &len) > 0)
        {
            return new DatagramSocket (inet_ntoa (((struct sockaddr_in*) &address)->sin_addr),
                                       ntohs (((struct sockaddr_in*) &address)->sin_port),
                                       -1, -1);
        }
    }

    return nullptr;
}

//==============================================================================
int DatagramSocket::waitUntilReady (const bool readyForReading,
                                    const int timeoutMsecs) const
{
    return connected ? SocketHelpers::waitForReadiness (handle, readyForReading, timeoutMsecs)
                     : -1;
}

int DatagramSocket::read (void* destBuffer, const int maxBytesToRead, const bool blockUntilSpecifiedAmountHasArrived)
{
    return connected ? SocketHelpers::readSocket (handle, destBuffer, maxBytesToRead, connected, blockUntilSpecifiedAmountHasArrived)
                     : -1;
}

int DatagramSocket::write (const void* sourceBuffer, const int numBytesToWrite)
{
    // You need to call connect() first to set the server address..
    jassert (serverAddress != 0 && connected);

    return connected ? (int) sendto (handle, (const char*) sourceBuffer,
                                     numBytesToWrite, 0,
                                     (const struct sockaddr*) serverAddress,
                                     sizeof (struct sockaddr_in))
                     : -1;
}

bool DatagramSocket::isLocal() const noexcept
{
    return hostName == "127.0.0.1";
}

#if JUCE_MSVC
 #pragma warning (pop)
#endif

END_JUCE_NAMESPACE
