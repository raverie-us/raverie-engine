// This code is in the public domain -- Ignacio Castaño <castano@gmail.com>

#include "Repo/src/nvcore/Debug.h"

using namespace nv;

int nvAbort(const char * exp, const char * file, int line, const char * func/*=NULL*/, const char * msg/*= NULL*/, ...)
{
  return 0;
}

void debug::terminate(int code)
{
}

void NV_CDECL nvDebugPrint(const char *msg, ...)
{
}

void debug::dumpInfo()
{
}

void debug::dumpCallstack(MessageHandler *messageHandler, int callstackLevelsToSkip /*= 0*/)
{
}

void debug::setMessageHandler(MessageHandler * message_handler)
{
}

void debug::resetMessageHandler()
{
}

void debug::setAssertHandler(AssertHandler * assert_handler)
{
}

void debug::resetAssertHandler()
{
}

void debug::enableSigHandler(bool interactive)
{
}

void debug::disableSigHandler()
{
}

bool debug::isDebuggerPresent()
{
    return false;
}

bool debug::attachToDebugger()
{
    return false;
}
