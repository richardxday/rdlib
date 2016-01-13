#ifndef __STRUCTURED_NODE__
#define __STRUCTURED_NODE__

#include "strsup.h"

class AStructuredNode : public AKeyValuePair {
public:
	AStructuredNode() : AKeyValuePair(),
						pFirstAttribute(NULL),
						pFirstChild(NULL) {}
	virtual ~AStructuredNode() {DELETE_LIST(pFirstAttribute); DELETE_LIST(pFirstChild);}
	
	LIST_FUNCTIONS(AStructuredNode);

	const AListNode *GetFirstAttribute() 	 const {return pFirstAttribute;}
	void			AddAttribute(AListNode *pNode) {pFirstAttribute = pNode->Append(pFirstAttribute);}
		
	const AListNode *GetFirstChild() 	   	 const {return pFirstChild;}
	void			AddChild(AListNode *pNode)     {pFirstChild = pNode->Append(pFirstChild);}

protected:
	AListNode *pFirstAttribute;
	AListNode *pFirstChild;

protected:
	NODETYPE_DEFINE(AStructuredNode);
};

#endif
