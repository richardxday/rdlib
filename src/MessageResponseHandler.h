
#ifndef __MESSAGE_RESPONSE_HANDLER__
#define __MESSAGE_RESPONSE_HANDLER__

#include "StdUri.h"

class AMessageResponseHandler {
public:
	AMessageResponseHandler(uint_t maxresponse = 256, uint32_t deftimeout = 2000, uint32_t defmaxgap = 200);
	virtual ~AMessageResponseHandler();

	virtual bool Open(const AString& uri);
	bool IsOpen() const {return comms.isopen();}
	virtual void Close();

	uint32_t GetTimeout() const {return timeout;}
	void   SetTimeout(uint32_t val) {timeout = val;}

	uint32_t GetMaxGap() const {return maxgap;}
	void   SetMaxGap(uint32_t val) {maxgap = val;}

	typedef void (*RESPONSECALLBACK)(AMessageResponseHandler& controller, bool success, const uint8_t *response, uint_t responsebytes, void *context);
	virtual bool SendMessage(const uint8_t *msg, uint_t msgbytes, RESPONSECALLBACK callback, uint_t minrespbytes, uint_t maxrespbytes = 0, void *context = NULL);
	virtual bool ContinueMessage(RESPONSECALLBACK callback, uint_t minrespbytes, uint_t maxrespbytes = 0, void *context = NULL);

	virtual void Process();

	bool InProgress() const {return msgvalid;}
	
	const uint8_t *GetResponse() const {return buffer;}
	uint_t GetResponseSize() const {return pos;}

protected:
	AStdUri			 comms;
	uint32_t			 timeout, maxgap;
	uint32_t			 tick, rxtick;
	uint_t   			 minbytes;
	uint_t			 maxbytes;
	uint8_t *buffer;
	uint_t   			 pos, buflen;
	RESPONSECALLBACK callback;
	void             *context;
	bool		     msgvalid;
};

#endif
