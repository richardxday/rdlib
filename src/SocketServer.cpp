
// set maximum number of connections
#define FD_SETSIZE 4096

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __LINUX__
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <limits.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "SocketServer.h"

#ifdef _WIN32
class ASocketSetupData {
public:
	ASocketSetupData() {
		::WSAStartup(MAKEWORD(2, 0), &WSAData);
	}
	~ASocketSetupData() {
		::WSACleanup();
	}

protected:
	WSADATA WSAData;
};
#endif

#define DEBUG_SOCKETS 0

uint8_t ASocketServer::staticbuf[4096];

ASocketServer::ASocketServer() : MaxSendBuffer(512 * 1024),
								 ProcessingDepth(0),
								 readfds(NULL),
								 writefds(NULL),
								 exceptfds(NULL),
								 autocloseunestablishedsockets(false)
{
	SetupSockets();

	readfds   = new fd_set;
	writefds  = new fd_set;
	exceptfds = new fd_set;

	SocketList.SetDestructor(&__DeleteSocket, this);
	WriteSocketList.SetDestructor(&__DeleteWriteSocket);
	DeleteSocketList.EnableDuplication(false);
}

ASocketServer::~ASocketServer()
{
	DeleteAllHandlers();
	DeleteSockets(true);

	if (exceptfds) delete (fd_set *)exceptfds;
	if (writefds)  delete (fd_set *)writefds;
	if (readfds)   delete (fd_set *)readfds;
}

void ASocketServer::SetupSockets()
{
#ifdef _WIN32
	static ASocketSetupData Sockets;
#endif
#ifdef __LINUX__
	signal(SIGPIPE, SIG_IGN);
#endif
}

void ASocketServer::__DeleteSocket(uptr_t item, void *context)
{
	ASocketServer *server = (ASocketServer *)context;
	HANDLER *handler = (HANDLER *)item;

	if (handler->destructor) {
		(*handler->destructor)((ASocketServer *)context, handler->socket, handler->context);
	}

	if (handler->socket >= 0) {
		uint32_t wrsocket;

		if ((wrsocket = server->WriteSocketList[handler->socket]) != 0) {
			__DeleteWriteSocket(wrsocket, NULL);
		}
		server->WriteSocketList.Replace(handler->socket, (uptr_t)0);

		CloseSocket(handler->socket);
	}

	delete handler;
}

void ASocketServer::__AcceptSocket(ASocketServer *server, int socket, void *context)
{
	UNUSED(server);
	((ASocketServer *)context)->AcceptSocket(socket);
}

void ASocketServer::__DeleteWriteSocket(uptr_t item, void *context)
{
	WRITEBUFFER *wrbuf = (WRITEBUFFER *)item;

	UNUSED(context);
	
	if (wrbuf) {
		if (wrbuf->buffer) free(wrbuf->buffer);

		delete wrbuf;
	}
}

void ASocketServer::CloseSocket(int socket)
{
	if (shutdown(socket, 1) >= 0) {
		recv(socket, (char *)staticbuf, sizeof(staticbuf), 0);
	}
#ifdef __LINUX__
	close(socket);
#else
	closesocket(socket);
#endif
}

void ASocketServer::SetNonBlocking(int socket)
{
#ifdef __LINUX__
	fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) | O_NONBLOCK);
#else
	u_long nonblocking = 1;
	ioctlsocket(socket, FIONBIO, &nonblocking);
#endif
}

void ASocketServer::SetNoDelay(int socket)
{
	int nodelay = 1;
	if (setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char *)&nodelay, sizeof(nodelay)) < 0) {
		debug("Failed to set No Delay on socket %d: %s\n", socket, strerror(errno));
	}
}

void ASocketServer::AcceptSocket(int socket)
{
	const ACCEPTHANDLER *accepthandler = (const ACCEPTHANDLER *)FindSocket(socket);
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	int socket1;

	if (!accepthandler) return;

	if ((socket1 = accept(socket, (sockaddr *)&addr, &len)) >= 0) {
		HANDLER *handler;

		SetNonBlocking(socket1);
		SetNoDelay(socket1);

		if ((handler = new HANDLER) != NULL) {
			memset(handler, 0, sizeof(*handler));

			*handler = accepthandler->acceptedhandler;
			handler->socket    = socket1;
			handler->sockaddr  = addr;
			handler->starttick = GetTickCount();
			handler->connected = true;

			SocketList.Add(handler);

#if DEBUG_SOCKETS
			debug("Accepted socket %d from socket %d at %s (context %s)\n", socket1, socket, AValue(handler->starttick).ToString().str(), AValue(handler->context).ToString().str());
#endif
			
			if (handler->connectcallback) (*handler->connectcallback)(this, handler->socket, handler->context);
		}
	}
}

bool ASocketServer::Resolve(const char *host, uint_t port, struct sockaddr_in *sockaddr)
{
	bool success = false;

	SetupSockets();

	memset(sockaddr, 0, sizeof(*sockaddr));
	sockaddr->sin_family = AF_INET;
	sockaddr->sin_port = htons((short) (port & 0xFFFF));

	if (host) {
		sockaddr->sin_addr.s_addr = inet_addr(host);

		if (sockaddr->sin_addr.s_addr == INADDR_NONE) {
			struct hostent *hp = gethostbyname(host);

			if (hp) {
				memcpy(&sockaddr->sin_addr, hp->h_addr, hp->h_length);
				sockaddr->sin_family = hp->h_addrtype;

				success = true;
			}
			else debug("Host '%s' not found\n", host);
		}
	}
	else {
		sockaddr->sin_addr.s_addr = INADDR_ANY;

		success = true;
	}

	return success;
}

int ASocketServer::CreateHandler(uint_t type,
								 const char *host,
								 uint_t port,
								 void (*connectcallback)(ASocketServer *server, int socket, void *context),
								 void (*readcallback)(ASocketServer *server, int socket, void *context),
								 void (*writecallback)(ASocketServer *server, int socket, void *context),
								 void (*destructor)(ASocketServer *server, int socket, void *context),
								 bool (*needwritecallback)(ASocketServer *server, int socket, void *context),
								 void *context)
{
	struct sockaddr_in sockaddr;
	bool resolved;
	int socket = -1;

	resolved = Resolve(host, port, &sockaddr);

	if ((socket = ::socket(sockaddr.sin_family, (type == Type_Datagram) ? SOCK_DGRAM : SOCK_STREAM, 0)) >= 0) {
		if (host && resolved && (type == Type_Client)) {
			SetNonBlocking(socket);
			SetNoDelay(socket);

#ifdef __LINUX__
#define ERROR_IN_PROGRESS (errno == EINPROGRESS)
#else
#define ERROR_IN_PROGRESS (WSAGetLastError() == WSAEWOULDBLOCK)
#endif

			if ((connect(socket, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) >= 0) ||
				ERROR_IN_PROGRESS) {
				HANDLER *handler;

				if ((handler = new HANDLER) != NULL) {
					memset(handler, 0, sizeof(*handler));

					handler->socket        	   = socket;
					handler->type          	   = type;
					handler->connectcallback   = connectcallback;
					handler->readcallback  	   = readcallback;
					handler->writecallback 	   = writecallback;
					handler->destructor    	   = destructor;
					handler->needwritecallback = needwritecallback;
					handler->context       	   = context;
					handler->sockaddr      	   = sockaddr;
					handler->starttick         = GetTickCount();
					handler->connected         = !autocloseunestablishedsockets;

					SocketList.Add(handler);

#if DEBUG_SOCKETS
					debug("New client socket %d at %s (context %s) to %s:%u\n", socket, AValue(handler->starttick).ToString().str(), AValue(handler->context).ToString().str(), host, port);
#endif

					if (handler->connectcallback) (*handler->connectcallback)(this, handler->socket, handler->context);
				}
				else {
					CloseSocket(socket);
					socket = -1;
				}
			}
			else {
				debug("connect on socket %d failed: %s\n", socket, strerror(errno));

				CloseSocket(socket);
				socket = -1;
			}
		}
		else if (type == Type_Server) {
			int rc = 1;

			setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char *)&rc, sizeof(rc));

			if (bind(socket, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) >= 0) {
				if (listen(socket, SOMAXCONN) >= 0) {
					ACCEPTHANDLER *handler;

					if ((handler = new ACCEPTHANDLER) != NULL) {
						memset(handler, 0, sizeof(*handler));

						handler->handler.socket        		   	   = socket;
						handler->handler.type         		   	   = type;
						handler->handler.readcallback 		   	   = &__AcceptSocket;
						handler->handler.context      		   	   = this;
						handler->handler.sockaddr     		   	   = sockaddr;
						handler->handler.starttick         		   = GetTickCount();
						handler->handler.connected         		   = true;

						handler->acceptedhandler.type              = type;
						handler->acceptedhandler.connectcallback   = connectcallback;
						handler->acceptedhandler.readcallback  	   = readcallback;
						handler->acceptedhandler.writecallback 	   = writecallback;
						handler->acceptedhandler.destructor    	   = destructor;
						handler->acceptedhandler.needwritecallback = needwritecallback;
						handler->acceptedhandler.context       	   = context;

#if DEBUG_SOCKETS
						debug("New server socket %d at %s (context %s) port %u\n", socket, AValue(handler->handler.starttick).ToString().str(), AValue(handler->acceptedhandler.context).ToString().str(), port);
#endif

						SocketList.Add(handler);
					}
					else {
						CloseSocket(socket);
						socket = -1;
					}
				}
				else {
					debug("listen on socket %d failed: %s\n", socket, strerror(errno));

					CloseSocket(socket);
					socket = -1;
				}
			}
			else {
				debug("bind on socket %d failed: %s\n", socket, strerror(errno));

				CloseSocket(socket);
				socket = -1;
			}
		}
		else if (type == Type_Datagram) {
			int rc = 1;

			setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char *)&rc, sizeof(rc));

			if (bind(socket, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) >= 0) {
				HANDLER *handler;

				if ((handler = new HANDLER) != NULL) {
					memset(handler, 0, sizeof(*handler));

					handler->socket        	   = socket;
					handler->type          	   = type;
					handler->connectcallback   = connectcallback;
					handler->readcallback  	   = readcallback;
					handler->writecallback 	   = writecallback;
					handler->destructor    	   = destructor;
					handler->needwritecallback = needwritecallback;
					handler->context       	   = context;
					handler->sockaddr      	   = sockaddr;
					handler->starttick         = GetTickCount();
					handler->connected         = true;

					SocketList.Add(handler);

					if (handler->connectcallback) (*handler->connectcallback)(this, handler->socket, handler->context);
				}
				else {
					CloseSocket(socket);
					socket = -1;
				}
			}
			else {
				debug("bind on socket %d failed: %s\n", socket, strerror(errno));

				CloseSocket(socket);
				socket = -1;
			}
		}
		else {
			CloseSocket(socket);
			socket = -1;
		}
		
		if (socket >= 0) {
			SetNonBlocking(socket);
		}
	}
	else {
		debug("Failed to open socket to host %s:%u failed: %s\n", host ? host : "localhost", port, strerror(errno));
	}

	return socket;
}

void ASocketServer::DeleteSockets(bool force)
{
	uint_t i;

	if (ProcessingDepth) {
		// don't allow DeleteSockets() to be called recursively
		return;
	}

	ProcessingDepth++;

	for (i = 0; i < DeleteSocketList.Count();) {
		WRITEBUFFER *wrbuf;
		HANDLER *handler = (HANDLER *)DeleteSocketList[i];
		int socket = handler->socket;

#if DEBUG_SOCKETS
		debug("Deleting socket %d handler (context %s)\n", socket, AValue(handler->context).ToString().str());
#endif

		if (((wrbuf = (WRITEBUFFER *)WriteSocketList[socket]) == NULL) || (wrbuf->pos == 0) || force) {
			__DeleteSocket((uptr_t)handler, this);
			SocketList.Remove((uptr_t)handler);
			DeleteSocketList.Remove((uptr_t)handler);
		}
		else {
			if (handler->destructor) {
				(*handler->destructor)(this, handler->socket, handler->context);
			}

			handler->connectcallback   = NULL;
			handler->readcallback      = NULL;
			handler->writecallback     = NULL;
			handler->destructor        = NULL;
			handler->needwritecallback = NULL;
			handler->context           = NULL;
			i++;
		}
	}

	ProcessingDepth--;
}

void ASocketServer::DeleteAllHandlers()
{
	uint_t i, n = SocketList.Count();

	for (i = 0; i < n; i++) {
		DeleteSocketList.Add(SocketList[i]);
	}

	if (!ProcessingDepth) DeleteSockets();
}

int ASocketServer::Process(uint_t timeout)
{
	// NOT thread safe!!!
	struct timeval tv;
	WRITEBUFFER *wrbuf;
	fd_set& readfds   = *(fd_set *)this->readfds;
	fd_set& writefds  = *(fd_set *)this->writefds;
	fd_set& exceptfds = *(fd_set *)this->exceptfds;
	uint_t i;
	int maxsocket = -1;
	int res = 0;

	// delete any sockets that are to be deleted
	DeleteSockets();

	if (ProcessingDepth) {
		// don't allow Process to be called recursively!!
		return res;
	}

	ProcessingDepth++;

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);

	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;

	for (i = 0; i < SocketList.Count(); i++) {
		const HANDLER *handler = (const HANDLER *)SocketList[i];
		int socket = handler->socket;

		FD_SET(socket, &exceptfds);

		if (handler->readcallback) {
			FD_SET(socket, &readfds);
			maxsocket = MAX(maxsocket, socket);
		}

		if ((((wrbuf = (WRITEBUFFER *)WriteSocketList[socket]) != NULL) && wrbuf->buffer && wrbuf->pos) ||
			(handler->writecallback && (!handler->needwritecallback || (*handler->needwritecallback)(this, socket, handler->context)))) {
			FD_SET(socket, &writefds);
			maxsocket = MAX(maxsocket, socket);
		}
	}

	if (maxsocket >= 0) {
		res = select(maxsocket + 1, &readfds, &writefds, &exceptfds, &tv);

		for (i = 0; i < SocketList.Count(); i++) {
			HANDLER *handler = (HANDLER *)SocketList[i];
			int  socket = handler->socket;

			if (FD_ISSET(socket, &exceptfds)) {
 				DeleteWriteBuffer(socket);
				DeleteHandler(socket);
				continue;
			}

			if (FD_ISSET(socket, &readfds)) {
#if DEBUG_SOCKETS
				if (!handler->connected) {
					debug("Socket %d has become connected (context %s) via read\n", socket, AValue(handler->context).ToString().str());
				}
#endif

				handler->connected = true;
				(*handler->readcallback)(this, socket, handler->context);
			}

			if (FD_ISSET(socket, &writefds)) {
#if DEBUG_SOCKETS
				if (!handler->connected) {
					debug("Socket %d has become connected (context %s) via write\n", socket, AValue(handler->context).ToString().str());
				}
#endif
				
				handler->connected = true;
				if (((wrbuf = (WRITEBUFFER *)WriteSocketList[socket]) != NULL) && wrbuf->buffer && wrbuf->pos) {
					sint_t bytes = -1;

					if (handler->type == Type_Datagram) {
						bytes = sendto(socket, (const char *)wrbuf->buffer, wrbuf->pos, 0, &handler->to, sizeof(handler->to));
					}
					// this will fail if connection not made
					else bytes = send(socket, (const char *)wrbuf->buffer, wrbuf->pos, 0);

					if (bytes < 0) {
						DeleteWriteBuffer(socket);
						DeleteHandler(socket);
					}
					else {
						wrbuf->pos -= bytes;
						if (wrbuf->pos) memmove(wrbuf->buffer, wrbuf->buffer + bytes, wrbuf->pos);
						else {
							free(wrbuf->buffer);
							delete wrbuf;

							WriteSocketList.Replace(socket, (uptr_t)0);
						}
					}
				}
				else if (handler->writecallback) {
					(*handler->writecallback)(this, socket, handler->context);
				}
			}

			if (!handler->connected && ((GetTickCount() - handler->starttick) >= 10000)) {
#if DEBUG_SOCKETS
				debug("Closing socket %d (context %s) because it has not become read or write enabled for 10s\n", socket, AValue(handler->context).ToString().str());
#else
				debug("Closing socket %d because it has not become read or write enabled for 10s\n", socket);
#endif
				
				DeleteWriteBuffer(socket);
				DeleteHandler(socket);
			}
		}
	}

#if 0
	for (i = 0; i < SocketList.Count(); i++) {
		const HANDLER *handler = (const HANDLER *)SocketList[i];
		int socket = handler->socket;

		if (((wrbuf = (WRITEBUFFER *)WriteSocketList[socket]) != NULL) && wrbuf->buffer && wrbuf->pos) {
			debug("Socket %d still has %u bytes to send!\n", socket, wrbuf->pos);
		}
	}
#endif

	ProcessingDepth--;

	// try again to delete sockets
	DeleteSockets();

	return res;
}

void *ASocketServer::FindSocket(int socket) const
{
	uint_t i, n = SocketList.Count();

	for (i = 0; i < n; i++) {
		HANDLER *handler = (HANDLER *)SocketList[i];

		if (socket == handler->socket) {
			return handler;
		}
	}

	return NULL;
}

const struct sockaddr_in *ASocketServer::GetSocketAddr(int socket) const
{
	const struct sockaddr_in *sockaddr = NULL;
	const HANDLER *handler;

	if ((handler = (HANDLER *)FindSocket(socket)) != NULL) {
		sockaddr = &handler->sockaddr;
	}

	return sockaddr;
}

void ASocketServer::SetConnectHandler(int socket, void (*connectcallback)(ASocketServer *server, int socket, void *context))
{
	HANDLER *handler;

	if ((handler = (HANDLER *)FindSocket(socket)) != NULL) {
		handler->connectcallback = connectcallback;
	}
}

void ASocketServer::SetReadHandler(int socket, void (*readcallback)(ASocketServer *server, int socket, void *context))
{
	HANDLER *handler;

	if ((handler = (HANDLER *)FindSocket(socket)) != NULL) {
		handler->readcallback = readcallback;
	}
}

void ASocketServer::SetWriteHandler(int socket, void (*writecallback)(ASocketServer *server, int socket, void *context))
{
	HANDLER *handler;

	if ((handler = (HANDLER *)FindSocket(socket)) != NULL) {
		handler->writecallback = writecallback;
	}
}

void ASocketServer::DeleteWriteBuffer(int socket)
{
	WRITEBUFFER *wrbuf;

	if ((wrbuf = (WRITEBUFFER *)WriteSocketList[socket]) != NULL) {
		free(wrbuf->buffer);
		delete wrbuf;

		WriteSocketList.Replace(socket, (uptr_t)0);
	}
}

void ASocketServer::DeleteHandler(int socket)
{
	HANDLER *handler;

	if ((handler = (HANDLER *)FindSocket(socket)) != NULL) {
		DeleteSocketList.Add((uptr_t)handler);
	}

	if (!ProcessingDepth) DeleteSockets();
}

void ASocketServer::SetDestructor(int socket, void (*destructor)(ASocketServer *server, int socket, void *context))
{
	HANDLER *handler;

	if ((handler = (HANDLER *)FindSocket(socket)) != NULL) {
		handler->destructor = destructor;
	}
}

void ASocketServer::SetNeedWriteHandler(int socket, bool (*needwritecallback)(ASocketServer *server, int socket, void *context))
{
	HANDLER *handler;

	if ((handler = (HANDLER *)FindSocket(socket)) != NULL) {
		handler->needwritecallback = needwritecallback;
	}
}

void ASocketServer::SetContext(int socket, void *context)
{
	HANDLER *handler;

	if ((handler = (HANDLER *)FindSocket(socket)) != NULL) {
		handler->context = context;
	}
}

const struct sockaddr_in *ASocketServer::GetDatagramSource(int socket) const
{
	const struct sockaddr_in *from = NULL;
	const HANDLER *handler;

	if ((handler = (const HANDLER *)FindSocket(socket)) != NULL) {
		from = (const struct sockaddr_in *)&handler->from;
	}

	return from;
}

bool ASocketServer::SetDatagramDestination(int socket, const struct sockaddr_in *to)
{
	HANDLER *handler;
	bool    success = false;

	if ((handler = (HANDLER *)FindSocket(socket)) != NULL) {
		handler->to = *(struct sockaddr *)to;

		success = true;
	}

	return success;
}

sint_t ASocketServer::BytesAvailable(int socket)
{
	if (DeleteSocketList.Find(socket) >= 0) return -1;

#ifdef __LINUX__
	return recv(socket, staticbuf, sizeof(staticbuf), MSG_PEEK);
#else
	u_long bytes1 = 0;
	int bytes = -1;

	if (::ioctlsocket(socket, FIONREAD, &bytes1) != SOCKET_ERROR) {
		bytes = (int)bytes1;
	}

	return bytes;
#endif
}

sint_t ASocketServer::BytesLeftToWrite(int socket)
{
	if (DeleteSocketList.Find(socket) >= 0) return -1;

	int res = 0;
	const WRITEBUFFER *wrbuf;
	if ((wrbuf = (const WRITEBUFFER *)WriteSocketList[socket]) != NULL) {
		res = wrbuf->pos;
	}

	return res;
}

sint_t ASocketServer::ReadSocket(int socket, uint8_t *buffer, uint_t bytes, struct sockaddr *from)
{
	HANDLER *handler = (HANDLER *)FindSocket(socket);
	socklen_t _len;
	sint_t res;

	if ((DeleteSocketList.Find(socket) >= 0) || !handler) return -1;

	if (handler->type == Type_Datagram) {
		res = recvfrom(socket, (char *)buffer, bytes, 0, &handler->from, &_len);
		if (from) *from = handler->from;
	}
	else res = recv(socket, (char *)buffer, bytes, 0);

	return res;
}

sint_t ASocketServer::WriteSocket(int socket, const uint8_t *buffer, uint_t bytes, const struct sockaddr *to)
{
	HANDLER     *handler = (HANDLER *)FindSocket(socket);
	WRITEBUFFER *wrbuf   = (WRITEBUFFER *)WriteSocketList[socket];

	if ((DeleteSocketList.Find(socket) >= 0) || !handler) return -1;

	if (!wrbuf) {
		if ((wrbuf = new WRITEBUFFER) != NULL) {
			memset(wrbuf, 0, sizeof(*wrbuf));
			WriteSocketList.Replace(socket, (uptr_t)wrbuf);
		}
	}

	bytes = MIN(bytes, SUBZ(MaxSendBuffer, wrbuf->pos));

	if (wrbuf && (wrbuf->pos < MaxSendBuffer)) {
		uint_t len = wrbuf->pos + bytes;
		if (len > wrbuf->len) {
			wrbuf->buffer = (uint8_t *)realloc(wrbuf->buffer, len);
			wrbuf->len    = len;
		}

		memcpy(wrbuf->buffer + wrbuf->pos, buffer, bytes);
		wrbuf->pos += bytes;

		if (to) handler->to = *to;
	}

	return bytes;
}

sint_t ASocketServer::ReadLineData(int socket, SOCKETREADER& reader, struct sockaddr *from)
{
	sint_t bytes, totalbytes = 0;

	while ((bytes = ReadSocket(socket, reader.buffer, sizeof(reader.buffer), from)) > 0) {
		AString str((const char *)reader.buffer, bytes);

		reader.lines += str.SearchAndReplace("\r", "");

		totalbytes += bytes;
	}

	return (totalbytes > 0) ? totalbytes : bytes;
}

bool ASocketServer::ReadLine(SOCKETREADER& reader, AString& line)
{
	bool res = false;

	if (reader.lines.Pos("\n", 0) >= 0) {
		line = reader.lines.CutLine(0, "\n", 0);
		res  = true;
	}

	return res;
}

AString ASocketServer::GetClientAddr(int socket) const
{
	const HANDLER *handler = (const HANDLER *)FindSocket(socket);
	AString client;

	if (handler) client = ASocketServer::GetClientAddr(&handler->sockaddr);

	return client;
}

uint_t ASocketServer::GetClientPort(int socket) const
{
	const HANDLER *handler = (const HANDLER *)FindSocket(socket);
	uint_t port = 0;

	if (handler) port = ASocketServer::GetClientPort(&handler->sockaddr);

	return port;
}

AString ASocketServer::GetClientAddr(const struct sockaddr_in *sockaddr)
{
	AString client;

	if (sockaddr && (sockaddr->sin_family == AF_INET)) {
		client = inet_ntoa(sockaddr->sin_addr);
	}

	return client;
}

uint_t ASocketServer::GetClientPort(const struct sockaddr_in *sockaddr)
{
	uint_t port = 0;

	if (sockaddr && (sockaddr->sin_family == AF_INET)) {
		port = ntohs(sockaddr->sin_port);
	}

	return port;
}

bool ASocketServer::SetReceiveBufferSize(int socket, uint_t bytes)
{
	bool success = false;

	if (socket >= 0) {
		int arg = bytes;
		
		success = (setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (const char *)&arg, sizeof(arg)) == 0);
		if (!success) debug("Failed to set receive buffer size of socket %d to %d: %s\n", socket, arg, strerror(errno));
	}
	else debug("No socket when trying to set receiver buffer size!\n");

	return success;
}

bool ASocketServer::Handler::Open(const char *host, uint_t port)
{
	bool success = false;

	if (!IsOpen()) {
		if ((socket = server->CreateHandler(Type_Client,
											host, port,
											&__connectcallback,
											&__readcallback,
											&__writecallback,
											&__destructor,
											&__needwritecallback,
											this)) >= 0) {
			success = true;
		}
	}

	return success;
}

void ASocketServer::Handler::Close()
{
	if (IsOpen()) {
		server->DeleteHandler(socket);
	}
}

bool ASocketServer::Handler::SetReceiveBufferSize(uint_t bytes)
{
	return server && server->SetReceiveBufferSize(socket, bytes);
}
