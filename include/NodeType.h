
#ifndef __NODETYPE__
#define __NODETYPE__

#include "types.h"

#define NODETYPE_DEFINE(type)    static uint_t NodeType
#define NODETYPE_IMPLEMENT(type) uint_t type::NodeType = ANodeType::AllocateNodeType(#type)

#define NODETYPE(type,parent)                                                                                       \
    static  uint_t GetNodeType()                     {return NodeType;}                                             \
    virtual uint_t GetObjectType()      const        {return NodeType;}                                             \
    virtual bool   IsType(uint_t otype) const        {return (otype == NodeType) ? true : parent::IsType(otype);}   \
    static  bool   IsType(const ANodeType *pNode)    {return (pNode && pNode->IsType(NodeType));}                   \
    static  type   *Cast(ANodeType *pNode)           {return IsType(pNode) ? (type *)pNode : NULL;}                 \
    static  const type *Cast(const ANodeType *pNode) {return IsType(pNode) ? (const type *)pNode : NULL;}

class ANodeType {
public:
    ANodeType() {}
    virtual ~ANodeType() {}

    static  uint_t GetNodeType()             {return NodeType;}
    virtual uint_t GetObjectType()     const {return NodeType;}
    virtual bool   IsType(uint_t type) const {return (type == NodeType);}

    static  uint_t AllocateNodeType(const char *type);
    static  const char *GetNodeTypeName(uint_t type);

    const char *GetObjectTypeName() const {return GetNodeTypeName(GetObjectType());}

protected:
    NODETYPE_DEFINE(ANodeType);
};

#endif
