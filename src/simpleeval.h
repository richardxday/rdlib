#ifndef __SIMPLE_EVAL__
#define __SIMPLE_EVAL__

#include "strsup.h"
#include "Hash.h"

extern uint_t evaluate(const AHash& vars, const AString& str, uint_t pos, sint32_t& val);
extern bool evaluate(const AHash& vars, const AString& expr, sint32_t& val);

#endif
