
#ifndef __EVAL_NUMBER__
#define __EVAL_NUMBER__

#include "types.h"

class AString;
class AValue {
public:
	enum {
		VALUE_INVALID = 0,
		VALUE_SIGNED_INT8,
		VALUE_SIGNED_SHORT,
		VALUE_SIGNED_INT,
		VALUE_SIGNED_LONG,
		VALUE_SIGNED_LLONG,
		VALUE_UNSIGNED_INT8,
		VALUE_UNSIGNED_SHORT,
		VALUE_UNSIGNED_INT,
		VALUE_UNSIGNED_LONG,
		VALUE_UNSIGNED_LLONG,
		VALUE_FLOAT,
		VALUE_DOUBLE,

		_VALUE_INTEGER_START = VALUE_SIGNED_INT8,
		_VALUE_INTEGER_END   = VALUE_UNSIGNED_LLONG,

		_VALUE_SIGNED_START  = VALUE_SIGNED_INT8,
		_VALUE_SIGNED_END    = VALUE_SIGNED_LLONG,

		_VALUE_FLOAT_START   = VALUE_FLOAT,
		_VALUE_FLOAT_END	 = VALUE_DOUBLE,
		
		VALUE_ITEMS,

		VALUE_EXTENDED = VALUE_ITEMS,
	};

	AValue();
	AValue(sint8_t  val);
	AValue(uint8_t  val);
	AValue(sshort_t val);
	AValue(ushort_t val);
	AValue(sint_t   val);
	AValue(uint_t   val);
	AValue(slong_t 	val);
	AValue(ulong_t 	val);
	AValue(sllong_t val);
	AValue(ullong_t val);
	AValue(float  	val);
	AValue(double 	val);
	AValue(const AValue& val);
	AValue(const void *p, uint_t type);
	~AValue();

	void Clear() {Type = VALUE_INVALID; Value.u = 0;}

	AValue& operator = (bool     val) {return operator = ((uint8_t)val);}
	AValue& operator = (sint8_t  val);
	AValue& operator = (uint8_t  val);
	AValue& operator = (sshort_t val);
	AValue& operator = (ushort_t val);
	AValue& operator = (sint_t   val);
	AValue& operator = (uint_t   val);
	AValue& operator = (slong_t  val);
	AValue& operator = (ulong_t  val);
	AValue& operator = (sllong_t val);
	AValue& operator = (ullong_t val);
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
	operator sshort_t() const {return (sshort_t)operator sint64_t();}
	operator ushort_t() const {return (ushort_t)operator uint64_t();}
	operator sint_t() 	const {return (sint_t)operator sint64_t();}
	operator uint_t() 	const {return (uint_t)operator uint64_t();}
#if LONG_IS_64BITS
	operator slong_t() 	const;
	operator ulong_t() 	const;
	operator sllong_t() const {return (sllong_t)operator sint64_t();}
	operator ullong_t() const {return (ullong_t)operator uint64_t();}
#else
	operator slong_t() 	const {return (slong_t)operator sint64_t();}
	operator ulong_t() 	const {return (ulong_t)operator uint64_t();}
	operator sllong_t() const;
	operator ullong_t() const;
#endif
	
	operator float()  	const;
	operator double() 	const;

	static uint_t TypeOf(sint8_t  val) 		{UNUSED(val); return VALUE_SIGNED_INT8;}
	static uint_t TypeOf(uint8_t  val) 		{UNUSED(val); return VALUE_UNSIGNED_INT8;}
	static uint_t TypeOf(sshort_t val) 		{UNUSED(val); return VALUE_SIGNED_SHORT;}
	static uint_t TypeOf(ushort_t val) 		{UNUSED(val); return VALUE_UNSIGNED_SHORT;}
	static uint_t TypeOf(sint_t   val) 		{UNUSED(val); return VALUE_SIGNED_INT;}
	static uint_t TypeOf(uint_t   val) 		{UNUSED(val); return VALUE_UNSIGNED_INT;}
	static uint_t TypeOf(slong_t  val) 		{UNUSED(val); return VALUE_SIGNED_LONG;}
	static uint_t TypeOf(ulong_t  val) 		{UNUSED(val); return VALUE_UNSIGNED_LONG;}
	static uint_t TypeOf(sllong_t val) 		{UNUSED(val); return VALUE_SIGNED_LLONG;}
	static uint_t TypeOf(ullong_t val) 		{UNUSED(val); return VALUE_UNSIGNED_LLONG;}
	static uint_t TypeOf(float    val) 		{UNUSED(val); return VALUE_FLOAT;}
	static uint_t TypeOf(double   val) 		{UNUSED(val); return VALUE_DOUBLE;}
	static uint_t TypeOf(const sint8_t  *p) {UNUSED(p);   return VALUE_SIGNED_INT8;}
	static uint_t TypeOf(const uint8_t  *p) {UNUSED(p);   return VALUE_UNSIGNED_INT8;}
	static uint_t TypeOf(const sshort_t *p) {UNUSED(p);   return VALUE_SIGNED_SHORT;}
	static uint_t TypeOf(const ushort_t *p) {UNUSED(p);   return VALUE_UNSIGNED_SHORT;}
	static uint_t TypeOf(const sint_t   *p) {UNUSED(p);   return VALUE_SIGNED_INT;}
	static uint_t TypeOf(const uint_t   *p) {UNUSED(p);   return VALUE_UNSIGNED_INT;}
	static uint_t TypeOf(const slong_t  *p) {UNUSED(p);   return VALUE_SIGNED_LONG;}
	static uint_t TypeOf(const ulong_t  *p) {UNUSED(p);   return VALUE_UNSIGNED_LONG;}
	static uint_t TypeOf(const sllong_t *p) {UNUSED(p);   return VALUE_SIGNED_LLONG;}
	static uint_t TypeOf(const ullong_t *p) {UNUSED(p);   return VALUE_UNSIGNED_LLONG;}
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

	AString ToString(const char *format = "") const;
	
	operator AString() const;

protected:
	uint8_t Type;
	uint8_t bReadOnly;
	union {
		sint64_t i;
		uint64_t u;
		double   f;
	} Value;

	static uint_t TypeSizes[VALUE_ITEMS];
};

#endif
