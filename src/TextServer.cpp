
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "TextServer.h"

ATextServer::ATextServer(ASocketServer& iserver) : server(iserver),
												   creator(NULL),
												   creator_context(NULL),
												   socket(-1),
												   closewhendone(false)
{
	clientlist.SetDestructor(&__DeleteHandler);
}

ATextServer::~ATextServer()
{
	Delete(true);
}

bool ATextServer::Create(uint_t port, ATextHandler *(*icreator)(ATextServer& server, int socket, void *context), void *context, const char *bindaddress)
{
	bool success = false;

	if ((socket = server.CreateHandler(ASocketServer::Type_Server,
									   bindaddress,
									   port,
									   &__connectcallback,
									   &__readcallback,
									   &__writecallback,
									   &__destructor,
									   &__needwritecallback,
									   this)) >= 0) {
		creator         = icreator;
		creator_context = context;
		success         = true;
	}
	
	return success;
}

void ATextServer::Delete(bool force)
{
	uint_t i;

	for (i = 0; i < clientlist.Count(); i++) {
		ATextHandler *handler = (ATextHandler *)clientlist[i];

		handler->Close();
	}

	if (force || (clientlist.Count() == 0)) {
		if (socket >= 0) {
			server.DeleteHandler(socket);
			socket = -1;
		}
		creator         = NULL;
		creator_context = NULL;
		closewhendone   = false;
	}
	else {
		//debug("Cannot delete server yet - still have %u clients\n", clientlist.Count());
		closewhendone = true;
	}
}

void ATextServer::Process(uint_t timeout)
{
	server.Process(timeout);
}

void ATextServer::connectcallback(ASocketServer *server, int socket)
{
	if (creator) {
		ATextHandler *handler;

		if ((handler = (*creator)(*this, socket, creator_context)) != NULL) {
			clientlist.Add((uptr_t)handler);
		}
		else {
			debug("Failed to create connection\n");
			server->SetContext(socket, NULL);
			server->DeleteHandler(socket);
		}
	}
	else {
		debug("No creator for connection!\n");
		server->SetContext(socket, NULL);
		server->DeleteHandler(socket);
	}
}

void ATextServer::__readcallback(ASocketServer *server, int socket, void *context)
{
	ATextHandler *handler = (ATextHandler *)context;

	if (handler) {
		static uint8_t buffer[1024];
		int n;

		if ((n = server->ReadSocket(socket, buffer, sizeof(buffer))) > 0) {
			AString data = AString((const char *)buffer, n).SearchAndReplace("\r", "");

			handler->AddData(data);
		}
		else {
			debug("Failed to read data from socket\n");
			server->DeleteHandler(socket);
			return;
		}

		handler->SendData();
	}
	else {
		debug("Received data on an unregistered handler!\n");
		server->DeleteHandler(socket);
	}
}

void ATextServer::__writecallback(ASocketServer *server, int socket, void *context)
{
	ATextHandler *handler = (ATextHandler *)context;

	if (handler) {
		handler->SendData();
	}
	else {
		debug("Writing data from an unregistered handler!\n");
		server->DeleteHandler(socket);
	}
}

void ATextServer::__destructor(ASocketServer *server, int socket, void *context)
{
	ATextHandler *handler = (ATextHandler *)context;

	UNUSED(server);
	UNUSED(socket);

	if (handler) {
		ATextServer& server = handler->GetServer();

		server.clientlist.Remove((uptr_t)handler);
		delete handler;

		if (server.closewhendone) server.Delete();
	}
}

bool ATextServer::__needwritecallback(ASocketServer *server, int socket, void *context)
{
	ATextHandler *handler = (ATextHandler *)context;
	bool need = false;

	UNUSED(server);

	if (handler) {
		need = handler->CanSendData();
		handler->CloseIfNeeded();
	}
	else {
		debug("Writing data from an unregistered handler!\n");
		server->DeleteHandler(socket);
	}

	return need;
}

void ATextServer::__DeleteHandler(uptr_t item, void *context)
{
	ATextHandler *handler = (ATextHandler *)item;

	UNUSED(context);

	if (handler) delete handler;
}

/*----------------------------------------------------------------------------------------------------*/

ATextHandler::ATextHandler(ATextServer& iserver, int isocket) : server(iserver),
																socket(isocket),
																closewhendone(false)
{
	server.GetSocketServer().SetContext(socket, this);
}

ATextHandler::~ATextHandler()
{
	Close();
}

void ATextHandler::AddData(const AString& str)
{
	recvdata += str;

	int p;
	while ((p = recvdata.Pos("\n")) >= 0) {
		AString command = recvdata.Left(p);

		recvdata = recvdata.Mid(p + 1);

		Process(command);
	}
}

void ATextHandler::Close(bool force)
{
	if (socket >= 0) {
		if (force || (senddata.Empty() && (server.GetSocketServer().BytesLeftToWrite(socket) <= 0))) {
			server.GetSocketServer().DeleteHandler(socket);
			socket = -1;
			closewhendone = false;
		}
		else {
			//debug("Cannot close socket yet - still have %u bytes to send\n", senddata.len() + server.GetSocketServer().BytesLeftToWrite(socket));
			closewhendone = true;
		}
	}
}

void ATextHandler::Write(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	senddata.vprintf(fmt, ap);
	va_end(ap);

	SendData();
}

void ATextHandler::WriteToOthers(const char *fmt, ...)
{
	AString str;
	va_list ap;

	va_start(ap, fmt);
	str.vprintf(fmt, ap);
	va_end(ap);

	const ADataList& clientlist = server.GetClientList();
	uint_t i;
	for (i = 0; i < clientlist.Count(); i++) {
		ATextHandler *handler = (ATextHandler *)clientlist[i];
		
		if (handler != this) {
			handler->senddata += str;
			handler->SendData();
		}
	}
}

void ATextHandler::SendData()
{
	if (senddata.Valid()) {
		ASocketServer& sserver = server.GetSocketServer();
		int n;

		if ((n = sserver.WriteSocket(socket, senddata)) > 0) {
			senddata = senddata.Mid(n);
		}
		else {
			debug("Failed to write data to socket\n");
			sserver.DeleteHandler(socket);
		}
	}
}

void ATextHandler::Process(const AString& command)
{
	UNUSED(command);
}
