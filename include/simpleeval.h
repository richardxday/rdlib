#ifndef __SIMPLE_EVAL__
#define __SIMPLE_EVAL__

#include <vector>
#include <map>

#include "strsup.h"
#include "Hash.h"
#include "EvalNumber.h"

typedef std::map<AString, AValue> simplevars_t;
typedef std::vector<AValue>       simpleargs_t;

extern void AddSimpleFunction(const AString& name, AValue (*fn)(const AString& funcname, const simplevars_t& vars, const simpleargs_t& values, AString& errors, void *context), void *context = NULL);
extern void RemoveSimpleFunction(const AString& name);

extern uint_t Evaluate(simplevars_t& vars, const AString& str, uint_t pos, AValue& val, AString& errors);
extern bool Evaluate(simplevars_t& vars, const AString& expr, AValue& val, AString& errors);

#endif
