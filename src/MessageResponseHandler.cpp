
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MessageResponseHandler.h"

AMessageResponseHandler::AMessageResponseHandler(uint_t maxresponse, uint32_t deftimeout, uint32_t defmaxgap) : timeout(deftimeout),
																										  maxgap(defmaxgap),
																										  tick(0),
																										  rxtick(0),
																										  minbytes(0),
																										  maxbytes(0),
																										  buffer(new uint8_t[maxresponse]),
																										  pos(0),
																										  buflen(maxresponse),
																										  callback(NULL),
																										  context(NULL),
																										  msgvalid(false)
{
}
									   
AMessageResponseHandler::~AMessageResponseHandler()
{
	Close();

	if (buffer) delete[] buffer;
}

bool AMessageResponseHandler::Open(const AString& uri)
{
	bool success = false;

	if (!IsOpen() && comms.open(uri)) {
		msgvalid = false;
		success  = true;
	}

	return success;
}

void AMessageResponseHandler::Close()
{
	comms.close();
	msgvalid = false;
}

bool AMessageResponseHandler::SendMessage(const uint8_t *msg, uint_t msgbytes, RESPONSECALLBACK callback, uint_t minrespbytes, uint_t maxrespbytes, void *context)
{
	bool success = false;

	if (IsOpen() && !msgvalid) {
		sint32_t n;

		if (!msg || !msgbytes || ((n = comms.writebytes(msg, msgbytes)) == (sint32_t)msgbytes)) {
			minbytes = minrespbytes;
			maxbytes = LIMIT(maxrespbytes, minbytes, buflen);
			this->callback = callback;
			this->context  = context;
			msgvalid = (maxbytes > 0);
			tick     = GetTickCount();
			pos      = 0;
			success  = true;
		}
		else Close();
	}

	return success;
}

bool AMessageResponseHandler::ContinueMessage(RESPONSECALLBACK callback, uint_t minrespbytes, uint_t maxrespbytes, void *context)
{
	return SendMessage(NULL, 0, callback, minrespbytes, maxrespbytes, context);
}

void AMessageResponseHandler::Process()
{
	if (IsOpen() && msgvalid) {
		sint32_t n;

		if ((n = comms.readbytes(buffer + pos, maxbytes - pos)) > 0) {
			rxtick = GetTickCount();
			pos   += n;
		}
		
		if (n < 0) {
			if (callback) (*callback)(*this, false, NULL, 0, context);
			Close();
		}
		else if ((pos == maxbytes) ||
				 (((GetTickCount() - tick) >= timeout) &&
				  (!pos || ((GetTickCount() - rxtick) >= maxgap)))) {
			msgvalid = false;
			if (callback) (*callback)(*this, (pos >= minbytes), buffer, pos, context);
		}
	}
}
