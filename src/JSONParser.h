
#ifndef __JSON_PARSER__
#define __JSON_PARSER__

#include "strsup.h"
#include "DataList.h"

class AJSONString;
class AJSONValue;
class AJSONArray;
class AJSONObject;
class AJSONType {
public:
	AJSONType() : parent(NULL) {}
	virtual ~AJSONType() {}
	
	AJSONType *GetParent() const {return parent;}
	void SetParent(AJSONType *iparent) {parent = iparent;}
	
	virtual void Add(AJSONType *obj) {UNUSED(obj);}
    virtual void Remove(AJSONType *obj) {UNUSED(obj);}
	
    virtual AJSONString *GetAsString() {return NULL;}
    virtual AJSONValue 	*GetAsValue()  {return NULL;}
    virtual AJSONArray 	*GetAsArray()  {return NULL;}
    virtual AJSONObject *GetAsObject() {return NULL;}

	AJSONValue *Find(const AString& name);
	AJSONValue *FindNext(const AString& name);
	AJSONValue *Find(const AString& name, AJSONValue *lastfound);

    virtual AString ToString() const {return "";}

protected:
	AJSONValue *Find(const AString& name, AJSONValue *lastfound, bool& test);
	
protected:
	AJSONType *parent;
};

class AJSONString : public AJSONType {
public:
	AJSONString(const AString& value = "", bool hasquotes = false) : AJSONType() {
		Value     = value;
		HasQuotes = hasquotes;
	}
	virtual ~AJSONString() {}

    virtual AJSONString *GetAsString() {return this;}
	
    virtual AString ToString() const {
		return HasQuotes ? "\"" + Value + "\"" : Value;
    }

	AString Value;
	bool    HasQuotes;
};

class AJSONValue : public AJSONType {
public:
	AJSONValue(const AString& iname = "", AJSONType *ivalue = NULL) : AJSONType(),
																	  value(NULL) {
		SetName(iname);
		SetValue(ivalue);
	} 
	virtual ~AJSONValue() {
		if (value) delete value;
	}

    virtual AJSONValue *GetAsValue() {return this;}

	const AString& GetName() const {return name;}
	void SetName(const AString& iname) {name = iname;}
	
	AJSONType *GetValue() const {return value;}
	void SetValue(AJSONType *ivalue) {
		if (ivalue) ivalue->SetParent(this);
		if (value)  value->SetParent(NULL);
		value = ivalue;
	}

	virtual void Add(AJSONType *obj) {SetValue(obj);}
	virtual void Remove(AJSONType *obj) {if (obj == value) SetValue(NULL);}

    virtual AString ToString() const {
		return "\"" + name + "\":" + (value ? value->ToString() : "<empty>");
    }

protected:
	AString   name;
	AJSONType *value;
};

class AJSONArray : public AJSONType {
public:
	AJSONArray() : AJSONType() {items.SetDestructor(&__DeleteItem);}
	virtual ~AJSONArray() {}

    virtual AJSONArray *GetAsArray() {return this;}

	const ADataList& List() const {return items;}
	uint_t Count() const {return items.Count();}

	virtual void Add(AJSONType *obj) {
		if (obj) {obj->SetParent(this); items.Add(obj);}
	}
	virtual void Remove(AJSONType *obj) {
		if (obj && (items.Remove(obj) >= 0)) obj->SetParent(NULL);
	}

    void AddValue(const AString& value, bool hasquotes);
    AJSONArray *AddArray();
	AJSONObject *AddObject();

	virtual AString ToString() const;

protected:
	static void __DeleteItem(uptr_t item, void *context) {
		UNUSED(context);
		delete (AJSONType *)item;
	}

protected:
	ADataList items;
};

class AJSONObject : public AJSONType {
public:
	AJSONObject() : AJSONType() {items.SetDestructor(&__DeleteItem);}
	virtual ~AJSONObject() {}

    virtual AJSONObject *GetAsObject() {return this;}

	const ADataList& List() const {return items;}
	uint_t Count() const {return items.Count();}

	virtual void Add(AJSONType *obj) {
		if (obj && obj->GetAsValue()) {obj->SetParent(this); items.Add(obj);}
	}
	virtual void Remove(AJSONType *obj) {
		if (obj && (items.Remove(obj) >= 0)) obj->SetParent(NULL);
	}

    void AddValue(const AString& name, const AString& value, bool hasquotes) {
		Add(new AJSONValue(name, new AJSONString(value, hasquotes)));
    }

    AJSONArray *AddArray(const AString& name) {
		AJSONArray *array = new AJSONArray();
		Add(new AJSONValue(name, array));
		return array;
    }
	
    AJSONObject *AddObject(const AString& name) {
		AJSONObject *obj = new AJSONObject();
		Add(new AJSONValue(name, obj));
		return obj;
    }

	virtual AString ToString() const;

protected:
	static void __DeleteItem(uptr_t item, void *context) {
		UNUSED(context);
		delete (AJSONType *)item;
	}

protected:
	ADataList items;
};

class AJSONParser {
public:
	AJSONParser();
	~AJSONParser();

    typedef enum {
		Parsing = 0,							// 0
		Complete,								// 1
		InvalidStartCharacter,					// 2
		StackUnderflow,							// 3
		MissingQuotesAroundName,				// 4
		IllegalNameLocation,					// 5
		IllegalObjectLocation,					// 6
		IllegalArrayLocation,					// 7
		IllegalValueLocation,					// 8
		IllegalObjectClose,						// 9
		IllegalArrayClose,						// 10
		IllegalState,							// 11
		UnrecognizedCharacter,					// 12
		ParsingIncomplete,						// 13
		CharactersInBuffer,						// 14
    } JSONStatus;

	JSONStatus parse(const char *buffer, uint_t count);

	JSONStatus GetStatus() const {return status;}

	AJSONObject *GetTree() {return tree;}

	AString ToString() const;
	
protected:
	typedef enum {
		LookingForStart = 0,					// 0
		LookingForName,							// 1
		LookingForValue,						// 2
		LookingForSeparator,					// 3
    } STATE;

    bool      StackEmpty() {return (stack.Count() == 0);}
    AJSONType *StackTop()  {return (AJSONType *)stack.Last();}
    bool      StackPop()   {stack.EndPop(); return (stack.Count() == 0); }

protected:
	ADataList   stack;
	AJSONObject *tree;
	char  	   	*block;
	uint_t  	   	blocklen, pos, parsepos;
	JSONStatus 	status;
	STATE       state;
};

#endif

