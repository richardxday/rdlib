
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "simpleeval.h"

#define IsWhiteSpaceEx(c) (IsWhiteSpace(c) || (c == '\n') || (c == '\r'))

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

uint_t evaluate(const AHash& vars, const AString& str, uint_t pos, sint32_t& val)
{
	enum {
		State_First_PreModifier = 0,
		State_First_Operand,
		State_Operator,
		State_Second_PreModifier,
		State_Second_Operand,
	};
	AString premod, oper, arg;
	sint32_t 	val1  = 0;
	uint_t    state = State_First_PreModifier;

	while (str[pos]) {
		pos = skipwhitespace(str, pos);

		if (str[pos] == ')') break;

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
					evaluate(vars, arg.Mid(1, arg.len() - 2), 0, val1);
				}
				else if (IsSymbolStart(arg[0])) {
					val1 = vars.Read(arg);
				}
				else {
					val1 = (sint32_t)arg;
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
						if (val1 == 0) fprintf(stderr, "Warning: ignoring modulo by zero in '%s'\n", str.str());
						else		   val = val % val1;
					}
					else if (oper  == "/") {
						if (val1 == 0) fprintf(stderr, "Warning: ignoring divide by zero in '%s'\n", str.str());
						else		   val = val / val1;
					}
					state = State_Operator;
				}
				break;

			case State_Operator:
				pos   = getoper(str, pos, oper);
				state = State_Second_PreModifier;
				break;

			default:
				state = State_Operator;
				break;
		}

		pos = skipwhitespace(str, pos);
	}

	return pos;
}

bool evaluate(const AHash& vars, const AString& expr, sint32_t& val)
{
	bool success = true;
	uint_t pos = 0;

	pos = evaluate(vars, expr, pos, val);
	success = (expr[pos] == 0);
	
	return success;
}

