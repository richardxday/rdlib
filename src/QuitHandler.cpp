
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

AQuitHandler::AQuitHandler(quitfunc_t func, void *context) : bHasQuit(false)
{
    signal(SIGINT, __Signal);

    if (pDefaultHandler == NULL) {
        pDefaultHandler = this;
    }

    if (func != NULL) {
        AddHandler(func, context);
    }
}

AQuitHandler::~AQuitHandler()
{
    if (pDefaultHandler == this) {
        pDefaultHandler = NULL;
    }
}

void AQuitHandler::AddHandler(quitfunc_t func, void *context)
{
    quitfunc_list_entry_t entry = {func, context};
    quitfuncs.push_back(entry);
}

void AQuitHandler::RemoveHandler(quitfunc_t func, void *context)
{
    for (auto it = quitfuncs.begin(); it != quitfuncs.end();) {
        if ((func == it->pFunc) && (context == it->pContext)) {
            it = quitfuncs.erase(it);
        }
        else ++it;
    }
}

void AQuitHandler::__Signal(int sig)
{
    if (pDefaultHandler != NULL) {
        pDefaultHandler->Signal(sig);
    }
}

void AQuitHandler::Signal(int sig)
{
    if (sig == SIGINT) {
        if (quitfuncs.size() > 0) {
            for (auto it = quitfuncs.rbegin(); it != quitfuncs.rend(); ++it) {
                if ((*it->pFunc)(this, it->pContext)) {
                    bHasQuit = true;
                    break;
                }
            }
        }
        else bHasQuit = true;
    }
}
