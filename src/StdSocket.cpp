
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "StdSocket.h"

NODETYPE_IMPLEMENT(AStdSocket);

AStdSocket::AStdSocket(ASocketServer& iserver) : AStdData(),
												 server(iserver),
												 client(NULL),
												 buffer(NULL),
												 socket(-1),
												 bufferpos(0),
												 bufferlen(0),
												 isserver(false),
												 socketclosed(false)
{
}

AStdSocket::~AStdSocket()
{
	close();
}

bool AStdSocket::open(int isocket)
{
	bool success = false;

	if (!isopen()) {
		socket = isocket;
		socketclosed = false;

		server.SetReadHandler(socket, &__readcallback);
		server.SetDestructor(socket, &__destructor);
		server.SetContext(socket, this);

		success = true;
	}

	return success;
}

bool AStdSocket::open(const char *host, uint_t port, uint_t type)
{
	bool success = false;

	if (!isopen()) {
		isserver = (type == ASocketServer::Type_Server);

		socketclosed = false;

		if ((socket = server.CreateHandler(type,
										   host,
										   port,
										   isserver ? &__connectcallback : NULL,
										   isserver ? NULL : &__readcallback,
										   NULL,
										   isserver ? NULL : &__destructor,
										   NULL,
										   this)) >= 0) {
			success = true;
		}
	}

	if (!success) close();

	return success;
}

sint_t AStdSocket::close()
{
	//debug("AStdSocket<%08lx>: close client %08lx socket %d\n", (uptr_t)this, (uint32_t)client, socket);

	flush();

	if (client) {
		delete client;
		client = NULL;
	}

	if (isopen()) {
		server.DeleteHandler(socket);
		server.Process(0);
		socket = -1;
	}

	if (buffer) {
		delete[] buffer;
		buffer = NULL;
	}

	bufferpos = bufferlen    = 0;
	isserver  = socketclosed = false;

	return 0;
}

void AStdSocket::connectcallback(ASocketServer *server, int socket)
{
	if (!client) {
		if ((client = new AStdSocket(*server)) != NULL) {
			if (client->open(socket)) {
				//debug("AStdSocket<%08lx>: Client connected, socket = %d\n", (uptr_t)this, socket);
			}
			else {
				delete client;
				client = NULL;
			}
		}
		else server->DeleteHandler(socket);
	}
	else {
		//debug("AStdSocket<%08lx>: Client rejected because client = %08lx\n", (uptr_t)this, (uint32_t)client);
		server->DeleteHandler(socket);
	}
}

void AStdSocket::readcallback(ASocketServer *server, int socket)
{
	int bytes;

	if ((bytes = server->BytesAvailable(socket)) > 0) {
		uint_t newbuflen = bufferpos + bytes;

		if (!buffer) {
			if ((buffer = new uint8_t[newbuflen]) != NULL) {
				bufferlen = newbuflen;
			}
		}
		else if (newbuflen > bufferlen) {
			uint8_t *newbuf;

			if ((newbuf = new uint8_t[newbuflen]) != NULL) {
				if (bufferpos) memcpy(newbuf, buffer, bufferpos);
				delete[] buffer;
				buffer  = newbuf;
				bufferlen = newbuflen;
			}
		}

		if ((bytes = server->ReadSocket(socket, buffer + bufferpos, bytes)) > 0) {
			bufferpos += bytes;
		}
		else server->DeleteHandler(socket);
	}
	else server->DeleteHandler(socket);
}

void AStdSocket::destructor(ASocketServer *server, int socket)
{
	//debug("AStdSocket<%08lx>: destructor\n", (uptr_t)this);
	UNUSED(server);
	UNUSED(socket);

	socketclosed = true;
}

slong_t AStdSocket::bytesavailable()
{
	slong_t res = -1;

	if (isopen()) {
		server.Process(0);

		if (client && ((res = client->bytesavailable()) == 0)) {
			delete client;
			client = NULL;
		}

		if (!client) {
			res = bufferpos;
			if ((res == 0) && socketclosed) res = -1;
		}

		//debug("AStdSocket<%08lx>: bytesavailable() client %08lx res %1d\n", (uptr_t)this, (uint32_t)client, res);
	}

	return res;
}

slong_t AStdSocket::bytesqueued()
{
	slong_t res = -1;

	if (isopen()) {
		server.Process(0);

		if (client && ((res = client->bytesqueued()) == 0)) {
			delete client;
			client = NULL;
		}

		if (!client) {
			res = server.BytesLeftToWrite(socket);
			if ((res == 0) && socketclosed) res = -1;
		}
	}

	return res;
}

sint_t AStdSocket::flush()
{
	sint_t res = -1;

	if (isopen()) {
		if (client) res = client->flush();
		else {
			res = 0;

			while (server.BytesLeftToWrite(socket) > 0) {
				res = server.Process(500);
				if (res < 0) break;
			}
			if ((res == 0) && socketclosed) res = -1;
		}
	}

	return res;
}

slong_t AStdSocket::readdata(void *buf, size_t bytes)
{
	slong_t res = -1;

	if (isopen()) {
		if (client && ((res = client->readdata(buf, bytes)) < 0)) {
			//debug("Shutting down client\n");
			delete client;
			client = NULL;
		}

		if (!client) {
			res = MIN(bytes, (size_t)bufferpos);
			if (res) memcpy((uint8_t *)buf, buffer, res);
			bufferpos -= res;
			if (bufferpos) memmove(buffer, buffer + res, bufferpos);
			if ((res == 0) && socketclosed) res = -1;
		}
	}

	return res;
}

slong_t AStdSocket::writedata(const void *buf, size_t bytes)
{
	slong_t res = -1;

	if (isopen()) {
		//debug("AStdSocket<%08lx>: writedata\n", (uptr_t)this);

		if (client && ((res = client->writedata(buf, bytes)) < 0)) {
			//debug("Shutting down client\n");
			delete client;
			client = NULL;
		}

		if (!client) {
			if (socketclosed)  res = -1;
			else if (isserver) res = 0;
			else			   res = server.WriteSocket(socket, (const uint8_t *)buf, bytes);
		}

		server.Process(0);

		if (client && !client->isopen()) {
			//debug("Shutting down client\n");
			delete client;
			client = NULL;
		}
	}

	return res;
}

const sockaddr_in *AStdSocket::getdatagramsource() const
{
	const sockaddr_in *from = NULL;
	
	if (isopen()) from = server.GetDatagramSource(socket);

	return from;
}

bool AStdSocket::setdatagramdestination(const char *host, uint_t port)
{
	struct sockaddr_in to;
	bool success = false;
	
	if (isopen()) {
		if (ASocketServer::Resolve(host, port, &to)) {
			success = setdatagramdestination(&to);
		}
	}
}

bool AStdSocket::setdatagramdestination(const sockaddr_in *to)
{
	bool success = false;
	
	if (isopen()) {
		success = server.SetDatagramDestination(socket, to);
	}

	return success;
}

