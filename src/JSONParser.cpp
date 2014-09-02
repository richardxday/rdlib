
#include <stdio.h>

#include "JSONParser.h"

AJSONValue *AJSONType::Find(const AString& name)
{
	return Find(name, NULL);
}

AJSONValue *AJSONType::FindNext(const AString& name)
{
	AJSONValue *res = NULL;

	if (GetAsValue()) {
        AJSONType *node = GetParent();

        if (node->GetAsObject()) node = node->GetParent();
        if (node) res = node->Find(name, GetAsValue());
	}

	return res;
}

AJSONValue *AJSONType::Find(const AString& name, AJSONValue *lastfound)
{
	bool test = (lastfound == NULL);
	return Find(name, lastfound, test);
}

AJSONValue *AJSONType::Find(const AString& name, AJSONValue *lastfound, bool& test)
{
	AJSONValue *res = NULL;

	if (GetAsValue()) {
        AJSONValue *value = GetAsValue();

        if      (value->GetName() == name) res = value;
        else if (value->GetValue())        res = value->GetValue()->Find(name, lastfound, test);

        if (res == lastfound) {
			res = NULL;
			test = true;
        }
	}
	else if (GetAsObject()) {
        AJSONObject      *obj = GetAsObject();
		const ADataList& list = obj->List();
		uint_t i;

		for (i = 0; i < list.Count(); i++) {
			AJSONType  *value  = (AJSONType *)list[i];
			AJSONValue *fvalue = value->Find(name, lastfound, test);

			if (test && (fvalue != NULL)) {
				res = fvalue;
				break;
			}
			if (!test && (fvalue == lastfound)) test = true;
        }
	}
	else if (GetAsArray()) {
        AJSONArray       *obj = GetAsArray();
		const ADataList& list = obj->List();
		uint_t i;

		for (i = 0; i < list.Count(); i++) {
			AJSONType  *value  = (AJSONType *)list[i];
			AJSONValue *fvalue = value->Find(name, lastfound, test);

			if (test && (fvalue != NULL)) {
				res = fvalue;
				break;
			}
			if (!test && (fvalue == lastfound)) test = true;
        }
	}

	return res;
}

void AJSONArray::AddValue(const AString& value, bool hasquotes)
{
	Add(new AJSONString(value, hasquotes));
}

AJSONArray *AJSONArray::AddArray()
{
	AJSONArray *array = new AJSONArray();
	Add(array);
	return array;
}
	
AJSONObject *AJSONArray::AddObject()
{
	AJSONObject *obj = new AJSONObject();
	Add(obj);
	return obj;
}

AString AJSONArray::ToString() const
{
	AString res = "[";
	uint_t i = 0;
	
	for (i = 0; i < items.Count(); i++) {
		AJSONType *obj = (AJSONType *)items[i];

		if (i) res += ",";
		res += obj->ToString();
	}
	
	res += "]";

	return res;
}

AString AJSONObject::ToString() const
{
	AString res = "{";
	uint_t i = 0;
	
	for (i = 0; i < items.Count(); i++) {
		AJSONType *obj = (AJSONType *)items[i];

		if (i) res += ",";
		res += obj->ToString();
	}
	
	res += "}";

	return res;
}

/*----------------------------------------------------------------------------------------------------*/

static const char *Whitespace = " \t\r\n";
static const char *Terminators = ",]}";

static int find_str_end(char *block, const char *terminators)
{
	int i, l = 0;
	bool quote = false;

	// run through text until either 0-byte is found or a terminator is
	// found OUTSIDE quotes
	for (i = 0; (block[i] != 0) && (quote || !strchr(terminators, block[i])); i++) {
        char c = block[i];

        // if a quote is found, prevent terminator check until closing
        // quote is found
        if (c == '\\') i++;
        else if ((i == 0) && (c == '\"')) quote = true;
        else if (quote && (c == '\"')) quote = false;
        else if (!quote &&
                 !RANGE(c, '0', '9') &&
                 !RANGE(c, 'a', 'z') &&
                 !RANGE(c, 'A', 'Z') &&
                 (c != '-') &&
                 (c != '.') &&
                 (c != '+')) break;
	}

	// if valid terminator found, return length
	if (block[i] != 0) l = i;

	//printf("find_str_end on '%s' with '%s' = %u, i = %u\n", block, terminators, l, i);

	return l;
}

static int strip_spaces(char *block, int l, bool end)
{
	int i;

	if (l == 0) return l;

	for (i = 0; (block[i] != 0) && (i < l) && strchr(Whitespace, block[i]); i++) ;

	if (i > 0) {
        // remove whitespace from start
        l -= i;
        memcpy(block + 0, block + i, l);
	}

	if (end) {
        // remove any whitespace from end
        while ((l > 0) && strchr(Whitespace, block[l - 1])) l--;
	}

	return l;
}

static int strip_string(char *block, int l, bool& hasquotes)
{
	l = strip_spaces(block, l, true);

	hasquotes = false;

	if (l == 0) return l;

	// strip quotes
	if ((l > 0) && (block[0] == '\"') && (block[l - 1] == '\"')) {
        l -= 2;

        // move text back over opening quote and ignore closing quote
        memcpy(block + 0, block + 1, l);

        hasquotes = true;
	}
	else hasquotes = false;

	// terminate string
	block[l] = 0;

	return l;
}

static void de_escape(char *block)
{
	int i, j;

	for (i = j = 0; (block[i] != 0); i++) {
        char c = block[i];

        if (c == '\\') {
			c = block[++i];

			switch (c) {
				case 'n': c = '\n'; break;
				case 'r': c = '\r'; break;
				case 't': c = '\t'; break;
				case '\\': c = '\\'; break;
				case 0: i--; break;
				default: break;
			}
        }

        block[j++] = c;
	}

	block[j] = 0;
}

AJSONParser::AJSONParser()
{
	tree   = NULL;
	blocklen = 256;
	block  = new char[blocklen];
	pos    = parsepos = 0;
	status = Parsing;
	state  = LookingForStart;
}

AJSONParser::~AJSONParser()
{
	if (block) delete[] block;
	if (tree)  delete tree;
}

AJSONParser::JSONStatus AJSONParser::parse(const char *buffer, uint_t count)
{
	uint_t start = 0;

	while ((status == Parsing) && (count > 0)) {
        uint_t n1 = MIN(blocklen - 1 - pos, count);
        bool needmoredata = false;

        // copy as many chars from input array to working storage as possible
        memcpy(block + pos, buffer + start, n1);

        // increment/decrement counters
        pos   += n1;
        start += n1;
        count -= n1;

        while ((pos > 0) && (status == Parsing) && !needmoredata) {
			uint_t l = 0, len;
			char c;

			// strip block of spaces from start ONLY
			len = pos;
			pos = strip_spaces(block, pos, false);
			block[pos] = 0;
			len -= pos;

			parsepos += len;

			if (pos == 0) break;

			c = block[0];
			
			//debug("status %u state %u count %u pos %u parsepos %u stack %u c '%c' block '%s'\n", status, state, count, pos, parsepos, stack.Count(), c, block);

			switch (state) {
				case LookingForStart:     // at startup, just look for opening {
					if (c == '{') {
						// top of tree
						tree = new AJSONObject();
						stack.Push(tree);

						// erase character
						l = 1;

						// move to looking for name
						state = LookingForName;
					}
					else {
						// first character is NOT at '{' throw an error
						status = InvalidStartCharacter;
					}
					break;

				case LookingForName:
					if (c == '}') {
						// empty object

						if (StackTop()->GetAsObject()) {
							AJSONObject *obj = StackTop()->GetAsObject();

							if (obj->Count() == 0) {
								if (StackPop()) {
									status = StackUnderflow;
								}
								else if (StackTop()->GetAsValue()) {
									if (StackPop()) {
										status = Complete;
									}
								}
								else status = StackUnderflow;
							}
							else status = IllegalObjectClose;
						}
						else status = IllegalObjectClose;

						state = LookingForSeparator;

						// erase character
						l = 1;
					}
					else if (c != '\"') {
						//printf("Error: illegal character found whilst looking for name start: '%c'\n", c);

						status = MissingQuotesAroundName;
						break;
					}
					else if ((l = find_str_end(block, ":")) > 0) {
						// name string
						// remove quotes and terminate
						bool hasquotes;

						strip_string(block, l, hasquotes);

						// erase up to and including ':' character
						l++;

						AString    name(block, strlen(block));

						//debug("New item name: '%s'\n", name.str());

						AJSONValue *val = new AJSONValue();
						val->SetName(name);
						if (StackTop() && (StackTop()->GetAsObject() || StackTop()->GetAsArray())) {
							StackTop()->Add(val);
							stack.Push(val);
						}
						else status = IllegalNameLocation;

						// look for value
						state = LookingForValue;
					}
					else {
						// need more data
						needmoredata = true;
					}
					break;

				case LookingForValue:
					if (c == ']') {
						if (StackTop()->GetAsArray()) {
							if (StackPop()) {
								status = StackUnderflow;
							}
							else if (StackTop()->GetAsValue()) {
								if (StackPop()) {
									status = StackUnderflow;
								}
							}
						}

						state = LookingForSeparator;

						// erase character
						l = 1;
					}
					else if (c == '{') {
						if ((StackTop()->GetAsArray()) || (StackTop()->GetAsValue())) {
							AJSONObject *obj = new AJSONObject();
							StackTop()->Add(obj);
							stack.Push(obj);
						}
						else status = IllegalObjectLocation;

						// erase character
						l = 1;

						// look for name
						state = LookingForName;
					}
					else if (c == '[') {
						// new array
						if (StackTop()->GetAsValue()) {
							AJSONValue *obj  = StackTop()->GetAsValue();
							AJSONArray *list = new AJSONArray();
							obj->SetValue(list);
							stack.Push(list);
						}
						else status = IllegalArrayLocation;

						// erase character
						l = 1;

						// look for name
						state = LookingForValue;
					}
					else if ((l = find_str_end(block, Terminators)) > 0) {
						// value string
						// save terminating character
						char terminator = block[l];
						bool hasquotes;

						// remove quotes either side
						strip_string(block, l, hasquotes);

						// terminate string
						block[l] = 0;

						// de-escape characters
						de_escape(block);

						//debug("block '%s' l %u\n", block, l);

						AJSONString *val = new AJSONString();
						val->Value     = AString(block, strlen(block));
						val->HasQuotes = hasquotes;

						if (StackTop()->GetAsValue()) {
							AJSONValue *obj = StackTop()->GetAsValue();
							obj->SetValue(val);

							if (StackPop()) {
								status = StackUnderflow;
							}
						}
						else if (StackTop()->GetAsArray()) {
							AJSONArray *list = StackTop()->GetAsArray();
							list->Add(val);
						}
						else status = IllegalValueLocation;

						// restore terminating character
						block[l] = terminator;

						// look for name
						state = LookingForSeparator;
					}
					else {
						// need more data
						needmoredata = true;
					}
					break;

				case LookingForSeparator:
					if (c == ',') {
						// erase character
						l = 1;

						if (StackTop()->GetAsArray()) {
							state = LookingForValue;
						}
						else {
							state = LookingForName;
						}
					}
					else if (c == '}') {
						// call [empty] handler

						if (StackTop()->GetAsObject()) {
							if (StackPop()) {
								status = Complete;
							}
							else if (StackTop()->GetAsValue()) {
								if (StackPop()) {
									status = Complete;
								}
							}
						}
						else status = IllegalObjectClose;

						// erase character
						l = 1;
					}
					else if (c == ']') {
						// call [empty] handler
						if (StackTop()->GetAsArray()) {
							if (StackPop()) {
								status = StackUnderflow;
							}
							else if (StackTop()->GetAsValue()) {
								if (StackPop()) {
									status = StackUnderflow;
								}
							}
						}
						else status = IllegalArrayClose;

						// erase character
						l = 1;
					}
					else {
						status = UnrecognizedCharacter;
					}
					break;

				default:
					status = IllegalState;
					break;
			}

			// erase specified number of characters and move
			// remainder of buffer to front of buffer
			if (l > 0) {
				pos -= l;
				parsepos += l;
				memmove(block, block + l, pos + 1);
			}
        }

        pos = strip_spaces(block, pos, false);

        if (n1 == 0) break;
	}

	if (status == Complete) {
        if      (!StackEmpty())            status = ParsingIncomplete;
        else if ((count > 0) || (pos > 0)) status = CharactersInBuffer;
	}

	return status;
}

AString AJSONParser::ToString() const
{
	return tree ? tree->ToString() : "";
}
