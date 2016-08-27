
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "misc.h"
#include "Thread.h"

/*--------------------------------------------------------------------------------*/
/** Simple thread implementation
 */
/*--------------------------------------------------------------------------------*/
AThread::AThread() : threadstarted(false),
					 quitthread(false)
{
}

AThread::~AThread()
{
	Stop();
}

/*--------------------------------------------------------------------------------*/
/** Start thread
 */
/*--------------------------------------------------------------------------------*/
bool AThread::Start()
{
	bool success = false;

	if (!threadstarted) {
		memset(&thread, 0, sizeof(thread));
		
		if (pthread_create(&thread, NULL, &__ThreadStart, (void *)this) == 0) {
			threadstarted = true;
			success = true;
		}
		else {
			memset(&thread, 0, sizeof(thread));
			debug("Failed to start thread");
		}
	}
	else debug("Request to start thread that is already running!");
	
	return success;
}

/*--------------------------------------------------------------------------------*/
/** Stop thread
 */
/*--------------------------------------------------------------------------------*/
void AThread::Stop()
{
	if (threadstarted) {
		// tell thread to quit
		quitthread = true;

		// perform any releasing/signalling functions
		ThreadQuitting();
		
		// wait until thread has quit
		pthread_join(thread, NULL);

		// reset internal state
		memset(&thread, 0, sizeof(thread));
		threadstarted = false;
		quitthread = false;
	}
}

/*--------------------------------------------------------------------------------*/
/** Static thread entry point
 */
/*--------------------------------------------------------------------------------*/
void *AThread::__ThreadStart(void *arg)
{
	AThread& thread = *(AThread *)arg;
	return thread.Run();
}

