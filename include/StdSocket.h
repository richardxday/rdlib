#ifndef __STANDARD_SOCKET__
#define __STANDARD_SOCKET__

#include "misc.h"
#include "StdData.h"
#include "SocketServer.h"

class AStdSocket : public AStdData {
public:
    AStdSocket(ASocketServer& iserver);
    virtual ~AStdSocket();

    virtual bool open(const char *host, uint_t port, uint_t type = ASocketServer::Type_Client);
    virtual sint_t close();

    virtual bool isopen()             const {return (socket >= 0);}
    virtual bool isserverconnection() const {return isserver;}
    virtual bool clientconnected()    const {return (client != NULL);}

    virtual slong_t bytesavailable();
    virtual slong_t bytesqueued();

    virtual sint_t  flush();

    virtual const sockaddr_in *getdatagramsource() const;
    virtual bool setdatagramdestination(const char *host, uint_t port);
    virtual bool setdatagramdestination(const sockaddr_in *to);

    NODETYPE(AStdSocket, AStdData);

protected:
    virtual bool open(int isocket);

    virtual slong_t readdata(void *buf, size_t bytes);
    virtual slong_t writedata(const void *buf, size_t bytes);

    virtual void connectcallback(ASocketServer *server, int socket);
    static void __connectcallback(ASocketServer *server, int socket, void *context) {
        ((AStdSocket *)context)->connectcallback(server, socket);
    }

    virtual void readcallback(ASocketServer *server, int socket);
    static void __readcallback(ASocketServer *server, int socket, void *context) {
        ((AStdSocket *)context)->readcallback(server, socket);
    }

    virtual void destructor(ASocketServer *server, int socket);
    static void __destructor(ASocketServer *server, int socket, void *context) {
        ((AStdSocket *)context)->destructor(server, socket);
    }

protected:
    ASocketServer& server;
    AStdSocket *client;
    uint8_t *buffer;
    int  socket;
    uint_t bufferpos, bufferlen;
    bool isserver;
    bool socketclosed;

    NODETYPE_DEFINE(AStdSocket);
};

#endif
