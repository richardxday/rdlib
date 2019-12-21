#ifndef __HTTP_REQUEST__
#define __HTTP_REQUEST__

#include <vector>

#include "SocketServer.h"

class AHTTPRequest : public ASocketServer::Handler {
public:
    AHTTPRequest(ASocketServer *_server = NULL);
    AHTTPRequest(ASocketServer *_server, const AString& _url);
    virtual ~AHTTPRequest();

    virtual bool OpenURL(const AString& _url);
    virtual bool IsComplete() const {return complete;}

protected:
    virtual void OnConnect();
    virtual void OnRead();
    virtual void OnWrite();
    virtual bool NeedWrite() {return sendrequest;}
    virtual void Cleanup();

    virtual void ProcessData() {}

    void RemoveBytes(uint_t n);
    virtual bool GetHeader(AString& header, uint_t maxlen);

protected:
    AString              url;
    AString              host;
    uint_t               port;
    bool                 sendrequest;
    bool                 complete;
    std::vector<uint8_t> data;
    uint_t               pos;
};

#endif
