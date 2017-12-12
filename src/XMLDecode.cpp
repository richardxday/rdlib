
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "QuitHandler.h"
#include "XMLDecode.h"
#include "DataList.h"

#define IsXMLStartChar(x) (IsSymbolStart(x) || ((x) == '?') || ((x) == '!'))
#define IsXMLNameChar(x) (IsSymbolChar(x) || ((x) == ':') || ((x) == '-'))
#define IsXMLWhiteSpace(x) (IsWhiteSpace(x) || ((x) == '\n') || ((x) == '\r'))

static bool debug_decode = false;

static uint_t FindXMLMarker(const AString& str, uint_t p1)
{
	uint_t i, p2 = 0;
	int quote = 0;

	for (i = p1; str[i]; i++) {
		if		(quote && (str[i] == quote)) quote = 0;
		else if (!quote && IsQuoteChar(str[i])) quote = str[i];
		else if (!quote && (str[i] == '>')) {
			p2 = i;
			break;
		}
	}

	return p2;
}

bool DecodeXML(AStructuredNode& root, const AString& str)
{
	AStructuredNode *pNode = &root;
	AKeyValuePair   *pAttr;
	ADataList       stack;
	uint_t p1 = 0, p2;

	while (IsXMLWhiteSpace(str[p1])) p1++;
	while (!HasQuit() && pNode && (str[p1] == '<')) {
		bool popnode = false;

		p1++;

		if (debug_decode) debug("p1: %u, str: '%s'\n", p1, str.Mid(p1, 40).str());

		if (str[p1] == '/') {
			p1++;
			if ((pNode != &root) && ((p2 = FindXMLMarker(str, p1)) > p1)) {
				uint_t p3 = p1;

				if (IsSymbolStart(str[p1])) p1++;
				while (IsXMLNameChar(str[p1])) p1++;

				AString name = str.Mid(p3, p1 - p3);
				if (name == pNode->Key) {
					if (debug_decode) debug("Closing node '%s'\n", pNode->Key.str());
					popnode = true;
					p1 = p2 + 1;
				}
				else {
					debug("Unknown close object '%s' at %u\n", name.str(), p1);
					break;
				}
			}
			else {
				debug("Unknown close marker at %u\n", p1);
				break;
			}
		}
		else if (str.Mid(p1, 3) == "!--") {
			if ((p2 = str.Pos("-->", p1 + 3)) > p1) p1 = p2 + 3;
			else {
				debug("Unterminated comment marker at %u\n", p1);
				break;
			}
		}
		else if ((p2 = FindXMLMarker(str, p1)) > p1) {
			if (pNode) stack.Push((void *)pNode);

			if ((pNode = new AStructuredNode) != NULL) {
				uint_t p3 = p2;
				bool   complete = ((str[p3 - 1] == '/') || ((str[p1] == '?') && (str[p3 - 1] == '?')));

				if (complete) p3--;

				complete |= (str[p1] == '!');

				if ((str[p1] == '?') || (str[p1] == '!')) pNode->SetType(str[p1++]);

				while (IsXMLWhiteSpace(str[p1])) p1++;

				uint_t p4 = p1;
				if (IsSymbolStart(str[p1])) p1++;
				while (IsXMLNameChar(str[p1])) p1++;

				pNode->Key = str.Mid(p4, p1 - p4).DeHTMLify();
				if (debug_decode) debug("Created new node '%s' (%s)\n", pNode->Key.str(), complete ? "complete" : "open");

				while (IsXMLWhiteSpace(str[p1])) p1++;

				while (p1 < p3) {
					p4 = p1;
					if (IsSymbolStart(str[p1])) p1++;
					while (IsXMLNameChar(str[p1])) p1++;
					uint_t p5 = p1;

					while (IsXMLWhiteSpace(str[p1])) p1++;
					if (str[p1] == '=') p1++;
					while (IsXMLWhiteSpace(str[p1])) p1++;

					int quote = 0;
					if (IsQuoteChar(str[p1])) quote = str[p1++];

					uint_t p6 = p1;
					while (str[p1] && ((quote && (str[p1] != quote)) ||
									   (!quote && !IsXMLWhiteSpace(str[p1])))) p1++;
					uint_t p7 = p1;

					if (quote && (str[p1] == quote)) p1++;
					while (IsXMLWhiteSpace(str[p1])) p1++;

					if ((p5 > p4) && ((pAttr = new AKeyValuePair) != NULL)) {
						pAttr->Key   = str.Mid(p4, p5 - p4).DeHTMLify();
						pAttr->Value = str.Mid(p6, p7 - p6).DeHTMLify();
						pNode->AddAttribute(pAttr);
						if (debug_decode) debug("Added attribute '%s'='%s' to '%s'\n", pAttr->Key.str(), pAttr->Value.str(), pNode->Key.str());
					}
					else break;
				}

				AStructuredNode *pParent = (AStructuredNode *)stack.Last();
				if (pParent) pParent->AddChild(pNode);

				p2++;
				while (IsXMLWhiteSpace(str[p2])) p2++;

				p3 = p1 = p2;
				while (str[p2] && (str[p2] != '<')) {
					p2++;
					if (!IsXMLWhiteSpace(str[p2 - 1])) p3 = p2;
				}
				if (p3 > p1) {
					pNode->Value = str.Mid(p1, p3 - p1).DeHTMLify();
					if (debug_decode) debug("Set value of '%s' to '%s'\n", pNode->Key.str(), pNode->Value.str());
				}
				p1 = p2;

				popnode = complete;
			}
		}

		if (popnode) {
			if (stack.Count()) {
				pNode = (AStructuredNode *)stack.EndPop();
				if (debug_decode) debug("Back to node '%s'\n", pNode->Key.str());
			}
			else {
				debug("Stack empty at %u\n", p1);
				break;
			}
		}

		while (IsXMLWhiteSpace(str[p1])) p1++;
	}

	if (stack.Count()) debug("Unterminated XML entries at %u\n", p1);
	if (!pNode)		   debug("Extra XML termination at %u\n", p1);

	return (!str[p1] && (pNode == &root));
}

bool DecodeXMLFromFile(AStructuredNode& root, const AString& filename)
{
	AString str;
	bool    success = false;

	if (str.ReadFromFile(filename)) {
		success = DecodeXML(root, str);
	}

	return success;
}
