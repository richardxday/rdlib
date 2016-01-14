#ifndef __STRUCTURED_NODE__
#define __STRUCTURED_NODE__

#include "strsup.h"

class AStructuredNode : public AKeyValuePair {
public:
	AStructuredNode() : AKeyValuePair(),
						pAttributes(NULL),
						pChildren(NULL),
						Type(0) {}
	virtual ~AStructuredNode() {DELETE_LIST(pAttributes); DELETE_LIST(pChildren);}
	
	LIST_FUNCTIONS_EX(AStructuredNode, AKeyValuePair);

	void    SetType(uint8_t type) {Type = type;}
	uint8_t GetType()       const {return Type;}
	
	const AListNode *GetAttributes()		 const {return pAttributes;}
	void			AddAttribute(AListNode *pNode) {pAttributes = pNode->Append(pAttributes);}
		
	const AListNode *GetChildren()			 const {return pChildren;}
	void			AddChild(AListNode *pNode)     {pChildren = pNode->Append(pChildren);}

protected:
	AListNode *pAttributes;
	AListNode *pChildren;
	uint8_t	  Type;
	
protected:
	NODETYPE_DEFINE(AStructuredNode);
};

#endif
