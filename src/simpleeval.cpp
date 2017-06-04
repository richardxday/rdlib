
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "simpleeval.h"

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

uint_t Evaluate(const simplevars_t& vars, const AString& str, uint_t pos, AValue& val, AString& errors)
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

	while (str[pos]) {
		pos = skipwhitespace(str, pos);

		if (str[pos] == ')') break;

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
					else if ((it = vars.find(arg)) == vars.end()) {
						errors.printf("Variable '%s' not found at '%s'\n", arg.str(), str.Mid(pos0).str());
					}
					else val1 = it->second;
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
					if		(oper  == "+")	val	 += val1;
					else if (oper  == "-")	val	 -= val1;
					else if (oper  == "*")	val	 *= val1;
					else if (oper  == "^")	val	 ^= val1;
					else if (oper  == "&")	val	 &= val1;
					else if (oper  == "|")	val	 |= val1;
					else if (oper  == "&&")	val	  = val && val1;
					else if (oper  == "||")	val	  = val || val1;
					else if (oper  == "==")	val	  = (val == val1);
					else if (oper  == "!=")	val	  = (val != val1);
					else if (oper  == "<=")	val	  = (val <= val1);
					else if (oper  == ">=")	val	  = (val >= val1);
					else if (oper  == "<>")	val	  = (val != val1);
					else if (oper  == "=")	val	  = (val == val1);
					else if (oper  == "<")	val	  = (val <  val1);
					else if (oper  == ">")	val	  = (val >  val1);
					else if (oper  == "<<")	val	<<= val1;
					else if (oper  == ">>")	val	>>= val1;
					else if (oper  == "%") {
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

bool Evaluate(const simplevars_t& vars, const AString& expr, AValue& val, AString& errors)
{
	bool success = true;
	uint_t pos = 0;

	pos = Evaluate(vars, expr, pos, val, errors);
	success = (expr[pos] == 0);

	return success;
}
