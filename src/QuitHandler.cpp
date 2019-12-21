
#include <stdio.h>
#include <signal.h>

#include "QuitHandler.h"

/* end of includes */

AQuitHandler *AQuitHandler::pDefaultHandler = NULL;

bool HasQuit()
{
    AQuitHandler *handler = AQuitHandler::GetQuitHandler();
    return (handler ? handler->HasQuit() : false);
}

AQuitHandler::AQuitHandler(QUITFUNC func, void *context) : pFunc(func),
                                                           pContext(context),
                                                           bHasQuit(false)
{
    signal(SIGINT, __Signal);

    if (!pDefaultHandler) pDefaultHandler = this;
}

AQuitHandler::~AQuitHandler()
{
    if (pDefaultHandler == this) pDefaultHandler = NULL;
}

void AQuitHandler::__Signal(int sig)
{
    if (pDefaultHandler) pDefaultHandler->Signal(sig);
}

void AQuitHandler::Signal(int sig)
{
    if (sig == SIGINT) {
        if (pFunc) bHasQuit = (*pFunc)(this, pContext);
        else       bHasQuit = true;
    }
}
