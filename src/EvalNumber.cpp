
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "EvalNumber.h"
#include "strsup.h"

/* end of includes */

static const char *Modifiers = "+-/~!";
static const struct {
	const char *Name;
	uint_t     Type;
} TypeDefs[] = {
	{"byte",   		  AValue::VALUE_UNSIGNED_INT8},
	{"int8",   		  AValue::VALUE_UNSIGNED_INT8},
	{"word",   		  AValue::VALUE_UNSIGNED_SHORT},
	{"short",  		  AValue::VALUE_UNSIGNED_SHORT},
	{"int",   		  AValue::VALUE_UNSIGNED_INT},
	{"long",  		  AValue::VALUE_UNSIGNED_LONG},
	{"llong",  		  AValue::VALUE_UNSIGNED_LLONG},
	{"signed-byte",   AValue::VALUE_SIGNED_INT8},
	{"signed-int8",   AValue::VALUE_SIGNED_INT8},
	{"signed-word",   AValue::VALUE_SIGNED_SHORT},
	{"signed-short",  AValue::VALUE_SIGNED_SHORT},
	{"signed-int",    AValue::VALUE_SIGNED_INT},
	{"signed-long",   AValue::VALUE_SIGNED_LONG},
	{"signed-llong",  AValue::VALUE_SIGNED_LLONG},
	{"ptr", 		  AValue::VALUE_POINTER},
	{"float",  		  AValue::VALUE_FLOAT},
	{"double", 		  AValue::VALUE_DOUBLE},
	{"ub",   		  AValue::VALUE_UNSIGNED_INT8},
	{"u8",   		  AValue::VALUE_UNSIGNED_INT8},
	{"uw",   		  AValue::VALUE_UNSIGNED_SHORT},
	{"u16",   		  AValue::VALUE_UNSIGNED_SHORT},
	{"ul",   		  AValue::VALUE_UNSIGNED_LONG},
	{"u32",   		  AValue::VALUE_UNSIGNED_LONG},
	{"ud",  		  AValue::VALUE_UNSIGNED_LLONG},
	{"u64",  		  AValue::VALUE_UNSIGNED_LLONG},
	{"sb",    		  AValue::VALUE_SIGNED_INT8},
	{"s8",    		  AValue::VALUE_SIGNED_INT8},
	{"sw",    		  AValue::VALUE_SIGNED_SHORT},
	{"s16",    		  AValue::VALUE_SIGNED_SHORT},
	{"sl",    		  AValue::VALUE_SIGNED_LONG},
	{"s32",    		  AValue::VALUE_SIGNED_LONG},
	{"sd",    		  AValue::VALUE_SIGNED_LLONG},
	{"s64",    		  AValue::VALUE_SIGNED_LLONG},
	{"p", 		  	  AValue::VALUE_POINTER},
	{"f",  		  	  AValue::VALUE_FLOAT},
	{"d", 		  	  AValue::VALUE_DOUBLE},
};

uint_t AValue::TypeSizes[VALUE_ITEMS] = {
	0,
	sizeof(sint8_t),
	sizeof(sshort_t),
	sizeof(sint_t),
	sizeof(slong_t),
	sizeof(sllong_t),
	sizeof(uint8_t),
	sizeof(ushort_t),
	sizeof(uint_t),
	sizeof(ulong_t),
	sizeof(ullong_t),
	sizeof(void *),
	sizeof(float),
	sizeof(double),
};

AValue::AValue() : Type(VALUE_INVALID),
				   bReadOnly(false)
{
	memset(&Value, 0, sizeof(Value));
}

AValue::AValue(sint8_t val) : Type(VALUE_INVALID),
							  bReadOnly(false)
{
	memset(&Value, 0, sizeof(Value));
	operator = (val);
}

AValue::AValue(uint8_t val) : Type(VALUE_INVALID),
							  bReadOnly(false)
{
	memset(&Value, 0, sizeof(Value));
	operator = (val);
}

AValue::AValue(sshort_t val) : Type(VALUE_INVALID),
							   bReadOnly(false)
{
	memset(&Value, 0, sizeof(Value));
	operator = (val);
}

AValue::AValue(ushort_t val) : Type(VALUE_INVALID),
							   bReadOnly(false)
{
	memset(&Value, 0, sizeof(Value));
	operator = (val);
}

AValue::AValue(sint_t val) : Type(VALUE_INVALID),
							 bReadOnly(false)
{
	memset(&Value, 0, sizeof(Value));
	operator = (val);
}

AValue::AValue(uint_t val) : Type(VALUE_INVALID),
							 bReadOnly(false)
{
	memset(&Value, 0, sizeof(Value));
	operator = (val);
}

AValue::AValue(slong_t val) : Type(VALUE_INVALID),
							  bReadOnly(false)
{
	memset(&Value, 0, sizeof(Value));
	operator = (val);
}

AValue::AValue(ulong_t val) : Type(VALUE_INVALID),
							  bReadOnly(false)
{
	memset(&Value, 0, sizeof(Value));
	operator = (val);
}

AValue::AValue(sllong_t val) : Type(VALUE_INVALID),
							   bReadOnly(false)
{
	memset(&Value, 0, sizeof(Value));
	operator = (val);
}

AValue::AValue(ullong_t val) : Type(VALUE_INVALID),
							   bReadOnly(false)
{
	memset(&Value, 0, sizeof(Value));
	operator = (val);
}

AValue::AValue(const void *val) : Type(VALUE_INVALID),
								  bReadOnly(false)
{
	memset(&Value, 0, sizeof(Value));
	operator = (val);
}

AValue::AValue(float val) : Type(VALUE_INVALID),
							bReadOnly(false)
{
	memset(&Value, 0, sizeof(Value));
	operator = (val);
}

AValue::AValue(double val) : Type(VALUE_INVALID),
							 bReadOnly(false)
{
	memset(&Value, 0, sizeof(Value));
	operator = (val);
}

AValue::AValue(const AValue& val) : Type(VALUE_INVALID),
									bReadOnly(false)
{
	memset(&Value, 0, sizeof(Value));
	operator = (val);
}

AValue::AValue(const void *p, uint_t type) : Type(VALUE_INVALID),
											 bReadOnly(false)
{
	memset(&Value, 0, sizeof(Value));
	Set(p, type);
}

AValue::~AValue()
{
	Clear();
}

uint_t AValue::GetTypeSize(uint_t type)
{
	if (type < VALUE_ITEMS) return TypeSizes[type];
	return 0;
}

uint_t AValue::GetType(const char *text, bool caseSensitive)
{
	uint_t i;

	if (caseSensitive) {
		for (i = 0; i < NUMBEROF(TypeDefs); i++) {
			if (CompareCase(text, TypeDefs[i].Name) == 0) break;
		}
	}
	else {
		for (i = 0; i < NUMBEROF(TypeDefs); i++) {
			if (CompareNoCase(text, TypeDefs[i].Name) == 0) break;
		}
	}

	if (i < NUMBEROF(TypeDefs)) return TypeDefs[i].Type;

	return VALUE_INVALID;
}

AValue& AValue::operator = (sint8_t val)
{
	Clear();
	Type    = VALUE_SIGNED_INT8;
	Value.i = (sllong_t)val;
	return *this;
}

AValue& AValue::operator = (uint8_t val)
{
	Clear();
	Type    = VALUE_UNSIGNED_INT8;
	Value.u = (ullong_t)val;
	return *this;
}

AValue& AValue::operator = (sshort_t val)
{
	Clear();
	Type    = VALUE_SIGNED_SHORT;
	Value.i = (sllong_t)val;
	return *this;
}

AValue& AValue::operator = (ushort_t val)
{
	Clear();
	Type    = VALUE_UNSIGNED_SHORT;
	Value.u = (ullong_t)val;
	return *this;
}

AValue& AValue::operator = (sint_t val)
{
	Clear();
	Type    = VALUE_SIGNED_INT;
	Value.i = (sllong_t)val;
	return *this;
}

AValue& AValue::operator = (uint_t val)
{
	Clear();
	Type    = VALUE_UNSIGNED_INT;
	Value.u = (ullong_t)val;
	return *this;
}

AValue& AValue::operator = (slong_t val)
{
	Clear();
	Type    = VALUE_SIGNED_LONG;
	Value.i = (sllong_t)val;
	return *this;
}

AValue& AValue::operator = (ulong_t val)
{
	Clear();
	Type    = VALUE_UNSIGNED_LONG;
	Value.u = (ullong_t)val;
	return *this;
}

AValue& AValue::operator = (sllong_t val)
{
	Clear();
	Type    = VALUE_SIGNED_LLONG;
	Value.i = (sint64_t)val;
	return *this;
}

AValue& AValue::operator = (ullong_t val)
{
	Clear();
	Type    = VALUE_UNSIGNED_LLONG;
	Value.u = (uint64_t)val;
	return *this;
}

AValue& AValue::operator = (const void *val)
{
	Clear();
	Type    = VALUE_POINTER;
	Value.u = (uint64_t)val;
	return *this;
}

AValue& AValue::operator = (float val)
{
	Clear();
	Type    = VALUE_FLOAT;
	Value.f = val;
	return *this;
}

AValue& AValue::operator = (double val)
{
	Clear();
	Type    = VALUE_DOUBLE;
	Value.f = val;
	return *this;
}

AValue& AValue::operator = (const AValue& val)
{
	Clear();
	Type      = val.Type;
	bReadOnly = val.bReadOnly;
	Value     = val.Value;
	return *this;
}

AValue::operator sint64_t() const
{
	if      (IsInteger()) return Value.i;
	else if (IsFloat())   return (sint64_t)Value.f;
	return 0;
}

AValue::operator uint64_t() const
{
	if      (IsInteger()) return Value.u;
	else if (IsFloat())   return (uint64_t)Value.f;
	return 0;
}

AValue::operator const void *() const
{
	if (IsInteger()) return (const void *)Value.u;
	return NULL;
}

AValue::operator float() const
{
	if      (IsInteger()) return (float)Value.i;
	else if (IsFloat())   return (float)Value.f;
	return 0.0;
}

AValue::operator double() const
{
	if      (IsInteger()) return (double)Value.i;
	else if (IsFloat())   return Value.f;
	return 0.0;
}

bool AValue::Set(const void *p, uint_t type)
{
	bool success = true;

	switch (type) {
		case VALUE_SIGNED_INT8:
			*this = *(sint8_t *)p;
			break;
		case VALUE_SIGNED_SHORT:
			*this = *(sshort_t *)p;
			break;
		case VALUE_SIGNED_INT:
			*this = *(sint_t *)p;
			break;
		case VALUE_SIGNED_LONG:
			*this = *(slong_t *)p;
			break;
		case VALUE_SIGNED_LLONG:
			*this = *(sllong_t *)p;
			break;
		case VALUE_UNSIGNED_INT8:
			*this = *(uint8_t *)p;
			break;
		case VALUE_UNSIGNED_SHORT:
			*this = *(ushort_t *)p;
			break;
		case VALUE_UNSIGNED_INT:
			*this = *(uint_t *)p;
			break;
		case VALUE_UNSIGNED_LONG:
			*this = *(ulong_t *)p;
			break;
		case VALUE_UNSIGNED_LLONG:
			*this = *(ullong_t *)p;
			break;
		case VALUE_POINTER:
			*this = (const void *)p;
			break;
		case VALUE_FLOAT:
			*this = *(float *)p;
			break;
		case VALUE_DOUBLE:
			*this = *(double *)p;
			break;
		default:
			success = false;
			break;
	}

	return success;
}

bool AValue::Get(void *p, uint_t type)
{
	bool success = false;

	if (IsValid()) {
		success = true;

		switch (type) {
			case VALUE_SIGNED_INT8:
				*(sint8_t *)p = (sint8_t)*this;
				break;
			case VALUE_SIGNED_SHORT:
				*(sshort_t *)p = (sshort_t)*this;
				break;
			case VALUE_SIGNED_INT:
				*(sint_t *)p = (sint_t)*this;
				break;
			case VALUE_SIGNED_LONG:
				*(slong_t *)p = (slong_t)*this;
				break;
			case VALUE_SIGNED_LLONG:
				*(sllong_t *)p = (sllong_t)*this;
				break;
			case VALUE_UNSIGNED_INT8:
				*(uint8_t *)p = (uint8_t)*this;
				break;
			case VALUE_UNSIGNED_SHORT:
				*(ushort_t *)p = (ushort_t)*this;
				break;
			case VALUE_UNSIGNED_INT:
				*(uint_t *)p = (uint_t)*this;
				break;
			case VALUE_UNSIGNED_LONG:
				*(ulong_t *)p = (ulong_t)*this;
				break;
			case VALUE_UNSIGNED_LLONG:
				*(ullong_t *)p = (ullong_t)*this;
				break;
			case VALUE_POINTER:
				*(const void **)p = (const void *)*this;
				break;
			case VALUE_FLOAT:
				*(float *)p = (float)*this;
				break;
			case VALUE_DOUBLE:
				*(double *)p = (double)*this;
				break;
			default:
				success = false;
				break;
		}
	}

	return success;
}

AValue& AValue::ApplyModifier(char modifier, AString *error)
{
	if (IsValid()) {
		double val;

		if (error) error->Delete();

		switch (modifier) {
			case '+':
				break;
				
			case '-':
				if (IsInteger()) *this = -Value.i;
				else			 *this = -Value.f;
				break;
				
			case '~':
				*this = ~(ullong_t)*this;
				break;
				
			case '!':
				if (IsInteger()) *this = (Value.u == 0);
				else			 *this = (Value.f == 0.0);
				break;
				
			case '/':
				val = (double)*this;
				if (val != 0.0) *this = 1.0 / val;
				else {
					Clear();
					if (error) error->Format("Division by zero");
				}
				break;
				
			default:
				Clear();
				if (error) error->Format("Unknown modifier '%c'", modifier);
				break;
		}
	}
	else if (error) error->Format("Uninitialized value");

	return *this;
}

AValue& AValue::ApplyModifiers(const char *p1, const char *p2, AString *error)
{
	if (error) error->Delete();

	if (p1 && p2) {
		if (IsValid()) {
			const char *p;
			
			for (p = p2 - 1; p >= p1; p--) {
				ApplyModifier(p[0], error);
				if (!IsValid()) break;
			}
		}
		else if (error) error->Format("Uninitialized value");
	}

	return *this;
}

const char *AValue::FindEndOfModifiers(const char *p, const char *terminators)
{
	while (p[0] && strchr(Modifiers, p[0]) && (!terminators || !strchr(terminators, p[0]))) p++;
	return p;
}

bool AValue::IsNumberString(const char *p, bool allowModifiers)
{
	bool valid = false;

	if (allowModifiers) p = FindEndOfModifiers(p);
	
	if      (IsNumeralChar(p[0]))						 valid = true;
	else if (IsPointChar(p[0]) && IsNumeralChar(p[1]))   valid = true;
	else if (IsDoubleStartChar(p[0]) && IsHexChar(p[1])) valid = true;
	else if (IsHexStartChar(p[0]) && IsHexChar(p[1]))	 valid = true;
	else if (IsOctStartChar(p[0]) && IsOctChar(p[1]))	 valid = true;
	else if (IsHexString(p) && IsHexChar(p[2]))   		 valid = true;
	else if (IsOctString(p) && IsOctChar(p[2])) 		 valid = true;
	else if (IsBinString(p) && IsBinChar(p[2]))			 valid = true;

	return valid;
}

bool AValue::IsNumberString(const AString& str, uint_t p, bool allowModifiers)
{
	return IsNumberString(str.str() + p, allowModifiers);
}

const char *AValue::EvalNumber(const char *p, bool allowModifiers, const char *terminators, AString *error)
{
	static uint8_t charlookup[256];
	static bool    lookupinited = false;
	enum {
		FORMAT_DEC = 0,
		FORMAT_HEX,
		FORMAT_OCT,
		FORMAT_BIN,
	};	
	const char *modstart = NULL, *modend = NULL, *p0 = p;
	uint_t i;
	
	if (!lookupinited) {
		lookupinited = true;

		memset(charlookup, 255, sizeof(charlookup));

		for (i = 0; i < NUMBEROF(charlookup); i++) {
			char c = (char)i;
			if      (IsNumeralChar(c))  charlookup[i] = (c - '0');
			else if (IsHexUpperChar(c)) charlookup[i] = (c - 'A') + 10;
			else if (IsHexLowerChar(c)) charlookup[i] = (c - 'a') + 10;
		}
	}

	Clear();

	if (error) error->Delete();

	if (allowModifiers) {
		modstart = p;
		modend   = p = FindEndOfModifiers(modstart, terminators);
	}

	if (IsDoubleStartChar(p[0])) {
		uint64_t val = 0;

		p++;

		while (p[0] && (!terminators || !strchr(terminators, p[0]))) {
			uint_t v = charlookup[(uint_t)p[0]];
			if (v < 16) {
				val = (val << 4) + v;
				p++;
			}
			else break;
		}

		memcpy(&Value.f, &val, sizeof(val));
		Type = VALUE_DOUBLE;
	}
	else if (IsHexStartChar(p[0]) || IsOctStartChar(p[0]) || IsHexString(p) || IsBinString(p) || IsOctString(p) || IsNumeralChar(p[0]) || IsPointChar(p[0])) {
		static const uint_t radixes[] = {10, 16, 8, 2};
		uint_t type = FORMAT_DEC;

		if      (IsHexStartChar(p[0])) {type = FORMAT_HEX; p++;}
		else if (IsOctStartChar(p[0])) {type = FORMAT_OCT; p++;}
		else if (IsHexString(p))  	   {type = FORMAT_HEX; p += 2;}
		else if (IsOctString(p))  	   {type = FORMAT_OCT; p += 2;}
		else if (IsBinString(p))  	   {type = FORMAT_BIN; p += 2;}
		
		uint_t radix   = radixes[type];
 		bool   integer = true;
		for (i = 0; p[i] && integer && (!terminators || !strchr(terminators, p[i])); i++) {
			uint_t v = charlookup[(uint_t)p[i]];
			if      (v < radix) ;
			else if (IsPointChar(p[i])			   ||
					 (IsEngSymbolChar(p[i])		   &&
					  (IsNumeralChar(p[i + 1])     ||
					   IsHexStartChar(p[i + 1])    ||
					   IsHexString(p + i + 1)      ||
					   ((IsPositiveChar(p[i + 1])  ||
						 IsNegativeChar(p[i + 1])) &&
						(IsNumeralChar(p[i + 2])   ||
						 IsHexStartChar(p[i + 2])  ||
						 IsHexString(p + i + 2)))))) integer = false;
			else break;
		}

		if (integer) {
			uint64_t lradix = radix;
			uint64_t val = 0;

			while (p[0] && (!terminators || !strchr(terminators, p[0]))) {
				uint_t v = charlookup[(uint_t)p[0]];
				if (v < radix) {val = val * lradix + v; p++;}
				else break;
			}

			Value.u = val;
			Type = TypeOf(val);
		}
		else {
			double fradix = (double)radix, div = 1.0, divmul = 1.0, val = 0.0;
			int    expo = 0;
			bool   point = false, eng = false, negexp = false;
			
			while (p[0] && (!terminators || !strchr(terminators, p[0]))) {
				uint_t v = charlookup[(uint_t)p[0]];
				if      (v < radix)                   {val = val * fradix + (double)v; div *= divmul; p++;}
				else if (!point && IsPointChar(p[0])) {divmul = fradix; point = true; p++;}
				else if (IsEngSymbolChar(p[0])    &&
						 (IsNumeralChar(p[1])  	  ||
						  IsHexStartChar(p[1]) 	  ||
						  IsHexString(p + 1)   	  ||
						  ((IsPositiveChar(p[1])  ||
							IsNegativeChar(p[1])) &&
						   (IsNumeralChar(p[2])   ||
							IsHexStartChar(p[2])  ||
							IsHexString(p + 2))))) {
					p++;
					eng = true;
					if      (IsPositiveChar(p[0])) p++;
					else if (IsNegativeChar(p[0])) {negexp = true; p++;}
					break;
				}
				else break;
			}
			
			if (eng) {
				uint_t radix = 10;
				
				if      (IsHexStartChar(p[0])) {radix = 16; p++;}
				else if (IsOctStartChar(p[0])) {radix =  8; p++;}
				else if (IsHexString(p))  	   {radix = 16; p += 2;}
				else if (IsOctString(p))  	   {radix =  8; p += 2;}
				else if (IsBinString(p))  	   {radix =  2; p += 2;}
				
				while (p[0] && (!terminators || !strchr(terminators, p[0]))) {
					uint_t v = charlookup[(uint_t)p[0]];
					if	 (v < radix) {expo = expo * radix + v; p++;}
					else break;
				}
			}

			if (expo != 0) {
				if (negexp) val *= pow(radix, -(double)expo);
				else		val *= pow(radix,  (double)expo);
			}
			
			Value.f = val / div;
			Type = VALUE_DOUBLE;
		} 
	}
	else return p0;

	assert(IsValid());
	ApplyModifiers(modstart, modend, error);

	return p;
}

uint_t AValue::EvalNumber(const AString& str, uint_t p, bool allowModifiers, const char *terminators, AString *error)
{
	const char *pstr = str.str(); 
	return EvalNumber(pstr + p, allowModifiers, terminators, error) - pstr;
}

AString AValue::ToString(const char *format) const
{
	AString str;

	if (Type != VALUE_INVALID) {
		char    field[16];
		AString fmt;
		uint_t  l;
		char    formatchar = 0;

		field[0] = '%';
		strcpy(field + 1, format);
		l = strlen(field);
		
		if (l && IsAlphaChar(field[l - 1]) && (field[l - 1] != 'l')) {
			formatchar = field[l - 1];
			field[l - 1] = 0;
		}

		if (formatchar) {
			static const char legalintegerformatchars[] = "udxzp";
			static const char legalfloatformatchars[]   = "feg";

			if (IsInteger()) {
				if (!strchr(legalintegerformatchars, tolower(formatchar))) formatchar = 0;
			}
			else if (!strchr(legalfloatformatchars, tolower(formatchar))) formatchar = 0;
		}
		
		if (!formatchar) {
			if (IsInteger()) formatchar = IsSigned() ? 'd' : (IsPointer() ? 'x' : 'u');
			else			 formatchar = 'f';
		}

		if ((strlen(field) == 1) && IsPointer()) {
#if SYSTEM_IS_64BITS
			strcpy(field, "0x%016");
#else
			strcpy(field, "0x%08");
#endif
		}
		
		if (IsInteger()) {
			if (IsSigned()) {
				if (Type == VALUE_SIGNED_LLONG) {
					fmt.printf("%sll%c", field, formatchar);
					str.printf(fmt, (sllong_t)Value.i);
				}
				else {
					fmt.printf("%sl%c", field, formatchar);
					str.printf(fmt, (slong_t)Value.i);
				}
			}
			else if (IsPointer()) {
#if LONG_IS_64BITS
				fmt.printf("%sl%c", field, formatchar);
				str.printf(fmt, (ulong_t)Value.u);
#else
				fmt.printf("%sll%c", field, formatchar);
				str.printf(fmt, (ullong_t)Value.u);
#endif
			}
			else if (Type == VALUE_UNSIGNED_LLONG) {
				fmt.printf("%sll%c", field, formatchar);
				str.printf(fmt, (ullong_t)Value.u);
			}
			else {
				fmt.printf("%sl%c", field, formatchar);
				str.printf(fmt, (ulong_t)Value.u);
			}
		}
		else {
			fmt.printf("%sl%c", field, formatchar);
			str.printf(fmt, Value.f);
		}
	}
	
	return str;
}

AValue::operator AString() const
{
	return ToString();
}

AValue operator + (const AValue& val1, const AValue& val2)
{
	AValue res;
	if		(val1.IsPointer() && val2.IsSigned())  res = (const void *)((uint64_t)val1 + (sint64_t)val2);
	else if	(val1.IsPointer())					   res = (const void *)((uint64_t)val1 + (uint64_t)val2);
	else if (val1.IsSigned()  && val2.IsPointer()) res = (const void *)((uint64_t)val1 + (sint64_t)val2);
	else if (					 val2.IsPointer()) res = (const void *)((uint64_t)val1 + (uint64_t)val2);
	else if	(val1.IsFloat()   || val2.IsFloat())   res = (double)val1   + (double)val2;
	else if (val1.IsSigned()  || val2.IsSigned())  res = (sint64_t)val1 + (sint64_t)val2;
	else if (val1.IsInteger() || val2.IsInteger()) res = (uint64_t)val1 + (uint64_t)val2;
	return res;
}

AValue operator - (const AValue& val1, const AValue& val2)
{
	AValue res;
	if		(val1.IsPointer() && val2.IsSigned())  res = (const void *)((uint64_t)val1 - (sint64_t)val2);
	else if	(val1.IsPointer())					   res = (const void *)((uint64_t)val1 - (uint64_t)val2);
	else if (val1.IsSigned()  && val2.IsPointer()) res = (const void *)((uint64_t)val1 - (sint64_t)val2);
	else if (					 val2.IsPointer()) res = (const void *)((uint64_t)val1 - (uint64_t)val2);
	else if	(val1.IsFloat()   || val2.IsFloat())   res = (double)val1   - (double)val2;
	else if (val1.IsSigned()  || val2.IsSigned())  res = (sint64_t)val1 - (sint64_t)val2;
	else if (val1.IsInteger() || val2.IsInteger()) res = (uint64_t)val1 - (uint64_t)val2;
	return res;
}
	
AValue operator * (const AValue& val1, const AValue& val2)
{
	AValue res;
	if		(val1.IsPointer())					   res = (const void *)((uint64_t)val1 * (uint64_t)val2);
	else if (					 val2.IsPointer()) res = (const void *)((uint64_t)val1 * (uint64_t)val2);
	else if	(val1.IsFloat()   || val2.IsFloat())   res = (double)val1   * (double)val2;
	else if (val1.IsSigned()  || val2.IsSigned())  res = (sint64_t)val1 * (sint64_t)val2;
	else if (val1.IsInteger() || val2.IsInteger()) res = (uint64_t)val1 * (uint64_t)val2;
	return res;
}
	
AValue operator / (const AValue& val1, const AValue& val2)
{
	AValue res;
	if		(val1.IsPointer())					   res = (const void *)((uint64_t)val1 / (uint64_t)val2);
	else if (					 val2.IsPointer()) res = (const void *)((uint64_t)val1 / (uint64_t)val2);
	else if	(val1.IsFloat()   || val2.IsFloat())   res = (double)val1   / (double)val2;
	else if (val1.IsSigned()  || val2.IsSigned())  res = (sint64_t)val1 / (sint64_t)val2;
	else if (val1.IsInteger() || val2.IsInteger()) res = (uint64_t)val1 / (uint64_t)val2;
	return res;
}

AValue operator % (const AValue& val1, const AValue& val2)
{
	AValue res;
	if		(val1.IsPointer())					   res = (const void *)((uint64_t)val1 % (uint64_t)val2);
	else if (					 val2.IsPointer()) res = (const void *)((uint64_t)val1 % (uint64_t)val2);
	else if	(val1.IsFloat()   || val2.IsFloat())   res = fmod((double)val1, (double)val2);
	else if (val1.IsSigned()  || val2.IsSigned())  res = (sint64_t)val1 % (sint64_t)val2;
	else if (val1.IsInteger() || val2.IsInteger()) res = (uint64_t)val1 % (uint64_t)val2;
	return res;
}

AValue operator ^ (const AValue& val1, const AValue& val2)
{
	return AValue((uint64_t)val1 ^ (uint64_t)val2);
}

AValue operator & (const AValue& val1, const AValue& val2)
{
	return AValue((uint64_t)val1 & (uint64_t)val2);
}

AValue operator && (const AValue& val1, const AValue& val2)
{
	return AValue((uint64_t)val1 && (uint64_t)val2);
}

AValue operator | (const AValue& val1, const AValue& val2)
{
	return AValue((uint64_t)val1 | (uint64_t)val2);
}

AValue operator || (const AValue& val1, const AValue& val2)
{
	return AValue((uint64_t)val1 || (uint64_t)val2);
}

AValue operator << (const AValue& val1, const AValue& val2)
{
	AValue res;
	if		(val1.IsFloat()  || val2.IsFloat())  res = (double)val1 * pow(2.0, (double)val2);
	else if (val1.IsSigned() || val2.IsSigned()) res = (sint64_t)val1 << (sint64_t)val2;
	else										 res = (uint64_t)val1 << (uint64_t)val2;
	return res;
}

AValue operator >> (const AValue& val1, const AValue& val2)
{
	AValue res;
	if		(val1.IsFloat()  || val2.IsFloat())  res = (double)val1 * pow(2.0, -(double)val2);
	else if (val1.IsSigned() || val2.IsSigned()) res = (sint64_t)val1 >> (sint64_t)val2;
	else										 res = (uint64_t)val1 >> (uint64_t)val2;
	return res;
}

bool operator == (const AValue& val1, const AValue& val2)
{
	bool res;
	if		(val1.IsFloat()  || val2.IsFloat())  res = ((double)val1   == (double)val2);
	else if (val1.IsSigned() || val2.IsSigned()) res = ((sint64_t)val1 == (sint64_t)val2);
	else										 res = ((uint64_t)val1 == (uint64_t)val2);
	return res;
}

bool operator < (const AValue& val1, const AValue& val2)
{
	bool res;
	if		(val1.IsFloat()  || val2.IsFloat())  res = ((double)val1   <  (double)val2);
	else if (val1.IsSigned() || val2.IsSigned()) res = ((sint64_t)val1 <  (sint64_t)val2);
	else										 res = ((uint64_t)val1 <  (uint64_t)val2);
	return res;
}

bool operator <= (const AValue& val1, const AValue& val2)
{
	bool res;
	if		(val1.IsFloat()  || val2.IsFloat())  res = ((double)val1   <= (double)val2);
	else if (val1.IsSigned() || val2.IsSigned()) res = ((sint64_t)val1 <= (sint64_t)val2);
	else										 res = ((uint64_t)val1 <= (uint64_t)val2);
	return res;
}

bool operator > (const AValue& val1, const AValue& val2)
{
	bool res;
	if		(val1.IsFloat()  || val2.IsFloat())  res = ((double)val1   >  (double)val2);
	else if (val1.IsSigned() || val2.IsSigned()) res = ((sint64_t)val1 >  (sint64_t)val2);
	else										 res = ((uint64_t)val1 >  (uint64_t)val2);
	return res;
}

bool operator >= (const AValue& val1, const AValue& val2)
{
	bool res;
	if		(val1.IsFloat()  || val2.IsFloat())  res = ((double)val1   >= (double)val2);
	else if (val1.IsSigned() || val2.IsSigned()) res = ((sint64_t)val1 >= (sint64_t)val2);
	else										 res = ((uint64_t)val1 >= (uint64_t)val2);
	return res;
}

AValue& AValue::operator += (const AValue& val)
{
	*this = *this + val;
	return *this;
}

AValue& AValue::operator -= (const AValue& val)
{
	*this = *this - val;
	return *this;
}
	
AValue& AValue::operator *= (const AValue& val)
{
	*this = *this * val;
	return *this;
}
	
AValue& AValue::operator /= (const AValue& val)
{
	*this = *this / val;
	return *this;
}

AValue& AValue::operator %= (const AValue& val)
{
	*this = *this % val;
	return *this;
}

AValue& AValue::operator ^= (const AValue& val)
{
	*this = *this ^ val;
	return *this;
}

AValue& AValue::operator &= (const AValue& val)
{
	*this = *this & val;
	return *this;
}

AValue& AValue::operator |= (const AValue& val)
{
	*this = *this | val;
	return *this;
}

AValue& AValue::operator <<= (const AValue& val)
{
	*this = *this << val;
	return *this;
}

AValue& AValue::operator >>= (const AValue& val)
{
	*this = *this >> val;
	return *this;
}

AValue AValue::operator - () const
{
	AValue res;
	if (IsValid()) {
		if		(IsFloat()) res = -(double)*this;
		else				res = -(sint64_t)*this;
	}
	return res;
}

AValue AValue::operator ~ () const
{
	AValue res;
	if (IsValid()) {
		if	 (IsFloat()) res = (double)(~(uint64_t)*this);
		else 			 res = ~(uint64_t)*this;
	}
	return res;
}

AValue AValue::operator ! () const
{
	AValue res;
	if (IsValid()) {
		if	 (IsFloat()) res = (double)(!(uint64_t)*this);
		else 			 res = !(uint64_t)*this;
	}
	return res;
}
