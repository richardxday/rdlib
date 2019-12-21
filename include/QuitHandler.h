
#ifndef __QUIT_HANDLER__
#define __QUIT_HANDLER__

#include "misc.h"

class AQuitHandler;
typedef bool (*QUITFUNC)(AQuitHandler *pHandler, void *pContext);

extern bool HasQuit();

class AQuitHandler {
public:
    AQuitHandler(QUITFUNC func = NULL, void *context = NULL);
    ~AQuitHandler();

    void SetHandler(QUITFUNC func, void *context = NULL) {pFunc = func; pContext = context;}

    bool HasQuit() const {return bHasQuit;}

    void ClearQuit() {bHasQuit = false;}
    void ForceQuit() {bHasQuit = true;}

    static AQuitHandler *GetQuitHandler() {return pDefaultHandler;}

protected:
    static void __Signal(int sig);
    void Signal(int sig);

protected:
    QUITFUNC pFunc;
    void     *pContext;
    bool     bHasQuit;

    static AQuitHandler *pDefaultHandler;
};

#endif
