
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Evaluator.h"

#define IsWhiteSpaceEx(c) (IsWhiteSpace(c) || (c == '\n') || (c == '\r'))

static uint_t FindEndQuote(const AString& str, uint_t pos, char c)
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

static uint_t FindEnd(const AString& str, uint_t pos, char c)
{
	char c1;

	while (str[pos] && (str[pos] != c)) {
		switch (str[pos]) {
			case '(':
				pos = FindEnd(str, pos + 1, ')');
				if (str[pos] == ')') pos++;
				else return pos;
				break;

			case '[':
				pos = FindEnd(str, pos + 1, ']');
				if (str[pos] == ']') pos++;
				else return pos;
				break;

			case '{':
				pos = FindEnd(str, pos + 1, '}');
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
				pos = FindEndQuote(str, pos + 1, c1);
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

static uint_t SkipWhiteSpace(const AString& str, uint_t pos)
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

static uint_t GetArg(const AString& str, uint_t pos, AString& arg, AHash *vars)
{
	static const char chars[] = "!$%^&*(),./<>?#~@";

	UNUSED(vars);

	pos = SkipWhiteSpace(str, pos);

	uint_t pos0 = pos;
	if (IsNumeralChar(str[pos]) || IsSignChar(str[pos]) || IsPointChar(str[pos])) {
		while (IsNumeralChar(str[pos]) || IsSignChar(str[pos]) || IsPointChar(str[pos])) pos++;
	}
	else if (IsSymbolStart(str[pos])) {
		while (IsSymbolChar(str[pos])) pos++;
	}
	else {
		while (strchr(chars, str[pos])) pos++;
	}

	arg = str.Mid(pos0, pos - pos0);

	pos = SkipWhiteSpace(str, pos);

	return pos;
}

static AString EvaluateEx(const AString& str, AHash *vars)
{
	static uint_t level = 0;
	AString res;
	uint_t pos = 0;

	pos = SkipWhiteSpace(str, pos);

	AString header = AString("--").Copies(level) + ">";
	debug("%sEvaluate('%s')\n", header.str(), str.str());
	header = AString("--").Copies(++level) + ">";

	if (str[pos] == '(') {
		while (str[pos] == '(') {
			uint_t endpos = FindEnd(str, pos + 1, ')');
			
			if (str[endpos] == ')') {
				AString str1 = str.Mid(pos + 1, endpos - pos - 1);
				res = EvaluateEx(str1, vars);
				pos = endpos + 1;
			}
			else {
				printf("Missing ')' at '%s'\n", str.Mid(endpos).str());
				break;
			}

			pos = SkipWhiteSpace(str, pos);
		}
	}
	else {
		AString arg, arg1;

		pos = GetArg(str, pos, arg, vars);

		if (arg.len() > 0) {
			if (IsSymbolStart(arg[0])) {
				//printf("%sword='%s', char = '%c', end=%u\n", header.str(), word.str(), str[pos], (pos == endpos));

				if (!str[pos]) {
					const AString *val = (const AString *)vars->Read(arg);

					if (val) {
						printf("%sFunction '%s' is '%s'\n", header.str(), arg.str(), val->str());
					
						res = EvaluateEx(*val, vars);
					}
					else printf("%sUnknown function '%s'\n", header.str(), arg.str());
				}
				else if ((str[pos] == ':') && (str[pos + 1] == '=')) {
					pos += 2;

					pos = SkipWhiteSpace(str, pos);

					AString *val = new AString(str.Mid(pos));

					printf("%sNew function '%s' is '%s'\n", header.str(), arg.str(), val->str());

					vars->Insert(arg, (uptr_t)val);
				}
				else if (str[pos] == '=') {
					pos++;

					pos = SkipWhiteSpace(str, pos);

					AString *val = new AString(EvaluateEx(str.Mid(pos), vars));

					printf("%sVariable '%s' is '%s'\n", header.str(), arg.str(), val->str());

					vars->Insert(arg, (uptr_t)val);
				}
				else if (arg == "result") {
					res = EvaluateEx(str.Mid(pos), vars);
				}
			}
			else if (IsNumeralChar(arg[0])) {
			}
		}
	}

	level--;

	return res;
}

static void __DeleteVar(uptr_t item, void *context)
{
	UNUSED(context);

	delete (AString *)item;
}

AString Evaluate(const AString& str, AHash *vars)
{
	AString res;
	uint_t endpos = FindEnd(str, 0, 0);

	if (str[endpos] == 0) {
		AHash _vars;

		if (!vars) {
			vars = &_vars;
			vars->Create(100, &__DeleteVar);
		}

		res = EvaluateEx(str, vars);
	}
	else res.Format("Error at pos %u: '%s'\n", endpos, str.Mid(endpos).str());

	return res;
}
