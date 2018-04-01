
#ifndef __LIST_NODE__
#define __LIST_NODE__

#include "misc.h"
#include "NodeType.h"

class AListNode;
typedef int (*COMPARE_FUNC)(const AListNode *pNode1, const AListNode *pNode2, void *pContext);

#define DELETE_LIST(list)						\
    if (list) {									\
	    delete list->DeleteAll();				\
        list = NULL;							\
    }

#define LIST_ATTACH_EX(type,parent)										\
	using parent::InsertBefore;											\
	virtual type *InsertBefore(type *pNode) {return (type *)parent::InsertBefore(pNode);} \
	using parent::InsertAfter;											\
	virtual type *InsertAfter(type *pNode)  {return (type *)parent::InsertAfter(pNode);} \
	using parent::Attach;												\
	virtual type *Attach(type *pNode) {return (type *)parent::Attach(pNode);} \
	virtual type *Attach(type *pNode, COMPARE_FUNC fn, void *pContext = NULL) {return (type *)parent::Attach(pNode, fn, pContext);}	\
	using parent::Prepend;												\
	virtual type *Prepend(type *pNode) {return (type *)parent::Prepend(pNode);} \
	using parent::Append;												\
	virtual type *Append(type *pNode)  {return (type *)parent::Append(pNode);}
#define LIST_DETACH_EX(type,parent) virtual type *Detach() {return (type *)parent::Detach();}
#define LIST_NEXT_EX(type,parent)   virtual type *Next(int n = 1) const {return (type *)parent::Next(n);}
#define LIST_PREV_EX(type,parent)   virtual type *Prev(int n = 1) const {return (type *)parent::Prev(n);}
#define LIST_FIRST_EX(type,parent)  virtual type *First() const {return (type *)parent::First();}
#define LIST_LAST_EX(type,parent)   virtual type *Last() const {return (type *)parent::Last();}
#define LIST_MEMBER_EX(type,parent) static type *Member(type *pFirst, int n) {return (type *)parent::Member(pFirst, n);}
#define LIST_SORT_EX(type,parent)										\
	virtual type *Sort() {return (type *)parent::Sort();}				\
    virtual type *Sort(COMPARE_FUNC fn, void *pContext = NULL) {return (type *)parent::Sort(fn, pContext);}
#define LIST_MERGE_EX(type,parent)										\
	virtual type *MergeList(AListNode *pNode) {return (type *)parent::MergeList(pNode);} \
	virtual type *MergeList(AListNode *pNode, COMPARE_FUNC fn, void *pContext = NULL) {return (type *)parent::MergeList(pNode, fn, pContext);}

#define LIST_FUNCTIONS_EX(type,parent)			\
LIST_ATTACH_EX(type,parent);					\
LIST_DETACH_EX(type,parent);					\
LIST_FIRST_EX(type,parent);						\
LIST_LAST_EX(type,parent);						\
LIST_NEXT_EX(type,parent);						\
LIST_PREV_EX(type,parent);						\
LIST_MEMBER_EX(type,parent);					\
LIST_SORT_EX(type,parent);						\
LIST_MERGE_EX(type,parent);						\
NODETYPE(type,parent)

#define LIST_FUNCTIONS(type) LIST_FUNCTIONS_EX(type, AListNode)

#define LISTNODE_DUPLICATE(type) virtual AListNode *Duplicate() const {return new type(*this);}

class AListNode : public ANodeType {
public:
	AListNode();
	virtual ~AListNode();

	void Insert(AListNode *pNode1, AListNode *pNode2);
	
	virtual AListNode *InsertBefore(AListNode *pNode);
	virtual AListNode *InsertAfter(AListNode *pNode);
	virtual AListNode *Attach(AListNode *pNode);
	virtual AListNode *Attach(AListNode *pNode, COMPARE_FUNC fn, void *pContext = NULL);
	virtual AListNode *Prepend(AListNode *pNode);
	virtual AListNode *Append(AListNode *pNode);
	virtual AListNode *Detach();
	
	virtual AListNode *First() const;
	virtual AListNode *Last() const;

	bool IsAttached() const {return (pPrev || pNext);}
	bool IsFirst()    const {return (pPrev == NULL);}
	bool IsLast()     const {return (pNext == NULL);}

	virtual AListNode *Next(int n = 1) const;
	virtual AListNode *Prev(int n = 1) const;

	virtual AListNode *Sort();
	virtual AListNode *Sort(COMPARE_FUNC fn, void *pContext = NULL);

	virtual AListNode *MergeList(AListNode *pNode);
	virtual AListNode *MergeList(AListNode *pNode, COMPARE_FUNC fn, void *pContext = NULL);

	virtual AListNode *Traverse(bool (*fn)(AListNode *pNode, void *pContext), void *pContext = NULL);

	int Count(bool bTotal = true);

	AListNode *DeleteAll();

	virtual AListNode *Duplicate() const {return NULL;}

	static AListNode *Member(AListNode *pFirst, int n) {return pFirst ? pFirst->Next(n) : NULL;}
	static int Count(AListNode *pFirst) {return pFirst ? pFirst->Count(false) : 0;}

	NODETYPE(AListNode, ANodeType);

protected:
	friend class AList;

	virtual AListNode *AttachEx(AListNode *pFirst, AListNode *pNode);
	virtual AListNode *AttachEx(AListNode *pFirst, AListNode *pNode, COMPARE_FUNC fn, void *pContext = NULL);
	virtual void DetachEx();

	virtual int Compare(const AListNode *pNode1, const AListNode *pNode2) {(void)pNode1; (void)pNode2; return 0;}
	
protected:
	AListNode *pPrev, *pNext;

	NODETYPE_DEFINE(AListNode);
};

#define LIST_OBJECT(type)																					\
virtual void Push(type *pNode) {AList::Push(pNode);}														\
virtual type *Pop()    {return (type *)AList::Pop();}														\
virtual type *EndPop() {return (type *)AList::EndPop();}													\
virtual type *Member(int n) const {return (type *)AList::Member(n);}										\
virtual type *First() const {return (type *)AList::First();}												\
virtual type *Last()  const {return (type *)AList::Last();}													\
virtual type *Find(const type *pCmpNode, bool bFromStart, COMPARE_FUNC fn, void *pContext = NULL) const {	\
	return (type *)AList::Find(pCmpNode, bFromStart, fn, pContext);											\
}

class AList {
public:
	AList();
	AList(const AList& object);
	virtual ~AList();

	void Copy(const AList& object);

	void EnableAutoDelete(bool enable = true) {bAutoDelete = enable;}
	bool AutoDeleteEnabled() const {return bAutoDelete;}

	virtual void Add(AListNode *pNode);
	virtual void Add(AListNode *pNode, COMPARE_FUNC fn, void *pContext = NULL);
	virtual bool InsertBefore(AListNode *pNode, AListNode *pMemberNode);
	virtual bool InsertAfter(AListNode *pNode, AListNode *pMemberNode);
	virtual bool Replace(AListNode *pNode, AListNode *pMemberNode);
	virtual bool Remove(AListNode *pNode);
	virtual bool Delete(AListNode *pNode);
	virtual void RemoveAll();
	virtual void DeleteAll();

	virtual void Push(AListNode *pNode);
	virtual void StartPush(AListNode *pNode);
	virtual AListNode *Pop();
	virtual AListNode *EndPop();

	virtual AListNode *Member(int n) const {return AListNode::Member(pFirst, n);}
	virtual int        Member(const AListNode *pNode) const;
	virtual bool	   IsMember(const AListNode *pNode) const {return (pNode->First() == pFirst);}

	virtual AListNode *First() const {return pFirst;}
	virtual AListNode *Last()  const {return pLast;}

	virtual bool Find(const AListNode *pNode) const;
	virtual AListNode *Find(const AListNode *pCmpNode, bool bFromStart, COMPARE_FUNC fn, void *pContext = NULL) const;

	virtual void Sort();
	virtual void Sort(COMPARE_FUNC fn, void *pContext = NULL);

	virtual AList *Duplicate(AList *pList = NULL) const;

	int Count() const {return nNodes;}

	void Delete();

	virtual void MarkAsChanged() {}

	uint_t Limit(uint_t count, bool fromEnd = true);

protected:
	AListNode *pFirst, *pLast;
	uint_t    nNodes;
	bool	  bAutoDelete;
};

class AListListNode : public AListNode {
public:
	AListListNode();
	AListListNode(const AListListNode& object);
	virtual ~AListListNode();

	LIST_FUNCTIONS(AListListNode);

	LISTNODE_DUPLICATE(AListListNode);

	AList List;

protected:
	NODETYPE_DEFINE(AListListNode);
};

#endif
