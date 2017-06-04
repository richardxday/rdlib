
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "simpleeval.h"

#define DEBUGEVAL 0

#define IsWhiteSpaceEx(c) (IsWhiteSpace(c) || (c == '\n') || (c == '\r'))

typedef struct {
	AValue (*fn)(const AString& funcname, const simplevars_t& vars, const simpleargs_t& values, AString& errors, void *context);
	void *context;
} simplefunc_t;

static std::map<AString, simplefunc_t> funcs;

void AddSimpleFunction(const AString& name, AValue (*fn)(const AString& funcname, const simplevars_t& vars, const simpleargs_t& values, AString& errors, void *context), void *context)
{
	funcs[name].fn = fn;
	funcs[name].context = context;
}

void RemoveSimpleFunction(const AString& name)
{
	std::map<AString, simplefunc_t>::iterator it;

	if ((it = funcs.find(name)) != funcs.end()) {
		funcs.erase(it);
	}
}

static const struct {
	const char *funcname;
	size_t nparameters;
} __math_parameters[] = {
	{"sin", 1},
	{"cos", 1},
	{"tan", 1},
	{"asin", 1},
	{"acos", 1},
	{"atan", 1},
	{"atan2", 2},
	{"exp", 1},
	{"log", 1},
	{"ln", 1},
	{"sqrt", 1},
	{"pow", 2},
	{"mod", 2},
	{"deg2rad", 1},
	{"rad2deg", 1},
};

static AValue __math(const AString& funcname, const simplevars_t& vars, const simpleargs_t& values, AString& errors, void *context)
{
	static std::map<AString,size_t> expectedparameters;
	std::map<AString,size_t>::const_iterator it;
	AValue res;

	UNUSED(vars);
	UNUSED(context);
	
	if (expectedparameters.end() == expectedparameters.begin()) {
		uint_t i;

		for (i = 0; i < NUMBEROF(__math_parameters); i++) {
			expectedparameters[__math_parameters[i].funcname] = __math_parameters[i].nparameters;
		}
	}

	if ((it = expectedparameters.find(funcname)) != expectedparameters.end()) {
		if (it->second == values.size()) {
			if		(funcname == "sin")   	res = sin((double)values[0]);
			else if (funcname == "cos")   	res = cos((double)values[0]);
			else if (funcname == "tan")   	res = tan((double)values[0]);
			else if (funcname == "asin")  	res = asin((double)values[0]);
			else if (funcname == "acos")  	res = acos((double)values[0]);
			else if (funcname == "atan")  	res = atan((double)values[0]);
			else if (funcname == "atan2") 	res = atan2((double)values[0], (double)values[1]);
			else if (funcname == "exp")   	res = exp((double)values[0]);
			else if (funcname == "log")   	res = log10((double)values[0]);
			else if (funcname == "ln")    	res = log((double)values[0]);
			else if (funcname == "sqrt")  	res = sqrt((double)values[0]);
			else if (funcname == "pow")   	res = pow((double)values[0], (double)values[1]);
			else if (funcname == "mod")   	res = fmod((double)values[0], (double)values[1]);
			else if (funcname == "deg2rad") res = (double)values[0] * M_PI / 180.0;
			else if (funcname == "rad2deg") res = (double)values[0] * 180.0 / M_PI;
			else {
				errors.printf("Unimplemented function '%s'\n", funcname.str());
			}
		}
		else {
			errors.printf("Incorrect number of parameters to '%s', expected %u, supplied %u\n", funcname.str(), (uint_t)it->second, (uint_t)values.size());
		}
	}
	else {
		errors.printf("Unknown function '%s'\n", funcname.str());
	}
	
	return res;
}

static uint_t findendquote(const AString& str, uint_t pos, char c)
{
	while (str[pos] && (str[pos] != c)) {
		if (str[pos] == '\\') {
			pos++;
			if (str[pos]) pos++;
		}
		else pos++;
	}

	return pos;
}

static uint_t findend(const AString& str, uint_t pos, char c)
{
	char c1;

	while (str[pos] && (str[pos] != c)) {
		switch (str[pos]) {
			case '(':
				pos = findend(str, pos + 1, ')');
				if (str[pos] == ')') pos++;
				else return pos;
				break;

			case '[':
				pos = findend(str, pos + 1, ']');
				if (str[pos] == ']') pos++;
				else return pos;
				break;

			case '{':
				pos = findend(str, pos + 1, '}');
				if (str[pos] == '}') pos++;
				else return pos;
				break;

			case ')':
			case ']':
			case '}':
				return pos;

			case '\'':
			case '\"':
				c1  = str[pos];
				pos = findendquote(str, pos + 1, c1);
				if (str[pos] == c1) pos++;
				else return pos;
				break;

			case ';':
				if (str[pos + 1] == ';') {
					pos += 2;

					while (str[pos] && (str[pos] != '\n')) pos++;

					if (str[pos] == '\n') pos++;
					else return pos;
				}
				else pos++;
				break;

			default:
				pos++;
				break;
		}
	}

	return pos;
}

static uint_t skipwhitespace(const AString& str, uint_t pos)
{
	while (IsWhiteSpaceEx(str[pos])) pos++;

	if ((str[pos] == ';') && (str[pos + 1] == ';')) {
		pos += 2;

		while (str[pos] && (str[pos] != '\n')) pos++;

		if (str[pos] == '\n') pos++;

		while (IsWhiteSpaceEx(str[pos])) pos++;
	}

	return pos;
}

static uint_t getarg(const AString& str, uint_t pos, AString& arg)
{
	AString left2;

	pos = skipwhitespace(str, pos);

	uint_t pos0 = pos;
	left2 = str.Mid(pos, 2);
	if (str[pos] == '(') {
		pos = findend(str, pos + 1, ')');
		if (str[pos] == ')') pos++;
	}
	else if (IsSymbolStart(str[pos])) {
		while (IsSymbolChar(str[pos])) pos++;
	}
	else if (IsHexStartChar(str[pos]) || IsHexString(left2) || IsDoubleStartChar(str[pos])) {
		if (IsHexString(left2)) pos += 2;
		else					pos++;

		while (IsHexChar(str[pos])) pos++;
	}
	else if (IsOctStartChar(str[pos]) || IsOctString(left2)) {
		if (IsOctString(left2)) pos += 2;
		else					pos++;

		while (IsOctChar(str[pos])) pos++;
	}
	else if (IsNumeralChar(str[pos]) || IsSignChar(str[pos]) || IsPointChar(str[pos])) {
		pos++;

		while (IsNumeralChar(str[pos]) || IsSignChar(str[pos]) || IsPointChar(str[pos])) pos++;
	}

	arg = str.Mid(pos0, pos - pos0);

	pos = skipwhitespace(str, pos);

	return pos;
}

static uint_t getoper(const AString& str, uint_t pos, AString& arg)
{
	static const char chars[] = "~+-*/%^&|=!<>";

	pos = skipwhitespace(str, pos);

	uint_t pos0 = pos;
	while (strchr(chars, str[pos])) pos++;

	arg = str.Mid(pos0, pos - pos0);

	pos = skipwhitespace(str, pos);

	return pos;
}

uint_t Evaluate(simplevars_t& vars, const AString& str, uint_t pos, AValue& val, AString& errors)
{
	enum {
		State_First_PreModifier = 0,
		State_First_Operand,
		State_Operator,
		State_Second_PreModifier,
		State_Second_Operand,
	};
	AString premod, oper, arg;
	AValue  val1  = 0;
	uint_t  state = State_First_PreModifier;
	uint_t  operpos = 0;

	if (funcs.find(__math_parameters[0].funcname) == funcs.end()) {
		uint_t i;
		
		for (i = 0; i < NUMBEROF(__math_parameters); i++) {
			AddSimpleFunction(__math_parameters[i].funcname, &__math);
		}
	}
	
	while (str[pos]) {
		pos = skipwhitespace(str, pos);

		if (str[pos] == ')') break;
		if (str[pos] == ',') {
			pos++;
			val1  = 0;
			state = State_First_PreModifier;
		}

		uint_t pos0 = pos;
		switch (state) {
			case State_First_PreModifier:
			case State_Second_PreModifier:
				pos = getoper(str, pos, premod);
				state++;
				break;

			case State_First_Operand:
			case State_Second_Operand:
				pos = getarg(str, pos, arg);

				if (arg[0] == '(') {
					AString subexpr = arg.Mid(1, arg.len() - 2);
					if (!Evaluate(vars, subexpr, 0, val1, errors)) {
						errors.printf("Failed to evaluate expression '%s' at '%s'\n", subexpr.str(), str.Mid(pos0).str());
					}
#if DEBUGEVAL
					else debug("Result of '%s' is %s\n", subexpr.str(), val1.ToString().str());
#endif
				}
				else if (IsSymbolStart(arg[0])) {
					simplevars_t::const_iterator it;
					AString args;

					pos = skipwhitespace(str, pos);

					if (str[pos] == '(') {
						pos = getarg(str, pos, args);
						pos = skipwhitespace(str, pos);
					}

					if (args.Valid()) {
						std::map<AString, simplefunc_t>::const_iterator it;

						if ((it = funcs.find(arg)) == funcs.end()) {
							errors.printf("Function '%s' not found at '%s'\n", arg.str(), str.Mid(pos0).str());
						}
						else if ((args.FirstChar() == '(') && (args.LastChar() == ')')) {
							args = args.Mid(1, args.len() - 2);

							simpleargs_t values;
							uint_t i, n = args.CountColumns();
							for (i = 0; i < n; i++) {
								AString subexpr = args.Column(i);
								AValue  val = 0;

								if (Evaluate(vars, subexpr, val, errors)) {
									values.push_back(val);
								}
								else {
									errors.printf("Failed to evaluate expression '%s' at '%s'\n", subexpr.str(), str.Mid(pos0).str());
									break;
								}
							}

							if (errors.Empty()) {
								val1 = (*it->second.fn)(arg, vars, values, errors, it->second.context);
							}
						}
						else errors.printf("Invalid args '%s' for function '%s' at '%s'\n", args.str(), arg.str(), str.Mid(pos0).str());
					}
					else if (str.Mid(pos, 2) == ":=") {
						uint_t pos0 = pos + 2;
						pos = Evaluate(vars, str, pos0, val1, errors);
						if (errors.Empty()) {
#if DEBUGEVAL
							debug("vars[%s] = %s (from '%s')\n", arg.str(), val1.ToString().str(), str.Mid(pos0, pos - pos0).str());
#endif
							vars[arg] = val1;
						}
					}
					else if ((it = vars.find(arg)) == vars.end()) {
						errors.printf("Variable '%s' not found at '%s'\n", arg.str(), str.Mid(pos0).str());
					}
					else {
						val1 = it->second;
#if DEBUGEVAL
						debug("Var '%s' is %s\n", arg.str(), val1.ToString().str());
#endif
					}
				}
				else {
					val1 = AValue::EvalNumberEx(arg.str(), true, NULL, &errors);
				}

				if		(premod == "-") val1 = -val1;
				else if (premod == "~") val1 = ~val1;
				else if (premod == "!") val1 = !val1;

				if (state == State_First_Operand) {
					val = val1;
					state++;
				}
				else {
#if DEBUGEVAL
					debug("val=%s, val1=%s, oper='%s'\n", val.ToString().str(), val1.ToString().str(), oper.str());
#endif
					
					if		(oper == "+")  val  += val1;
					else if (oper == "-")  val  -= val1;
					else if (oper == "*")  val  *= val1;
					else if (oper == "^")  val  ^= val1;
					else if (oper == "&")  val  &= val1;
					else if (oper == "|")  val  |= val1;
					else if (oper == "&&") val   = val && val1;
					else if (oper == "||") val   = val || val1;
					else if (oper == "==") val   = (val == val1);
					else if (oper == "!=") val   = (val != val1);
					else if (oper == "<=") val   = (val <= val1);
					else if (oper == ">=") val   = (val >= val1);
					else if (oper == "<>") val   = (val != val1);
					else if (oper == "=")  val   = (val == val1);
					else if (oper == "<")  val   = (val <  val1);
					else if (oper == ">")  val   = (val >  val1);
					else if (oper == "<<") val <<= val1;
					else if (oper == ">>") val >>= val1;
					else if (oper == "%") {
						if (val1.IsZero()) errors.printf("Modulo by zero at '%s'\n", str.Mid(operpos).str());
						else			   val = val % val1;
					}
					else if (oper  == "/") {
						if (val1.IsZero()) errors.printf("Divide by zero at '%s'\n", str.Mid(operpos).str());
						else			   val = val / val1;
					}
					state = State_Operator;
				}
				break;

			case State_Operator:
				operpos = pos;
				pos   	= getoper(str, pos, oper);
				if (oper.Empty()) {
					errors.printf("Unrecognized operator '%s' at '%s'\n", oper.str(), str.Mid(operpos).str());
				}
				state 	= State_Second_PreModifier;
				break;

			default:
				state = State_Operator;
				break;
		}

		if (errors.Valid()) break;

		pos = skipwhitespace(str, pos);
	}

	return pos;
}

bool Evaluate(simplevars_t& vars, const AString& expr, AValue& val, AString& errors)
{
	bool success = true;
	uint_t pos = 0;

	pos = Evaluate(vars, expr, pos, val, errors);
	success = (errors.Empty() && (expr[pos] == 0));

	return success;
}
