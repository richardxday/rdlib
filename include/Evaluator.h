#ifndef __EVALUATOR__
#define __EVALUATOR__

#include "strsup.h"
#include "Hash.h"

extern AString Evaluate(const AString& str, AHash *vars = NULL);

#endif
