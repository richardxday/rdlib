#ifndef __TEXT_SERVER__
#define __TEXT_SERVER__

#include "SocketServer.h"
#include "DataList.h"

class ATextHandler;
class ATextServer {
public:
    ATextServer(ASocketServer& iserver);
    virtual ~ATextServer();

    virtual bool Create(uint_t port, ATextHandler *(*icreator)(ATextServer& server, int socket, void *context), void *context = NULL, const char *bindaddress = "0.0.0.0");
    virtual void Delete(bool force = false);

    bool IsValid() const {return (socket >= 0);}

    virtual void Process(uint_t timeout = 100);

    ASocketServer& GetSocketServer() {return server;}

    const ADataList& GetClientList() const {return clientlist;}

protected:
    static void __connectcallback(ASocketServer *server, int socket, void *context) {
        ((ATextServer *)context)->connectcallback(server, socket);
    }
    virtual void connectcallback(ASocketServer *server, int socket);

    static void __readcallback(ASocketServer *server, int socket, void *context);
    static void __writecallback(ASocketServer *server, int socket, void *context);
    static void __destructor(ASocketServer *server, int socket, void *context);
    static bool __needwritecallback(ASocketServer *server, int socket, void *context);

    static void __DeleteHandler(uptr_t item, void *context);

protected:
    ASocketServer& server;
    ATextHandler   *(*creator)(ATextServer& server, int socket, void *context);
    void           *creator_context;
    ADataList      clientlist;
    int            socket;
    bool           closewhendone;
};

class ATextHandler {
public:
    ATextHandler(ATextServer& iserver, int isocket);
    virtual ~ATextHandler();

    virtual void AddData(const AString& str);
    virtual void SendData();
    virtual void Close(bool force = false);
    virtual void CloseIfNeeded() {if (closewhendone) Close();}

    bool CanSendData() const {return senddata.Valid();}

    ATextServer& GetServer() {return server;}

protected:
    virtual void Write(const char *fmt, ...) PRINTF_FORMAT_METHOD;
    virtual void WriteToOthers(const char *fmt, ...) PRINTF_FORMAT_METHOD;

    virtual void Process(const AString& command);

protected:
    ATextServer& server;
    int          socket;
    AString      recvdata;
    AString      senddata;
    bool         closewhendone;
};

#endif
