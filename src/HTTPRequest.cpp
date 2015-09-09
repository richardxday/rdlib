
#include "HTTPRequest.h"

AHTTPRequest::AHTTPRequest(ASocketServer *_server) : ASocketServer::Handler(_server),
													 port(80),
													 sendrequest(false),
													 complete(false)
{
}

AHTTPRequest::AHTTPRequest(ASocketServer *_server, const AString& _url) : ASocketServer::Handler(_server),
																		  port(80),
																		  sendrequest(false),
																		  complete(false)
{
	Open(_url);
}

AHTTPRequest::~AHTTPRequest()
{
	Close();
}

bool AHTTPRequest::Open(const AString& _url)
{
	int p;
	
	url  = _url;
	port = 80;
	complete = false;

	if ((p = url.Pos(":")) > 0) {
		host = url.Left(p);
		port = (uint_t)url.Mid(p + 1);
	}
	else if ((p = url.Pos("/")) > 0) {
		host = url.Left(p);
	}
	else host = url;

	return ASocketServer::Handler::Open(host, port);
}

void AHTTPRequest::OnConnect()
{
	debug("Connected to %s:%u, sending request...\n", host.str(), port);
	sendrequest = true;
	complete = false;
}

void AHTTPRequest::OnRead()
{
	sint_t bytes;

	if ((bytes = server->BytesAvailable(socket)) > 0) {
		uint_t len = data.size();
		sint_t bytes1;
		
		data.resize(len + bytes);
		if ((bytes1 = server->ReadSocket(socket, &data[len], bytes)) > 0) {
			data.resize(len + bytes1);

			ProcessData();
		}
		else {
			debug("Failed to read %d bytes from socket %d\n", bytes, socket);
			Close();
		}
	}
	else {
		//debug("Connection to %s:%u closed%s\n", host.str(), port, sendrequest ? " (BEFORE request sent)" : "");
		Close();
	}
}

void AHTTPRequest::OnWrite()
{
	if (sendrequest) {
		AString req;
		
		req.printf("GET /%s", url.str());

		sendrequest = false;

		//debug("Sending '%s' to %s:%u\n", req.str(), host.str(), port);

		req += "\r\n\r\n";
		
		if (server->WriteSocket(socket, req) < 0) {
			debug("Failed to write %d bytes to socket %d\n", req.len(), socket);
			Close();
		}
	}
}

void AHTTPRequest::Cleanup()
{
	if (IsOpen()) {
		//debug("Request '%s' returned %u bytes\n", url.str(), (uint_t)data.size());
		complete = (data.size() != 0);
		ASocketServer::Handler::Cleanup();
	}

	data.resize(0);
	sendrequest = false;
}

bool AHTTPRequest::GetHeader(AString& header, uint_t maxlen)
{
	AString str((const char *)&data[0], MIN(data.size(), maxlen));
	bool success = false;
	int  p;
		
	if ((p = str.Pos("\r\n")) >= 0) {
		header = str.Left(p); p += 2;
			
		memmove(&data[0], &data[p], data.size() - p);
		data.resize(data.size() - p);

		success = true;
	}

	return success;
}

