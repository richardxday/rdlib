#ifndef __XML_DECODE__
#define __XML_DECODE__

#include "StructuredNode.h"

extern bool DecodeXML(AStructuredNode& root, const AString& str);
extern bool DecodeXMLFromFile(AStructuredNode& root, const AString& filename);

#endif
