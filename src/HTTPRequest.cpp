
#include "HTTPRequest.h"

AHTTPRequest::AHTTPRequest(ASocketServer *_server) : ASocketServer::Handler(_server),
													 port(80),
													 sendrequest(false),
													 complete(false),
													 pos(0)
{
}

AHTTPRequest::AHTTPRequest(ASocketServer *_server, const AString& _url) : ASocketServer::Handler(_server),
																		  port(80),
																		  sendrequest(false),
																		  complete(false),
																		  pos(0)
{
	OpenURL(_url);
}

AHTTPRequest::~AHTTPRequest()
{
	Close();
}

bool AHTTPRequest::OpenURL(const AString& _url)
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
	pos = 0;
}

void AHTTPRequest::OnRead()
{
	uint_t maxlen = pos + 65536;
	sint_t bytes;

	if (data.size() < maxlen) data.resize(maxlen);

	if ((bytes = server->ReadSocket(socket, &data[pos], data.size() - pos)) > 0) {
		pos += bytes;

		ProcessData();
	}
	else {
		debug("Failed to read %u bytes from socket %d\n", (uint_t)(data.size() - pos), socket);
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
	pos = 0;
	sendrequest = false;
}

void AHTTPRequest::RemoveBytes(uint_t n)
{
	n = MIN(n, pos);
	pos -= n;
	if (pos) memmove(&data[0], &data[n], pos);
}
  
bool AHTTPRequest::GetHeader(AString& header, uint_t maxlen)
{
	AString str((const char *)&data[0], MIN(pos, maxlen));
	bool success = false;
	int  p;
		
	if ((p = str.Pos("\r\n")) >= 0) {
		header = str.Left(p); p += 2;

		RemoveBytes(p);
		
		success = true;
	}

	return success;
}

