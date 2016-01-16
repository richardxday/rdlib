#ifndef __STRUCTURED_NODE__
#define __STRUCTURED_NODE__

#include "strsup.h"

class AStructuredNode : public AKeyValuePair {
public:
	AStructuredNode();
	virtual ~AStructuredNode();
	
	LIST_FUNCTIONS_EX(AStructuredNode, AKeyValuePair);

	void    SetType(uint8_t type) {Type = type;}
	uint8_t GetType()       const {return Type;}
	
	const AKeyValuePair *GetAttributes()		     const {return pAttributes;}
	void				AddAttribute(AKeyValuePair *pNode) {pAttributes = pNode->Append(pAttributes);}
	const AKeyValuePair *FindAttribute(const AString& Name, const AKeyValuePair *pAttribute = NULL) const;
	AString GetAttribute(const AString& Name, const AKeyValuePair *pAttribute = NULL) const;
	
	const AStructuredNode *GetChildren()		     const {return pChildren;}
	void				  AddChild(AStructuredNode *pNode) {pChildren = pNode->Append(pChildren);}
	const AStructuredNode *FindChild(const AString& name, const AStructuredNode *pChild = NULL) const;
	AString GetChildValue(const AString& Name, const AStructuredNode *pChild = NULL) const;

	void ToString(AString& str, uint_t level = 0) const;
	AString ToString() const {AString str; ToString(str); return str;}
	
protected:
	AKeyValuePair   *pAttributes;
	AStructuredNode *pChildren;
	uint8_t			Type;
	
protected:
	NODETYPE_DEFINE(AStructuredNode);
};

#endif
