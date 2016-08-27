
#include <string.h>

#include <errno.h>

#include "misc.h"
#include "ThreadLock.h"

AThreadLockObject::AThreadLockObject()
{
	pthread_mutexattr_t mta;

	pthread_mutexattr_init(&mta);
	/* or PTHREAD_MUTEX_RECURSIVE_NP */
	pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);     // MUST be recursive!

	if (pthread_mutex_init(&mutex, &mta) != 0)
	{
		debug("Failed to initialise mutex<%016lx>: %s", (ulong_t)&mutex, strerror(errno));
	}
}

AThreadLockObject::~AThreadLockObject()
{
	pthread_mutex_destroy(&mutex);
}

bool AThreadLockObject::Lock()
{
	bool success = (pthread_mutex_lock(&mutex) == 0);

	if (!success) debug("Failed to lock mutex<%016lx>: %s", (ulong_t)&mutex, strerror(errno));

	return success;
}

bool AThreadLockObject::Unlock()
{
	bool success = (pthread_mutex_unlock(&mutex) == 0);

	if (!success) debug("Failed to unlock mutex<%016lx>: %s", (ulong_t)&mutex, strerror(errno));

	return success;
}

/*----------------------------------------------------------------------------------------------------*/

AThreadSignalObject::AThreadSignalObject() : AThreadLockObject()
{
	if (pthread_cond_init(&cond, NULL) != 0)
	{
		debug("Failed to initialise cond<%016lx>: %s", (ulong_t)&cond, strerror(errno));
	}
}

AThreadSignalObject::~AThreadSignalObject()
{
	pthread_cond_destroy(&cond);
}

bool AThreadSignalObject::Wait()
{
	// NOTE: mutex MUST be LOCKED at this point
	bool success = (pthread_cond_wait(&cond, &mutex) == 0);

	if (!success) debug("Failed to wait on cond<%016lx>: %s", (ulong_t)&cond, strerror(errno));

	return success;
}

bool AThreadSignalObject::Signal()
{
	AThreadLock lock(*this);
	bool success = (pthread_cond_signal(&cond) == 0);

	if (!success) debug("Failed to signal cond<%016lx>: %s", (ulong_t)&cond, strerror(errno));

	return success;
}

bool AThreadSignalObject::Broadcast()
{
	AThreadLock lock(*this);
	bool success = (pthread_cond_broadcast(&cond) == 0);

	if (!success) debug("Failed to broadcast to cond<%016lx>: %s", (ulong_t)&cond, strerror(errno));

	return success;
}

/*----------------------------------------------------------------------------------------------------*/

AThreadBoolSignalObject::AThreadBoolSignalObject(bool initial_condition) : AThreadSignalObject(),
																		   condition(initial_condition)
{
}

AThreadBoolSignalObject::~AThreadBoolSignalObject()
{
}

bool AThreadBoolSignalObject::Wait()
{
	AThreadLock lock(*this);
	while (!condition)
	{
		if (!AThreadSignalObject::Wait()) return false;
	}
	condition = false;
	return true;
}

bool AThreadBoolSignalObject::Signal()
{
	AThreadLock lock(*this);
	condition = true;
	return AThreadSignalObject::Signal();
}

bool AThreadBoolSignalObject::Broadcast()
{
	AThreadLock lock(*this);
	condition = true;
	return AThreadSignalObject::Broadcast();
}
