
#ifndef __QUIT_HANDLER__
#define __QUIT_HANDLER__

#include <vector>

#include "misc.h"

class AQuitHandler;
typedef bool (*quitfunc_t)(AQuitHandler *pHandler, void *pContext);

extern bool HasQuit();

class AQuitHandler {
public:
    AQuitHandler(quitfunc_t func = NULL, void *context = NULL);
    ~AQuitHandler();

    void AddHandler(quitfunc_t func, void *context = NULL);
    void RemoveHandler(quitfunc_t func, void *context = NULL);

    bool HasQuit() const {return bHasQuit;}

    void ClearQuit() {bHasQuit = false;}
    void ForceQuit() {bHasQuit = true;}

    static AQuitHandler *GetQuitHandler() {return pDefaultHandler;}

protected:
    static void __Signal(int sig);
    void Signal(int sig);

    typedef struct {
        quitfunc_t pFunc;
        void       *pContext;
    } quitfunc_list_entry_t;

protected:
    std::vector<quitfunc_list_entry_t> quitfuncs;
    bool                               bHasQuit;

    static AQuitHandler *pDefaultHandler;
};

#endif
