
#ifndef __EVAL_NUMBER__
#define __EVAL_NUMBER__

#include "types.h"

#define NUMSTR(fmt,val) AValue(val).ToString(fmt).str()

enum {
	FORMAT_DEC = 0,
	FORMAT_HEX,
	FORMAT_OCT,
	FORMAT_BIN,
	FORMAT_HEX_C,
	FORMAT_OCT_C,
	FORMAT_BIN_C,
	FORMAT_HEX_NO_PREFIX,
	FORMAT_OCT_NO_PREFIX,
	FORMAT_BIN_NO_PREFIX,

	FORMAT_ITEMS,
};

class AString;
class AValue {
public:
	enum {
		VALUE_INVALID = 0,
		VALUE_SIGNED_INT8,
		VALUE_SIGNED_INT16,
		VALUE_SIGNED_INT,
		VALUE_SIGNED_INT32,
		VALUE_SIGNED_INT64,
		VALUE_UNSIGNED_INT8,
		VALUE_UNSIGNED_INT16,
		VALUE_UNSIGNED_INT,
		VALUE_UNSIGNED_INT32,
		VALUE_UNSIGNED_INT64,
		VALUE_FLOAT,
		VALUE_DOUBLE,

		_VALUE_INTEGER_START = VALUE_SIGNED_INT8,
		_VALUE_INTEGER_END   = VALUE_UNSIGNED_INT64,

		_VALUE_SIGNED_START  = VALUE_SIGNED_INT8,
		_VALUE_SIGNED_END    = VALUE_SIGNED_INT64,

		_VALUE_FLOAT_START   = VALUE_FLOAT,
		_VALUE_FLOAT_END	 = VALUE_DOUBLE,
		
		VALUE_ITEMS,

		VALUE_EXTENDED = VALUE_ITEMS,
	};

	AValue();
	AValue(sint8_t  val);
	AValue(uint8_t  val);
	AValue(sint16_t val);
	AValue(uint16_t val);
#if !LONG_IS_64BITS
	AValue(sint_t   val);
	AValue(uint_t   val);
#endif
	AValue(sint32_t val);
	AValue(uint32_t val);
	AValue(sint64_t val);
	AValue(uint64_t val);
	AValue(float  	val);
	AValue(double 	val);
	AValue(const AValue& val);
	AValue(const void *p, uint_t type);
	~AValue();

	void Clear() {Type = VALUE_INVALID; Value.u = 0;}

	AValue& operator = (bool     val) {return operator = ((uint8_t)val);}
	AValue& operator = (sint8_t  val);
	AValue& operator = (uint8_t  val);
	AValue& operator = (sint16_t val);
	AValue& operator = (uint16_t val);
#if !LONG_IS_64BITS
	AValue& operator = (sint_t   val);
	AValue& operator = (uint_t   val);
#endif
	AValue& operator = (sint32_t val);
	AValue& operator = (uint32_t val);
	AValue& operator = (sint64_t val);
	AValue& operator = (uint64_t val);
	AValue& operator = (float  	 val);
	AValue& operator = (double 	 val);
	AValue& operator = (const AValue& val);

	bool Set(const void *p, uint_t type);
	bool Get(void *p, uint_t type);

	static uint_t GetTypeSize(uint_t type);
	static uint_t GetType(const char *text, bool caseSensitive = false);

	uint_t GetType()   const {return Type;}
	void SetType(uint_t type) {Type = type;}

	bool IsValid()   const {return (Type != VALUE_INVALID);}
	bool IsInteger() const {return RANGE(Type, _VALUE_INTEGER_START, _VALUE_INTEGER_END);}
	bool IsSigned()  const {return RANGE(Type, _VALUE_SIGNED_START,  _VALUE_SIGNED_END);}
	bool IsFloat()   const {return RANGE(Type, _VALUE_FLOAT_START,   _VALUE_FLOAT_END);}

	bool IsZero()    const {return (Value.u == 0);}
	bool IsNonZero() const {return (Value.u != 0);}

	void SetReadOnly(bool ro = true) {bReadOnly = ro;}

	bool IsReadOnly() const {return bReadOnly;}
	bool IsExtended() const {return (Type >= VALUE_EXTENDED);}

	operator sint8_t()  const {return (sint8_t)operator sint64_t();}
	operator uint8_t()  const {return (uint8_t)operator uint64_t();}
	operator sint16_t() const {return (sint16_t)operator sint64_t();}
	operator uint16_t() const {return (uint16_t)operator uint64_t();}
#if !LONG_IS_64BITS
	operator sint_t() 	const {return (sint_t)operator sint64_t();}
	operator uint_t() 	const {return (uint_t)operator uint64_t();}
#endif
	operator sint32_t() const {return (sint32_t)operator sint64_t();}
	operator uint32_t() const {return (uint32_t)operator uint64_t();}
	operator sint64_t() const;
	operator uint64_t() const;

	operator float()  	const;
	operator double() 	const;

	static uint_t TypeOf(sint8_t  val) 		{UNUSED(val); return VALUE_SIGNED_INT8;}
	static uint_t TypeOf(sint16_t val) 		{UNUSED(val); return VALUE_SIGNED_INT16;}
#if !LONG_IS_64BITS
	static uint_t TypeOf(sint_t   val) 		{UNUSED(val); return VALUE_SIGNED_INT;}
#endif
	static uint_t TypeOf(sint32_t val) 		{UNUSED(val); return VALUE_SIGNED_INT32;}
	static uint_t TypeOf(sint64_t val) 		{UNUSED(val); return VALUE_SIGNED_INT64;}
	static uint_t TypeOf(uint8_t  val) 		{UNUSED(val); return VALUE_UNSIGNED_INT8;}
	static uint_t TypeOf(uint16_t val) 		{UNUSED(val); return VALUE_UNSIGNED_INT16;}
#if !LONG_IS_64BITS
	static uint_t TypeOf(uint_t   val) 		{UNUSED(val); return VALUE_UNSIGNED_INT;}
#endif
	static uint_t TypeOf(uint32_t val) 		{UNUSED(val); return VALUE_UNSIGNED_INT32;}
	static uint_t TypeOf(uint64_t val) 		{UNUSED(val); return VALUE_UNSIGNED_INT64;}
	static uint_t TypeOf(float    val) 		{UNUSED(val); return VALUE_FLOAT;}
	static uint_t TypeOf(double   val) 		{UNUSED(val); return VALUE_DOUBLE;}
	static uint_t TypeOf(const sint8_t  *p) {UNUSED(p);   return VALUE_SIGNED_INT8;}
	static uint_t TypeOf(const sint16_t *p) {UNUSED(p);   return VALUE_SIGNED_INT16;}
#if !LONG_IS_64BITS
	static uint_t TypeOf(const sint_t   *p) {UNUSED(p);   return VALUE_SIGNED_INT;}
#endif
	static uint_t TypeOf(const sint32_t *p) {UNUSED(p);   return VALUE_SIGNED_INT32;}
	static uint_t TypeOf(const sint64_t *p) {UNUSED(p);   return VALUE_SIGNED_INT64;}
	static uint_t TypeOf(const uint8_t  *p) {UNUSED(p);   return VALUE_UNSIGNED_INT8;}
	static uint_t TypeOf(const uint16_t *p) {UNUSED(p);   return VALUE_UNSIGNED_INT16;}
#if !LONG_IS_64BITS
	static uint_t TypeOf(const uint_t   *p) {UNUSED(p);   return VALUE_UNSIGNED_INT;}
#endif
	static uint_t TypeOf(const uint32_t *p) {UNUSED(p);   return VALUE_UNSIGNED_INT32;}
	static uint_t TypeOf(const uint64_t *p) {UNUSED(p);   return VALUE_UNSIGNED_INT64;}
	static uint_t TypeOf(const float  	*p) {UNUSED(p);   return VALUE_FLOAT;}
	static uint_t TypeOf(const double 	*p) {UNUSED(p);   return VALUE_DOUBLE;}

	AValue& ApplyModifier(char modifier, AString *error = NULL);
	AValue& ApplyModifiers(const char *p1, const char *p2, AString *error = NULL);

	static const char *FindEndOfModifiers(const char *p, const char *terminators = NULL);
	static bool IsNumberString(const char *p, bool allowModifiers = true);
	static bool IsNumberString(const AString& str, uint_t p = 0, bool allowModifiers = true);

	static AValue EvalNumberEx(const char *p, bool allowModifiers = true, const char *terminators = NULL, AString *error = NULL) {
		AValue val;
		val.EvalNumber(p, allowModifiers, terminators, error);
		return val;
	}

	const char *EvalNumber(const char *p, bool allowModifiers = true, const char *terminators = NULL, AString *error = NULL);
	uint_t EvalNumber(const AString& str, uint_t p, bool allowModifiers = true, const char *terminators = NULL, AString *error = NULL);

	const AValue& GenerateString(AString& str) const;
	const AValue& GenerateStringNice(AString& str, uint_t format = FORMAT_DEC, uint_t decPlaces = 3, uint_t fieldSize = 0, char prefix = ' ') const;

	AString GenerateString() const;
	AString GenerateStringNice(uint_t format = FORMAT_DEC, uint_t decPlaces = 3, uint_t fieldSize = 0, char prefix = ' ') const;

	AString ToString(const char *fmt = "n") const;
	
	operator AString() const;

protected:
	uint8_t Type;
	uint8_t bReadOnly;
	union {
		sint64_t i;
		sint64_t u;
		double   f;
	} Value;

	static uint_t TypeSizes[VALUE_ITEMS];
};

#endif
