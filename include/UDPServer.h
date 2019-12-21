#ifndef __UDP_SERVER__
#define __UDP_SERVER__

#include "SocketServer.h"

class AUDPServer {
public:
    AUDPServer(ASocketServer& iserver);
    virtual ~AUDPServer();

    virtual bool Create(const char *bindaddress, uint_t port);
    virtual void Delete();

    bool IsValid() const {return (socket >= 0);}

    virtual void Process(uint_t timeout = 100);

    ASocketServer& GetSocketServer() {return server;}

protected:
    static void __readcallback(ASocketServer *server, int socket, void *context) {
        UNUSED(server);
        UNUSED(socket);
        ((AUDPServer *)context)->ReadCallback();
    }

    void ReadCallback();

    void Send(const uint8_t *data, uint_t bytes, const struct sockaddr *to);
    void Send(const AString& data, const struct sockaddr *to);

    virtual bool Process(const uint8_t *data, uint_t bytes, const struct sockaddr *from);
    virtual bool Process(const AString& data, const struct sockaddr *from);

protected:
    ASocketServer& server;
    int            socket;
};

#endif
