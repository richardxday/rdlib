#ifndef __STANDARD_URI__
#define __STANDARD_URI__

#include "misc.h"
#include "strsup.h"
#include "StdData.h"
#include "Hash.h"

class AStdUri : public AStdData {
public:
    AStdUri();
    virtual ~AStdUri();

    virtual bool open(const AString& uri, bool writehint = false);
    virtual sint_t close();

    virtual bool isopen() const {return (handler && handler->isopen());}

    virtual slong_t bytesavailable() {return handler ? handler->bytesavailable() : -1;}
    virtual slong_t bytesqueued()    {return handler ? handler->bytesqueued()    : -1;}

    virtual slong_t tell() {return handler ? handler->tell() : -1;}
    virtual sint_t  seek(slong_t offset, uint_t origin = SEEK_SET) {return handler ? handler->seek(offset, origin) : -1;}
    virtual sint_t  flush() {return handler ? handler->flush() : -1;}
    virtual void    rewind() {if (handler) handler->rewind();}

    virtual sint_t  eof() {return handler ? handler->eof() : -1;}
    virtual sint_t  error() {return handler ? handler->error() : -1;}

    virtual sint_t  setvbuf(char *buffer, int mode, size_t size) {return handler ? handler->setvbuf(buffer, mode, size) : -1;}

    AStdData *gethandler() {return handler;}

    static void setglobaldefault(const char *name, uptr_t value) {globaldefaults.Insert(name, value);}
    void setdefault(const char *name, uptr_t value);
    uptr_t getdefault(const char *name, uptr_t defval = 0) {return (defaults && defaults->Exists(name)) ? defaults->Read(name) : (globaldefaults.Exists(name) ? globaldefaults.Read(name) : defval);}

    typedef bool (*TYPEHANDLER)(AStdUri& caller, const AString& type, const AString& addr, AStdData **handler, void *context);
    void addtypehandler(const char *type, TYPEHANDLER handler, void *context = NULL);
    void removetypehandler(const char *type);

    static void addglobaltypehandler(const char *type, TYPEHANDLER handler, void *context = NULL);
    static void removeglobaltypehandler(const char *type) {globaltypehandlers.Remove(type);}

    static void splittypeandaddr(const AString& uri, AString& type, AString& addr);
    static void splitnameandmode(const AString& addr, AString& name, AString& mode);
    static void splitnameandmode(const AString& addr, AString& name, uint_t& mode);
    static void splitserialparams(const AString& addr, AString& name, uint_t& baud, uint_t& mode, bool& flow);
    static void splittcpparams(const AString& addr, AString& host, uint_t& port, uint_t& type);
    static void splitudpparams(const AString& addr, AString& host, uint_t& port, AString& desthost, uint_t& destport);

    NODETYPE(AStdUri, AStdData);

protected:
    virtual slong_t readdata(void *buf, size_t bytes) {return handler ? handler->readbytes(buf, bytes) : 0;}
    virtual slong_t writedata(const void *buf, size_t bytes) {return handler ? handler->writebytes(buf, bytes) : 0;}

    typedef struct {
        TYPEHANDLER handler;
        void *context;
    } TYPEHANDLERDATA;
    static void __deletehandler(uptr_t item, void *context);

protected:
    AStdData *handler;
    AHash *defaults;
    AHash *typehandlers;

    NODETYPE_DEFINE(AStdUri);

    static AHash globaldefaults;
    static AHash globaltypehandlers;
};

#endif
