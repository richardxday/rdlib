
#ifndef __SOCKET_SERVER__
#define __SOCKET_SERVER__

#include "strsup.h"
#include "DataList.h"

#if defined(__LINUX__) || defined(__CYGWIN__)
#include <netinet/in.h>
#endif

class ASocketServer {
public:
	ASocketServer();
	~ASocketServer();

	enum {
		Type_Invalid = 0,
		Type_Server,
		Type_Client,
		Type_Datagram,
	};

	static bool Resolve(const char *host, uint_t port, struct sockaddr_in *sockaddr);

	int CreateHandler(uint_t type,
					  const char *host,
					  uint_t port,
					  void (*connectcallback)(ASocketServer *server, int socket, void *context),
					  void (*readcallback)(ASocketServer *server, int socket, void *context),
					  void (*writecallback)(ASocketServer *server, int socket, void *context) = NULL,
					  void (*destructor)(ASocketServer *server, int socket, void *context) = NULL,
					  bool (*needwritecallback)(ASocketServer *server, int socket, void *context) = NULL,
					  void *context = NULL);
	void DeleteAllHandlers();

	int Process(uint_t timeout);

	void SetConnectHandler(int socket, void (*connectcallback)(ASocketServer *server, int socket, void *context));
	void SetReadHandler(int socket, void (*readcallback)(ASocketServer *server, int socket, void *context));
	void SetWriteHandler(int socket, void (*writecallback)(ASocketServer *server, int socket, void *context));
	void SetDestructor(int socket, void (*destructor)(ASocketServer *server, int socket, void *context));
	void SetNeedWriteHandler(int socket, bool (*needwritecallback)(ASocketServer *server, int socket, void *context));
	void SetContext(int socket, void *context);
	const struct sockaddr_in *GetDatagramSource(int socket) const;
	void SetDatagramDestination(int socket, const struct sockaddr_in *to);

	void DeleteHandler(int socket);

	sint_t BytesAvailable(int socket);
	sint_t BytesLeftToWrite(int socket);
	sint_t ReadSocket(int socket, uint8_t *buffer, uint_t bytes, struct sockaddr *from = NULL);
	sint_t WriteSocket(int socket, const uint8_t *buffer, uint_t bytes, const struct sockaddr *to = NULL);

	uint_t GetMaxSendBuffer() const {return MaxSendBuffer;}
	void SetMaxSendBuffer(uint_t bufsize) {MaxSendBuffer = bufsize;}

	uint_t GetSocketCount() const {return SocketList.Count();}

	const struct sockaddr_in *GetSocketAddr(int socket) const;

	typedef struct {
		uint8_t    buffer[1024];
		AString lines;
	} SOCKETREADER;

	sint_t ReadLineData(int socket, SOCKETREADER& reader, struct sockaddr *from = NULL);
	static bool ReadLine(SOCKETREADER& reader, AString& line);

	sint_t WriteSocket(int socket, const AString& str, const struct sockaddr *to = NULL) {
		return WriteSocket(socket, (const uint8_t *)str.str(), str.len(), to);
	}

	AString GetClientAddr(int socket) const;

	static AString GetClientAddr(const struct sockaddr_in *sockaddr);

protected:
	static void SetupSockets();

	static void __DeleteSocket(uptr_t item, void *context);
	static void __AcceptSocket(ASocketServer *server, int socket, void *context);
	static void __DeleteWriteSocket(uptr_t item, void *context);
	static void CloseSocket(int socket);

	void *FindSocket(int socket) const;
	void AcceptSocket(int socket);
	void DeleteSockets(bool force = false);
	void SetNonBlocking(int socket);
	void SetNoDelay(int socket);

	typedef struct {
		int  socket;
		uint_t type;
		void (*connectcallback)(ASocketServer *server, int socket, void *context);
		void (*readcallback)(ASocketServer *server, int socket, void *context);
		void (*writecallback)(ASocketServer *server, int socket, void *context);
		void (*destructor)(ASocketServer *server, int socket, void *context);
		bool (*needwritecallback)(ASocketServer *server, int socket, void *context);
		void *context;
		struct sockaddr_in sockaddr;
		struct sockaddr from;
		struct sockaddr to;
	} HANDLER;

	typedef struct {
		HANDLER handler;
		HANDLER acceptedhandler;
	} ACCEPTHANDLER;

	typedef struct {
		uint8_t *buffer;
		uint_t pos, len;
		struct sockaddr dest;
	} WRITEBUFFER;

protected:
	ADataList SocketList;
	ADataList DeleteSocketList;
	ADataList WriteSocketList;
	uint_t	  MaxSendBuffer;
	uint_t	  ProcessingDepth;
	void      *readfds, *writefds;

	static uint8_t staticbuf[4096];
};

#endif
