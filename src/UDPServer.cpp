
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "UDPServer.h"

AUDPServer::AUDPServer(ASocketServer& iserver) : server(iserver),
                                                 socket(-1)
{
}

AUDPServer::~AUDPServer()
{
    Delete();
}

bool AUDPServer::Create(const char *bindaddress, uint_t port)
{
    bool success = false;

    if (!IsValid()) {
        if ((socket = server.CreateHandler(ASocketServer::Type_Datagram,
                                           bindaddress,
                                           port,
                                           NULL,
                                           &__readcallback,
                                           NULL,
                                           NULL,
                                           NULL,
                                           this)) >= 0) {
            success = true;
        }
        else debug("Failed to create UDP socket on port %u\n", port);
    }

    return success;
}

void AUDPServer::Delete()
{
    if (socket >= 0) {
        server.DeleteHandler(socket);
        socket = -1;
    }
}

void AUDPServer::Process(uint_t timeout)
{
    server.Process(timeout);
}

void AUDPServer::ReadCallback()
{
    uint8_t *data = NULL;
    int n, datalen = 0;

    while ((n = server.BytesAvailable(socket)) > 0) {
        struct sockaddr sockaddr;

        if (!data || (n > datalen)) {
            if (data) delete[] data;
            datalen = n;
            data = new uint8_t[datalen];
        }

        int n1;
        if ((n1 = server.ReadSocket(socket, data, n, &sockaddr)) > 0) {
            if (!Process(data, n1, &sockaddr)) {
                AString str((const char *)data, n1);

                if (!Process(str, &sockaddr)) {
                    debug("Packet unprocessed!\n");
                }
            }
        }

        if (n1 < 0) {
            debug("UDP socket closed\n");
            Delete();
        }
    }

    if (data) delete[] data;
}

void AUDPServer::Send(const uint8_t *data, uint_t bytes, const struct sockaddr *to)
{
    if (IsValid()) {
        int n;

        if ((n = server.WriteSocket(socket, data, bytes, to)) < (int)bytes) {
            debug("Only wrote %d/%u bytes!\n", n, bytes);
        }
    }
}

void AUDPServer::Send(const AString& data, const struct sockaddr *to)
{
    Send((const uint8_t *)data.str(), data.len(), to);
}

bool AUDPServer::Process(const uint8_t *data, uint_t bytes, const struct sockaddr *from)
{
    UNUSED(data);
    UNUSED(bytes);
    UNUSED(from);
    return false;
}

bool AUDPServer::Process(const AString& data, const struct sockaddr *from)
{
    UNUSED(data);
    UNUSED(from);
    return false;
}
