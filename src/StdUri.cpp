
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "StdUri.h"
#include "StdSocket.h"
#include "StdSerial.h"
#include "StdFile.h"
#include "StdDev.h"
#include "StdMemFile.h"
#include "strsup.h"

NODETYPE_IMPLEMENT(AStdUri);

AHash AStdUri::globaldefaults;
AHash AStdUri::globaltypehandlers(&AStdUri::__deletehandler);

AStdUri::AStdUri() : AStdData(),
					 handler(NULL),
					 defaults(NULL),
					 typehandlers(NULL)
{
}

AStdUri::~AStdUri()
{
	close();

	if (defaults) delete defaults;
	if (typehandlers) delete typehandlers;
}

void AStdUri::__deletehandler(uptr_t item, void *context)
{
	UNUSED(context);
	delete (TYPEHANDLERDATA *)item;
}

void AStdUri::setdefault(const char *name, uptr_t value)
{
	if (!defaults) defaults = new AHash();

	if (defaults) defaults->Insert(name, value);
}

void AStdUri::addtypehandler(const char *type, TYPEHANDLER handler, void *context)
{
	TYPEHANDLERDATA *ihandler;

	if (!typehandlers) typehandlers = new AHash(&__deletehandler);

	if (typehandlers && ((ihandler = new TYPEHANDLERDATA) != NULL)) {
		ihandler->handler = handler;
		ihandler->context = context;

		typehandlers->Insert(type, (uptr_t)ihandler);
	}
}

void AStdUri::removetypehandler(const char *type)
{
	if (typehandlers) {
		typehandlers->Remove(type);
	}
}

void AStdUri::addglobaltypehandler(const char *type, TYPEHANDLER handler, void *context)
{
	TYPEHANDLERDATA *ihandler;

	if ((ihandler = new TYPEHANDLERDATA) != NULL) {
		ihandler->handler = handler;
		ihandler->context = context;

		globaltypehandlers.Insert(type, (uptr_t)ihandler);
	}
}

void AStdUri::splittypeandaddr(const AString& uri, AString& type, AString& addr)
{
	int p;

	if ((p = uri.Pos("://")) >= 0) {
		type = uri.Left(p).ToLower();
		addr = uri.Mid(p + 3);
	}
	else {
		type.Delete();
		addr = uri;
	}
}

void AStdUri::splitnameandmode(const AString& addr, AString& name, AString& mode)
{
	AString str;

	addr.GetFieldNumber(":for:", 0, name);
	if (addr.GetFieldNumber(":for:", 1, str) >= 0) {
		mode = str;
	}
}

void AStdUri::splitnameandmode(const AString& addr, AString& name, uint_t& mode)
{
	AString str;

	addr.GetFieldNumber(":for:", 0, name);
	if (addr.GetFieldNumber(":for:", 1, str) >= 0) {
		uint_t i;

		mode = 0;

		for (i = 0; str[i]; i++) {
			switch (str[i]) {
				case 'r':
					switch (str[i + 1]) {
						case 'w':
							mode = (mode & ~O_ACCMODE) | O_RDWR;
							i++;
							break;

						case 'o':
							mode = (mode & ~O_ACCMODE) | O_RDONLY;
							i++;
							break;

						default:
							mode = (mode & ~O_ACCMODE) | O_RDONLY;
							break;
					}
					break;

				case 'w':
					switch (str[i + 1]) {
						case 'r':
							mode = (mode & ~O_ACCMODE) | O_RDWR;
							i++;
							break;

						case 'o':
							mode = (mode & ~O_ACCMODE) | O_WRONLY;
							i++;
							break;

						default:
							mode = (mode & ~O_ACCMODE) | O_WRONLY;
							break;
					}
					break;

				case 'c':
					mode |= O_CREAT;
					break;

				case 'a':
					mode |= O_APPEND;
					break;

				case 't':
					mode |= O_TRUNC;
					break;

				case 's':
#ifdef O_SYNC
					mode |= O_SYNC;
#endif
					break;

				case 'n':
#ifdef O_NONBLOCK
					mode |= O_NONBLOCK;
#endif
					break;

				case 'x':
#ifdef O_EXCL
					mode |= O_EXCL;
#endif
					break;

				case 'l':
#ifdef O_LARGEFILE
					mode |= O_LARGEFILE;
#endif
					break;

				case 'd':
#ifdef O_DIRECT
					mode |= O_DIRECT;
#endif
					break;

				case '$':
					mode = (uint_t)str.Mid(i);
					i = str.len() - 1;
					break;
			}
		}
	}
}

void AStdUri::splitserialparams(const AString& addr, AString& name, uint_t& baud, uint_t& mode, bool& flow)
{
	int p;

	if ((p = addr.PosNoCase(":baud")) >= 0) baud = (uint_t)addr.Mid(p + 5);
	if ((p = addr.PosNoCase(":8n1")) >= 0) mode = AStdSerial::DataMode_8N1;
	if ((p = addr.PosNoCase(":8n2")) >= 0) mode = AStdSerial::DataMode_8N2;
	if ((p = addr.PosNoCase(":8e1")) >= 0) mode = AStdSerial::DataMode_8E1;
	if ((p = addr.PosNoCase(":8e2")) >= 0) mode = AStdSerial::DataMode_8E2;
	if ((p = addr.PosNoCase(":8o1")) >= 0) mode = AStdSerial::DataMode_8O1;
	if ((p = addr.PosNoCase(":8o2")) >= 0) mode = AStdSerial::DataMode_8O2;
	if ((p = addr.PosNoCase(":hw")) >= 0) flow = true;
	if ((p = addr.PosNoCase(":none")) >= 0) flow = false;

	addr.GetFieldNumber(":", 0, name);
}

void AStdUri::splittcpparams(const AString& addr, AString& host, uint_t& port, uint_t& type)
{
	AString addr1 = addr, str;
	int p;

	if ((p = addr1.PosNoCase(":listen")) >= 0) {
		type = ASocketServer::Type_Server;
		addr1 = addr.Left(p);
	}

	addr1.GetFieldNumber(":", 0, host);
	if (host.Empty()) {
		if (type == ASocketServer::Type_Server) host = "0.0.0.0";
		else							        host = "localhost";
	}

	if (addr1.GetFieldNumber(":", 1, str) >= 0) {
		port = (uint_t)str;
	}
}

void AStdUri::splitudpparams(const AString& addr, AString& host, uint_t& port, AString& desthost, uint_t& destport)
{
	AString addr1, addr2, str;

	addr.GetFieldNumber(";", 0, addr1);
	addr.GetFieldNumber(";", 1, addr2);

	addr1.GetFieldNumber(":", 0, host);
	if (host.Empty()) host = "localhost";
	if (addr1.GetFieldNumber(":", 1, str) >= 0) {
		port = (uint_t)str;
	}

	addr2.GetFieldNumber(":", 0, desthost);
	if (desthost.Empty()) desthost = "localhost";
	if (addr2.GetFieldNumber(":", 1, str) >= 0) {
		destport = (uint_t)str;
	}
}

bool AStdUri::open(const AString& uri, bool writehint)
{
	bool success = false;

	if (!handler) {
		const TYPEHANDLERDATA *typehandler;
		AString type, addr;

		splittypeandaddr(uri, type, addr);

		if (typehandlers && ((typehandler = (const TYPEHANDLERDATA *)typehandlers->Read(type)) != NULL) && (*typehandler->handler)(*this, type, addr, &handler, typehandler->context)) {
			success = (handler != NULL);
		}
		else if (((typehandler = (const TYPEHANDLERDATA *)globaltypehandlers.Read(type)) != NULL) && (*typehandler->handler)(*this, type, addr, &handler, typehandler->context)) {
			success = (handler != NULL);
		}
		else if ((type.Empty() && (uri.Left(5) == "/dev/")) || (type == "dev")) {
			AStdDev *ihandler;
			AString name;
			uint_t mode = writehint ? O_RDWR : O_RDONLY;

			if (addr.Empty()) addr = uri;

			splitnameandmode(addr, name, mode);
			
			if ((ihandler = new AStdDev) != NULL) {
				if (ihandler->open(name, mode)) {
					handler = ihandler;
					success = true;
				}
				else delete ihandler;
			}
		}
		else if (type.Empty() || (type == "file")) {
			AStdFile *ihandler;
			AString name, mode = AString(writehint ? "w" : "r") + AString(getdefault("filebinary", 0) ? "b" : "");

			splitnameandmode(addr, name, mode);
			
			if (name == "stdin") {
				if ((ihandler = new AStdFile(stdin)) != NULL) {
					ihandler->setbinarymode(mode.Pos('b') >= 0);
					handler = ihandler;
					success = true;
				}
			}
			else if (name == "stdout") {
				if ((ihandler = new AStdFile(stdout)) != NULL) {
					ihandler->setbinarymode(mode.Pos('b') >= 0);
					handler = ihandler;
					success = true;
				}
			}
			else if (name == "stderr") {
				if ((ihandler = new AStdFile(stderr)) != NULL) {
					ihandler->setbinarymode(mode.Pos('b') >= 0);
					handler = ihandler;
					success = true;
				}
			}
			else if ((ihandler = new AStdFile) != NULL) {
				if (ihandler->open(name, mode)) {
					handler = ihandler;
					success = true;
				}
				else delete ihandler;
			}
		}
		else if (type == "mem") {
			AStdMemFile *ihandler;
			
			if ((ihandler = new AStdMemFile) != NULL) {
				if (ihandler->open()) {
					handler = ihandler;
					success = true;
				}
				else delete ihandler;
			}
		}
		else if (type == "serial") {
			AStdSerial *ihandler;
			AString name = addr;
			uint_t    baud = getdefault("serialbaud", 9600);
			uint_t    mode = getdefault("serialmode", AStdSerial::DataMode_8N1);
			bool    flow = (getdefault("serialflow", false) != 0);

			splitserialparams(addr, name, baud, mode, flow);

			if ((ihandler = new AStdSerial) != NULL) {
				if (ihandler->open(name, baud, flow, mode)) {
					handler = ihandler;
					success = true;
				}
				else delete ihandler;
			}
		}
		else if (type == "tcp") {
			ASocketServer *server = (ASocketServer *)getdefault("socketserver");
			AStdSocket *ihandler;
			AString host;
			uint_t    port = getdefault("tcpport", 8080);
			uint_t	type = ASocketServer::Type_Client;

			splittcpparams(addr, host, port, type);

			if (server && ((ihandler = new AStdSocket(*server)) != NULL)) {
				if (ihandler->open(host.Valid() ? host.str() : NULL, port, type)) {
					handler = ihandler;
					success = true;
				}
				else delete ihandler;
			}
		}
		else if (type == "udp") {
			ASocketServer *server = (ASocketServer *)getdefault("socketserver");
			AStdSocket *ihandler;
			AString host, desthost;
			uint_t    port = getdefault("udpport", 8080);
			uint_t    destport = getdefault("udpdestport", 8080);

			splitudpparams(addr, host, port, desthost, destport);

			if (server && ((ihandler = new AStdSocket(*server)) != NULL)) {
				if (ihandler->open(host.Valid() ? host.str() : NULL, port, ASocketServer::Type_Datagram)) {
					if (desthost.Valid()) {
						ihandler->setdatagramdestination(desthost, destport);
					}

					handler = ihandler;
					success = true;
				}
				else delete ihandler;
			}
		}
	}

	return success;
}

sint_t AStdUri::close()
{
	if (handler) {
		handler->close();
		delete handler;
		handler = NULL;
	}

	return 0;
}
