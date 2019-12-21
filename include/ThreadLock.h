#ifndef __SIMPLE_THREAD_LOCK__
#define __SIMPLE_THREAD_LOCK__

#include <pthread.h>

/*--------------------------------------------------------------------------------*/
/** A class to allow thread locking without the hassle of remembering to unlock the thread
 *
 * Simple add a ThreadLockObject to your class and then whenever you want to lock a resource
 * within the class use a ThreadLock object.  On construction of the ThreadLock object the
 * ThreadLockObject will be locked and on destruction if the ThreadLock object the
 * ThreadLockObject will be unlocked
 */
/*--------------------------------------------------------------------------------*/
class AThreadLockObject
{
public:
    AThreadLockObject();
    virtual ~AThreadLockObject();

    /*--------------------------------------------------------------------------------*/
    /** Explicit lock of mutex (AVOID: use a ThreadLock object)
     */
    /*--------------------------------------------------------------------------------*/
    virtual bool Lock();

    /*--------------------------------------------------------------------------------*/
    /** Explicit unlock of mutex (AVOID: use a ThreadLock object)
     */
    /*--------------------------------------------------------------------------------*/
    virtual bool Unlock();

protected:
    pthread_mutex_t mutex;
};

/*--------------------------------------------------------------------------------*/
/** Locking object - use this when you want to lock a ThreadLockObject
 */
/*--------------------------------------------------------------------------------*/
class AThreadLock
{
public:
    /*--------------------------------------------------------------------------------*/
    /** Constructor locks ThreadLockObject
     */
    /*--------------------------------------------------------------------------------*/
    AThreadLock(AThreadLockObject& lockobj) : obj(lockobj) {obj.Lock();}
    /*--------------------------------------------------------------------------------*/
    /** Const constructor to allow use in const methods
     */
    /*--------------------------------------------------------------------------------*/
    AThreadLock(const AThreadLockObject& lockobj) : obj(const_cast<AThreadLockObject&>(lockobj)) {obj.Lock();}
    /*--------------------------------------------------------------------------------*/
    /** Destructor unlocks ThreadLockObject
     */
    /*--------------------------------------------------------------------------------*/
    ~AThreadLock() {obj.Unlock();}

protected:
    AThreadLockObject& obj;
};

/*--------------------------------------------------------------------------------*/
/** Thread signalling base class - AVOID using as it doesn't handle initial conditions
 * Use AThreadBoolSignalObject for boolean conditions instead
 */
/*--------------------------------------------------------------------------------*/
class AThreadSignalObject : public AThreadLockObject
{
public:
    AThreadSignalObject();
    virtual ~AThreadSignalObject();

    /*--------------------------------------------------------------------------------*/
    /** Wait for condition to be triggered
     *
     * @note condition MUST NOT be set or this will wait forever!
     */
    /*--------------------------------------------------------------------------------*/
    virtual bool Wait();

    /*--------------------------------------------------------------------------------*/
    /** Signal first waiting thread
     */
    /*--------------------------------------------------------------------------------*/
    virtual bool Signal();

    /*--------------------------------------------------------------------------------*/
    /** Signal all waiting threads
     */
    /*--------------------------------------------------------------------------------*/
    virtual bool Broadcast();

protected:
    pthread_cond_t cond;
};

/*--------------------------------------------------------------------------------*/
/** Improvement on the above which allows proper handling of the 'signalled before wait' situation
 */
/*--------------------------------------------------------------------------------*/
class AThreadBoolSignalObject : public AThreadSignalObject
{
public:
    AThreadBoolSignalObject(bool initial_condition = false);
    virtual ~AThreadBoolSignalObject();

    /*--------------------------------------------------------------------------------*/
    /** Wait for condition to be true
     */
    /*--------------------------------------------------------------------------------*/
    virtual bool Wait();

    /*--------------------------------------------------------------------------------*/
    /** Signal first waiting thread
     */
    /*--------------------------------------------------------------------------------*/
    virtual bool Signal();

    /*--------------------------------------------------------------------------------*/
    /** Signal all waiting threads
     */
    /*--------------------------------------------------------------------------------*/
    virtual bool Broadcast();

protected:
    volatile bool condition;
};

#endif
