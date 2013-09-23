/** Written by Tim Cooper from Edval:  tim@edval.com.au  */

#include "DynamicType.h"
#include "header.h"
#include <string>



extern void snazzyFree(void *ptr);
extern char* snazzyStrdup(const char* s);
extern void* snazzyMalloc(size_t n);
extern void* snazzyRealloc(void* ptr, size_t size);

interface const char* UniqueString(const char* s);

unsigned char (*Dynamic::utf8ToCodePageConverter)(const unsigned char* &s);



//---------------------------------- Misc -------------------------------

static bool IsDigit(int ch)
{
    return (ch >= '0' and ch <= '9');
}



//---------------------------------- base64.h: -------------------------------
/* 
   base64.cpp and base64.h

   Copyright (C) 2004-2008 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

#include <iostream>

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;

}

std::string base64_decode(std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}


/*---------------------------------------------------------------------*/

void stringToJson(const char* s, std::ostringstream &ostr)
{
	ostr << '"';
	for ( ; *s; s++) {
		if (*s == '\n')
			ostr << "\\n";
		else if (*s == '\t')
			ostr << "\\t";
		else if (*s == '"')
			ostr << "\\\"";
		else if (*s == '\\')
			ostr << "\\\\";
		else ostr << *s;
	}
	ostr << '"';
}


static char* parseJsonString(const char* &sr)
{	const char *s=sr;
	char *buf;
	
	buf = (char*)snazzyMalloc(50);
	int idx = 0;
	if (*s == '"')
		s++;
	while (*s != '"') {
		if (*s == '\0')
			break;
		char ch;
		if (*s == '\\') {
			s++;
			if (*s == 'n')
				ch = '\n';
			else if (*s == 't')
				ch = '\t';
			else if (*s == '\0')
				break;
			else ch = *s;
			s++;
		}
		else ch = *s++;
		buf = (char*)snazzyRealloc(buf, idx + 2);//snazzyHeap does its own exponential reserving
		buf[idx++] = ch;
	}
	buf[idx] = '\0';
	if (*s == '"')
		s++;
	sr = s;
	return buf;
}


void Dynamic::StringImplementation::toJson(std::ostringstream &ostr) const 
{
	stringToJson(s, ostr);
}


void Dynamic::FloatImplementation::toJson(std::ostringstream &ostr) const 
{
	ostr.precision(6);
	ostr << f; 
}


void Dynamic::TimestampImplementation::toJson(std::ostringstream &ostr) const 
{	char buf[30];

	ostr << toIso8601(buf, sizeof(buf));
}


void Dynamic::BinaryDataImplementation::toJson(std::ostringstream &ostr) const
{
	ostr << "\"" << base64_encode(data, len) << "\"";
}


void Dynamic::ArrayImplementation::toJson(std::ostringstream &ostr) const
{
	ostr << '[';
	int limit;
	limit = int(_size);
	for (int i=0; i < limit; ++i) {
		if (i > 0)
			ostr << ',';
		A[i].value->toJson(ostr);
	}
	ostr << "]\n";
}


void Dynamic::StructImplementation::toJson(std::ostringstream &ostr) const
{
	ostr << "{";
	for (int i=0; i < size; i++) {
		if (i > 0)
			ostr << ',';
		stringToJson(A[i].name, ostr);
		ostr << ':';
		A[i].value.value->toJson(ostr);
	}
	ostr << "}\n";
}


void Dynamic::fromJson(kstr& s)
{	char tmp[512];
	Dynamic val;
	kstr s0;
	int i;

	while (isspace(*s))
		s++;
	switch (*s) {
		case 'n':	if (strbegins(s, "null")) {
						s += 4;
						value = new NullImplementation();
					}
					break;

		case 't':	if (strbegins(s, "true")) {
						s += 4;
						value = new BoolImplementation(true);
					}
					break;

		case 'f':	if (strbegins(s, "false")) {
						s += 5;
						value = new BoolImplementation(false);
					}
					break;

		case '"':	value = new StringImplementation(parseJsonString(s));
					break;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		case '-':
					i = 0;
					s0 = s;
					bool negative;
					negative = (*s == '-');
					if (negative)
						s++;
					while (IsDigit(*s))
						i = i * 10 + (*s++ - '0');
					if (*s == '.' or *s == 'e' or *s == 'E') {
						s = s0;
						char* d = tmp;
						while (IsDigit(*s) or *s == '-' or *s == 'e' or *s == 'E' or *s == '.') {
							*d++ = *s++;
							if (d + 1 >= tmp + sizeof(tmp))
								break;
						}
						*d = '\0';
						value = new FloatImplementation(atof(tmp));
					}
					else {
						value = new IntImplementation(negative ? -i : i);
					}
					break;

		case '[':	ArrayImplementation *array;
					value = array = new ArrayImplementation();
					s++;
					i = 0;
					while (isspace(*s))
						s++;
					do {
						if (*s == '\0')
							break;
						if (*s == ']') {
							s++;
							break;
						}
						array->resize(i+1);
						array->at(i++).fromJson(s);
						while (isspace(*s))
							s++;
						if (*s == ',')
							s++;
						else if (*s != ']')
							throw RestException("Unexpected char after json array element");
					} forever;
					break;

		case '{':	StructImplementation *record;
					value = record = new StructImplementation();
					s++;
					i = 0;
					do {
						while (isspace(*s))
							s++;
						if (*s == '\0')
							break;
						if (*s == '}') {
							s++;
							break;
						}
						str fieldname = parseJsonString(s);
						while (isspace(*s))
							s++;
						if (*s == ':')
							s++;
						else throw RestException("Missing ':' in string");
						record->field(UniqueString(fieldname)).fromJson(s);
						snazzyFree(fieldname);
						while (isspace(*s))
							s++;
						if (*s == ',')
							s++;
						else if (*s == '}') {
							s++;
							break;
						}
						else throw RestException("Unexpected char after json struct member");
					} forever;
					break;

		default:	throw RestException(std::string("Unexpected char in json data: ") + *s);
	}
}


Dynamic& Dynamic::StructImplementation::field(const char *s)
{
	for (int i=0; i < size; i++) {
		if (stricmp(A[i].name, s) == 0)
			return A[i].value;
	}
	if (size + 1 > allocated) {
		if (allocated == 0)
			allocated = 5;
		else allocated *= 2;
		A = (StructImplementationField*)realloc(A, allocated*sizeof(A[0]));
	}
	A[size].name = UniqueString(s);
	A[size].value.value = NULL;
	return A[size++].value;
}


StructImplementationField* Dynamic::StructImplementation::findField(const char *s)
// Returns NULL if the field doesn't exist.
{
	for (int i=0; i < size; i++) {
		if (stricmp(A[i].name, s) == 0)
			return &A[i];
	}
	return NULL;
}


Dynamic::StructImplementation::~StructImplementation()
{
	for (int i=0; i < size; i++)
		delete A[i].value.value;
	free(A);
}


Dynamic::ArrayImplementation::ArrayImplementation(ArrayImplementation &other)
{
	allocated = other.allocated;
	_size = other._size;
	A = (Dynamic*)malloc(allocated*sizeof(A[0]));
	for (int i=0; i < _size; i++) {
		A[i].value = other.A[i].value->cloneMe();
	}
	elementXmlName = other.elementXmlName;
}


Dynamic& Dynamic::ArrayImplementation::at(int idx)
{
	if (idx >= _size) {
		if (idx <= allocated) 
			resize(idx+1);
		for (int i=_size; i <= idx; i++)
			A[i].value = NULL;
		_size = idx + 1;
	}
	return A[idx]; 
}


void Dynamic::ArrayImplementation::resize(int n)
{
	allocated = n;
	A = (Dynamic*)realloc(A, allocated*sizeof(A[0]));
}


Dynamic::ArrayImplementation::~ArrayImplementation()
{
	for (int i=0; i < _size; i++)
		delete A[i].value;
	free(A);
}


Dynamic::StructImplementation::StructImplementation(StructImplementation &other)
{
	allocated = other.allocated;
	size = other.size;
	A = (StructImplementationField*)malloc(allocated*sizeof(A[0]));
	for (int i=0; i < size; i++) {
		A[i].name = other.A[i].name;	// It's an interned string.
		A[i].value.value = NULL;
		A[i].value = other.A[i].value;
	}
	xmlName = other.xmlName;
}


Dynamic Dynamic::Blob(void *data, int len)
{
	return Dynamic(new BinaryDataImplementation(data, len));
}


bool Dynamic::operator==(Dynamic& other)
{
	return *value == *other.value;
}


bool Dynamic::IntImplementation::operator==(Implementation& otherI) const
{
	return i == otherI.asInt();
}


bool Dynamic::FloatImplementation::operator==(Implementation& otherI) const
{
	return f == otherI.asFloat();
}


bool Dynamic::NullImplementation::operator==(Implementation& otherI) const
{
	return otherI.isNull();
}


bool Dynamic::StringImplementation::operator==(Implementation& otherI) const
{
	if (otherI.isString()) {
		return stricmp(s, (char*)otherI) == 0;
	}
	else if (otherI.isInt()) {
		char tmp[20];
		itoa((int)otherI, tmp, 10);
		return stricmp(s, tmp) == 0;
	}
	else if (otherI.isBool()) {
		bool value = (bool)otherI;
		return stricmp(s, value?"true":"false");
	}
	else if (otherI.isFloat()) {
		return atof(s) == (double)otherI;
	}
	else return no;
}


bool Dynamic::BoolImplementation::operator==(Implementation& otherI) const
{
	if (not otherI.isBool())
		return no;
	BoolImplementation *other = (BoolImplementation*)&otherI;
	return b == other->b;
}


bool Dynamic::ArrayImplementation::operator==(Implementation& otherI) const
{
	if (not otherI.isArray())
		return no;
	ArrayImplementation *other = (ArrayImplementation*)&otherI;
	if (_size != other->_size)
		return no;
	for (int i=0; i < _size; i++)
		if (A[i] != other->A[i])
			return no;
	return yes;
}


bool Dynamic::StructImplementation::operator==(Implementation& otherI) const
{
	if (not otherI.isStruct())
		return no;
	StructImplementation *other = (StructImplementation*)&otherI;
	if (size != other->size)
		return no;
	for (int i=0; i < size; i++) {
		StructImplementationField *field = other->findField(A[i].name);
		if (field == NULL)
			return no;
		if (A[i].value != field->value)
			return no;
	}
	return yes;
}


bool Dynamic::TimestampImplementation::operator==(Implementation& otherI) const
{
	return asTimestamp() == otherI.asTimestamp();
}


bool Dynamic::BinaryDataImplementation::operator==(Implementation& otherI) const
{
	if (not otherI.isBinary())
		return no;
	BinaryDataImplementation *other = (BinaryDataImplementation*)&otherI;
	if (this->len != other->len)
		return no;
	return memcmp(data, other->data, len) == 0;
}


Dynamic::Implementation* Dynamic::StructImplementation::DifferencesFrom(Implementation *_other)
{
	if (_other == NULL or not _other->isStruct())
		return cloneMe();
	StructImplementation *other = (StructImplementation*)_other;
	StructImplementation *dif = new StructImplementation();
	dif->xmlName = xmlName;
	for (int i=0; i < size; i++) {
		StructImplementationField *field = other->findField(A[i].name);
		if (field == NULL or field->value != A[i].value) {
			if (A[i].value.value != NULL)
				dif->field(A[i].name).value = A[i].value.value->cloneMe();
		}
	}
	return dif;
}


void Dynamic::ArrayImplementation::sort(compar_dynamic_fn cmp)
{
	qsort(A, size(), sizeof(A[0]), (cmp_fn)cmp);
}


long Dynamic::TimestampImplementation::asTimestamp() const
{
	return mktime((struct tm*)&timestamp);
}


int Dynamic::StringImplementation::asInt() const
{
	return atoi(s);
}


double Dynamic::StringImplementation::asFloat() const
{
	return atof(s);
}


long Dynamic::StringImplementation::asTimestamp() const
{
	TimestampImplementation ts;
	ts.fromIso8601(s);
	return ts.asTimestamp();
}


bool Dynamic::StringImplementation::asBool() const
{
	if (*s == 'T' or *s == 't' or *s == 'Y' or *s == 'y' or *s == '1')
		return true;
	return false;
}


const char* Dynamic::IntImplementation::asString(char dest[]) const
{
	sprintf(dest, "%d", i);
	return dest;
}


const char* Dynamic::FloatImplementation::asString(char dest[]) const
{
	sprintf(dest, "%1.6f", f);
	return dest;
}


static void SkipWhiteSpace(kstr &s)
{
	while (*s == ' ' or *s == '\t' or *s == '\n' or *s == '\r')
		s++;
}



/*------------------------------- XmlRpc (yech!) ----------------------------*/

// Map something like:    "T2-D&amp;T1"  to  "T2-D&T1"
// Returns a 'strdup()' version of the input string.

static str xmlDecode(kstr s, kstr end)
{
	str dest = (char*)malloc(end - s + 1);
	str d = dest;
	while (s < end) {
		if (*s != '&') {
			if ((*s & 0xc0) == 0xc0 and Dynamic::utf8ToCodePageConverter != NULL) {
				kstr before = s;
				*d++ = Dynamic::utf8ToCodePageConverter((const unsigned char*&)s);
				if (s == before)
					s++;//assert(false);
			}
			else *d++ = *s++;
		}
		else if (strbegins(s, "&amp;", true))
			*d++ = '&', s += 5;
		else if (strbegins(s, "&quot;", true))
			*d++ = '\"', s += 6;
		else if (strbegins(s, "&apos;", true)/*not standard*/ or strbegins(s, "&#039;", true))
			*d++ = '\'', s += 6;
		else if (strbegins(s, "&lt;", true))
			*d++ = '<', s += 4;
		else if (strbegins(s, "&gt;", true))
			*d++ = '>', s += 4;
		else if (strbegins(s, "&#", true)) {
			s += 2;
			*d++ = atoi(s);
			while (*s >= '0' and *s <= '9')
				s++;
			if (*s == ';')
				s++;
		}
		else
			*d++ = *s++;	// assert(false);
	}
	*d = '\0';
	return dest;
}


// Replace raw text with xml-encoded entities.

static std::string xmlEncode(kstr s)
{
	std::ostringstream ostr;

	while (*s) {
		if (*s == '&')
			ostr << "&amp;";
		else if (*s == '<')
			ostr << "&lt;";
		else if (*s == '>')
			ostr << "&gt;";
		else if (*s == '"')
			ostr << "&quot;";
		else if (*s == '\'')
			ostr << "&apos;";// Would David prefer:  "&#039;" ?
		else if (*s < ' ' or *s >= 127)
			ostr << "&#" << int((unsigned char)*s) << ';';
		else ostr << *s;
		s++;
	}
	return ostr.str();
}


static char* GobbleTag(kstr &s, char dest[128])
// E.g. given:  "< string  / >",  return "<string/>"
{
	*dest = '\0';
	SkipWhiteSpace(s);
	if (*s != '<')
		return dest;
	str d = dest;
	*d++ = *s++;
	SkipWhiteSpace(s);
	while (*s and *s != '>') {
		if (*s == ' ' and (d[-1] == '<' or d[-1] == '/' or s[1] == ' ' or s[1] == '/' or s[1] == '>'))
			s++;
		else *d++ = *s++;
	}
	*d++ = '>';
	*d = '\0';
	if (*s == '>')
		s++;
	return dest;
}


static char* GobbleTag(kstr &s, char dest[128], kstr *attributes)
/* This version is used for plain XML encoding.  E.g. given:  "<string j="5">",  
   return "<string>" and return with '*attributes' pointing to the 'j'. */
{
	*dest = '\0';
	*attributes = NULL;
	SkipWhiteSpace(s);
	if (*s != '<')
		return dest;
	str d = dest;
	*d++ = *s++;
	SkipWhiteSpace(s);
	while (*s and *s != '>') {
		if (*s == ' ') {
			if (d[-1] == '<' or d[-1] == '/' or s[1] == ' ' or s[1] == '/' or s[1] == '>')
				s++;	// These cases we skip
			else {
				while (*s == ' ' and *s)
					s++;
				*attributes = s;
				kstr t = strchr(s, '>');
				if (t == NULL) {
					s += strlen(s);
					break;
				}
				s = t;
				while (t > *attributes and t[-1] == ' ')
					t--;
				if (*t == '/')
					*d++ = '/';
				break;
			}
		}
		else *d++ = *s++;
	}
	*d++ = '>';
	*d = '\0';
	if (*s == '>')
		s++;
	return dest;
}


static void GobbleExpectedTag(kstr &s, kstr etag)
{	char tag[128];

	GobbleTag(s, tag);
	if (not strieq(tag, etag))
		throw RestException(std::string("Expecting tag: ") + etag);
}


static void GobbleExpectedEndTag(kstr &s, kstr tag)
{	char tmp[128];

	GobbleTag(s, tmp);
	if (tmp[1] != '/' or not strieq(tmp+2, tag+1))
		throw RestException(std::string("Expecting tag: </") + (tag+1));
}


void Dynamic::setXmlName(const char* xmlName) 
{ 
	if (value == NULL) 
		value = new StructImplementation;
	if (value->isStruct())
		((StructImplementation*)value)->xmlName = UniqueString(xmlName);
}				


void Dynamic::asArray(const char* elementXmlName)
{ 
	if (value == NULL) 
		value = new ArrayImplementation;
	if (value->isArray())
		((ArrayImplementation*)value)->elementXmlName = UniqueString(elementXmlName);
}				


void Dynamic::StructImplementation::fromXmlRpc(kstr &s)
{
	SkipWhiteSpace(s);
	while (strbegins(s, "<member>", true)) {
		s += strlen("<member>");

		// name
		GobbleExpectedTag(s, "<name>");
		kstr nameEnd = strchr(s, '<');
		if (nameEnd == NULL)
			throw RestException("Bad 'name' tag in struct");
		str name = xmlDecode(s, nameEnd);
		s = nameEnd;
		GobbleExpectedTag(s, "</name>");

		// value
		field(name).fromXmlRpc(s);
		free(name);

		GobbleExpectedTag(s, "</member>");
		SkipWhiteSpace(s);
	}
}


void Dynamic::ArrayImplementation::fromXmlRpc(kstr &s)
{	char tag[128];

	GobbleTag(s, tag);
	if (strieq(tag, "<data/>"))
		return;
	if (not strieq(tag, "<data>"))
		throw RestException("Expecting tag:  <data>");

	do {
		SkipWhiteSpace(s);
		if (strbegins(s, "</data>", true))
			break;
		next().fromXmlRpc(s);
	} while (true);

	GobbleExpectedTag(s, "</data>");
}


void Dynamic::StringImplementation::fromXmlRpc(kstr &s)
{
	kstr valueEnd = strchr(s, '<');
	if (valueEnd == NULL)
		throw RestException("Bad string");
	this->s = xmlDecode(s, valueEnd);
	s = valueEnd;
}


void Dynamic::NullImplementation::fromXmlRpc(kstr &s)
{
}


void Dynamic::BoolImplementation::fromXmlRpc(kstr &s)
{
	char ch = *s++;
	if (ch == '0')
		b = false;
	else if (ch == '1')
		b = true;
	else throw RestException("Bad bool value");
}


void Dynamic::IntImplementation::fromXmlRpc(kstr &s)
{
	char* valueEnd;
	i = (int)strtol(s, &valueEnd, 10);
	if (valueEnd == s)
		throw RestException("Bad int");
	s = valueEnd;
}


void Dynamic::FloatImplementation::fromXmlRpc(kstr &s)
{
	char* valueEnd;
	f = strtod(s, &valueEnd);
	if (valueEnd == s)
		throw RestException("Bad double");
	s = valueEnd;
}


void Dynamic::NullImplementation::toXmlRpc(std::ostringstream &ostr) const
{
	ostr << "<value><nil/></value>";
}


void Dynamic::StringImplementation::toXmlRpc(std::ostringstream &ostr) const
{
	if (s == NULL or *s == '\0')
		ostr << "<value><string/></value>";
	else ostr << "<value>" << xmlEncode(s) << "</value>";
	// The 'STRING_TAG' is optional.
}


void Dynamic::BoolImplementation::toXmlRpc(std::ostringstream &ostr) const
{
	ostr << "<value><boolean>" << (b?"1":"0") << "</boolean></value>";
}


void Dynamic::IntImplementation::toXmlRpc(std::ostringstream &ostr) const
{
	ostr << "<value><i4>" << i << "</i4></value>";
}


void Dynamic::FloatImplementation::toXmlRpc(std::ostringstream &ostr) const
{
	ostr << "<value><double>" << f << "</double></value>";
	// This will use the default ostream::precision() to display the double.  To display
	// values with greater accuracy, call e.g.  'ostr.precision(12)' at the top level.
}


void Dynamic::TimestampImplementation::fromXml(kstr &s, char tag[])
{
	kstr valueEnd = strchr(s, '<');
	if (valueEnd == NULL)
		throw RestException("Bad time value");
	fromIso8601(s);
	timestamp.tm_isdst = -1;
	timestamp.tm_mon -= 1;
	s = valueEnd;
}


void Dynamic::TimestampImplementation::fromXmlRpc(kstr &s)
{
	kstr valueEnd = strchr(s, '<');
	if (valueEnd == NULL)
		throw RestException("Bad time value");
	fromIso8601(s);
	timestamp.tm_isdst = -1;
	timestamp.tm_mon -= 1;
	s = valueEnd;
}


void Dynamic::StructImplementation::toXmlRpc(std::ostringstream &ostr) const
{
	ostr << "<value><struct>";
	for (int i=0; i < size; i++) {
		ostr << "<member><name>" << xmlEncode(A[i].name) << "</name>";
		A[i].value.toXmlRpc(ostr);
		ostr << "</member>";
	}
	ostr << "</struct></value>\n";
}


void Dynamic::ArrayImplementation::toXmlRpc(std::ostringstream &ostr) const
{
	ostr << "<value><array><data>";
	for (int i=0; i < _size; ++i)
		 A[i].toXmlRpc(ostr);
	ostr << "</data></array></value>";
}


char* Dynamic::TimestampImplementation::toIso8601(char dest[], int sizeofdest) const
{
	_snprintf_s(dest, sizeofdest, sizeofdest-1, 
				"%4d%02d%02dT%02d:%02d:%02d", 
				timestamp.tm_year,
				timestamp.tm_mon+1,
				timestamp.tm_mday,
				timestamp.tm_hour,
				timestamp.tm_min,
				timestamp.tm_sec);
	return dest;
}


void Dynamic::TimestampImplementation::fromIso8601(kstr s) const
{
	if (sscanf_s(s, "%4d%2d%2dT%2d:%2d:%2d", 
				&timestamp.tm_year,
				&timestamp.tm_mon,
				&timestamp.tm_mday,
				&timestamp.tm_hour,
				&timestamp.tm_min,
				&timestamp.tm_sec) != 6)
		throw RestException("Bad time value");
}


void Dynamic::TimestampImplementation::toXmlRpc(std::ostringstream &ostr) const
{	char buf[30];

	ostr << "<value><dateTime.iso8601>" << toIso8601(buf, sizeof(buf)) << "</dateTime.iso8601></value>";
}


void Dynamic::BinaryDataImplementation::fromXmlRpc(kstr &s)
{
	kstr valueEnd = strchr(s, '<');
	if (valueEnd == NULL)
		throw RestException("Bad base64");
	std::string base64 = std::string(s, valueEnd-s);
	std::string binary = base64_decode(base64);
	len = binary.length();
	data = (unsigned char*)malloc(len);
	memcpy(data, binary.c_str(), len);
	s = valueEnd;
}


void Dynamic::BinaryDataImplementation::toXmlRpc(std::ostringstream &ostr) const
{
	ostr << "<value><base64>" << base64_encode(data, len) << "</base64></value>";
}


void Dynamic::fromXmlRpc(kstr& s)
{	char tag[128];

	if (value) {
		delete value;
		value = NULL;
	}

	// Gobble the <value> tag:
	GobbleTag(s, tag);
	if (strieq(tag, "<value>"))
		;	// good
	else if (strieq(tag, "<value/>")) {
		// Jeff Rasmussen claims that <value/> is valid XmlRpc. I think he's correct.
		value = new StringImplementation("");
		return;
	}
	else
		throw RestException(std::string("Expecting tag: <value>"));

	// Gobble the type tag:
	GobbleTag(s, tag);
	if (*tag == '\0') {
		// "If no type is indicated, the type is string."
		value = new StringImplementation(NULL);
		value->fromXmlRpc(s);
	}
	else if (strchr(tag, '/')) {
		// It's an empty something-or-other:
		if (strieq(tag, "<string/>"))
			value = new StringImplementation("");
		else if (strieq(tag, "<nil/>"))
			value = new NullImplementation;
		else if (strieq(tag, "<struct/>"))
			value = new StructImplementation;
		else if (strieq(tag, "</value>")) {	
			// "If no type is indicated, the type is string."
			value = new StringImplementation("");
			return;
			// don't gobble VALUE_ETAG because we already did
		}
	}
	else {
		if (strieq(tag, "<boolean>"))
			value = new BoolImplementation(false);
		else if (strieq(tag, "<i4>"))
			value = new IntImplementation(0);
		else if (strieq(tag, "<int>"))
			value = new IntImplementation(0);
		else if (strieq(tag, "<double>"))
			value = new FloatImplementation(0);
		else if (strieq(tag, "<string>"))
			value = new StringImplementation;
		else if (strieq(tag, "<dateTime.iso8601>"))
			value = new TimestampImplementation;
		else if (strieq(tag, "<base64>"))
			value = new BinaryDataImplementation;
		else if (strieq(tag, "<array>"))
			value = new ArrayImplementation;
		else if (strieq(tag, "<struct>"))
			value = new StructImplementation;
		else
			throw RestException(std::string("Unknown type tag: ") + tag);
		value->fromXmlRpc(s);

		// Gobble the closing type tag:
		GobbleExpectedEndTag(s, tag);
	}

	GobbleExpectedTag(s, "</value>");
}


void parseXmlRpcMethodResponse(kstr s, Dynamic &output)
{	char xmlVersion[128];
	char tag[128];

	GobbleTag(s, xmlVersion);
	if (! strbegins(xmlVersion, "<?xml version", false)) {
		if (strstr(s, "<html") != NULL or strstr(s, "<HTML") != NULL)
			throw RestException("It looks like you've configured the wrong URL. That URL is sending us HTML text, "
						"whereas we need RPC data.");
		else throw RestException(std::string(s));
	}
	GobbleTag(s, tag);
	if (not strieq(tag, "<methodResponse>")) {
		if (strstr(s, "<title>Bad request!</title>"))
			throw RestException(std::string("Talking HTTP to a HTTPS server?"));
		else if (strstr(s, "Object not found"))
			throw RestException(std::string("Wrong URL (\"Object not found\")"));
		else throw RestException(std::string(s));
	}
	SkipWhiteSpace(s);
	if (strbegins(s, "<fault>", true)) {
		GobbleExpectedTag(s, "<fault>");
		output.fromXmlRpc(s);
		GobbleExpectedTag(s, "</fault>");
		throw RestException(output.asString(tag));
	}
	else {
		GobbleExpectedTag(s, "<params>");
		GobbleExpectedTag(s, "<param>");
		output.fromXmlRpc(s);
		// There's no real need to parse the bits at the end, is there?
		//GobbleExpectedTag(s, "</param>");
		//GobbleExpectedTag(s, "</params>");
	}
	//GobbleExpectedTag(s, "</methodResponse>");
}



//------------------------------ Plain XML: ------------------------

void Dynamic::NullImplementation::toXml(std::ostringstream &ostr) const
{
	ostr << "<nil/>";
}


void Dynamic::StringImplementation::fromXml(kstr &s, char tag[])
{
	fromXmlRpc(s);
}


void Dynamic::BoolImplementation::fromXml(kstr &s, char tag[])
{
	if (*s == '0')
		b = false, s++;
	else if (*s == '1')
		b = true, s++;
	else if (strbegins(s, "true"))
		b = true, s += 4;
	else if (strbegins(s, "false"))
		b = true, s += 5;
	else throw RestException("Bad bool value");
}


void Dynamic::IntImplementation::fromXml(kstr &s, char tag[])
{
	fromXmlRpc(s);
}


void Dynamic::FloatImplementation::fromXml(kstr &s, char tag[])
{
	fromXmlRpc(s);
}


void Dynamic::TimestampImplementation::toXml(std::ostringstream &ostr) const
{	char buf[30];

	ostr << toIso8601(buf, sizeof(buf));
}


void Dynamic::StringImplementation::toXml(std::ostringstream &ostr) const
{
	if (s)
		ostr << xmlEncode(s);
}


void Dynamic::BinaryDataImplementation::toXml(std::ostringstream &ostr) const
{
	toXmlRpc(ostr);
}


void Dynamic::BoolImplementation::toXml(std::ostringstream &ostr) const
{
	ostr << (b?"TRUE":"FALSE");
}


void Dynamic::IntImplementation::toXml(std::ostringstream &ostr) const
{
	ostr << i;
}


void Dynamic::FloatImplementation::toXml(std::ostringstream &ostr) const
{
	ostr << f;
	// This will use the default ostream::precision() to display the double.  To display
	// values with greater accuracy, call e.g.  'ostr.precision(12)' at the top level.
}


void Dynamic::ArrayImplementation::toXml(std::ostringstream &ostr) const
{
	for (int i=0; i < _size; i++) {
		if (A[i].value->isStruct())
			A[i].value->toXml(ostr);	// It has its own tag.
		else {
			ostr << '<' << xmlEncode(elementXmlName) << '>';
			A[i].value->toXml(ostr);
			ostr << '<' << '/' << xmlEncode(elementXmlName) << '>' << '\n';
		}
	}
}


const char* StructImplementationField::xmlAttribute()
{
	if (value.isArray() or value.isStruct())
		return NULL;
	if (*name == '@')
		return name+1;
	return NULL;
}


void Dynamic::StructImplementation::toXml(std::ostringstream &ostr) const
{	char dest[512];

	ostr << '<' << xmlEncode(xmlName);
	for (int i=0; i < size; i++) {
		if (kstr attr=A[i].xmlAttribute()) {
			kstr val = A[i].value.value->asString(dest);
			char quote = strchr(val, '"') ? '\'' : '"';
			ostr << ' ' << attr << '=' << quote;
			ostr << xmlEncode(val);
			ostr << quote;
		}
	}
	ostr << '>';
	for (int i=0; i < size; i++) {
		if (not A[i].xmlAttribute()) {
			if (A[i].value.isStruct()) {
				A[i].value.toXml(ostr);		
				// assert A[i].name == A[i].value->xmlName
			}
			else {
				ostr << '<' << xmlEncode(A[i].name) << '>';
				A[i].value.toXml(ostr);
				ostr << '<' << '/' << xmlEncode(A[i].name) << '>';
			}
		}
	}
	ostr << '<' << '/' << xmlEncode(xmlName) << '>' << '\n';
}


void Dynamic::NullImplementation::fromXml(kstr &s, char tag[])
{
}


void Dynamic::BinaryDataImplementation::fromXml(kstr &s, char tag[])
{
	fromXmlRpc(s);
}


void Dynamic::StructImplementation::fromXml(kstr &s, char tag[])
{	char child[512];
	kstr attributes;

	GobbleTag(s, tag, &attributes);
	SkipWhiteSpace(s);
	str tagEnd = strchr(tag, '>');
	*tagEnd = '\0';
	xmlName = UniqueString(tag+1);
	*tagEnd = '>';

	// Parse the attributes:
	if (attributes)
		parseXmlAttributes(attributes);

	// Parse the elements:
	while (not strbegins(s, "</", true)) {
		kstr peek = s;
		GobbleTag(peek, child, &attributes);
		str close = strchr(child, '>');
		if (close == NULL)
			throw RestException(std::string("Missing '>' for tag ") + tag);
		*close = '\0';
		Dynamic& fld = field(child+1);
		if (fld.value) {
			// Whoah!  This is an array, not a struct!
			*tag = '\1';
			return;
		}
		fld.fromXml(s, child);
		SkipWhiteSpace(s);
	}
	GobbleExpectedEndTag(s, tag);
}


void Dynamic::ArrayImplementation::fromXml(kstr &s, char tag[])
{	char child[512];
	kstr attributes;

	GobbleTag(s, tag, &attributes);
	SkipWhiteSpace(s);
	elementXmlName = NULL;
	while (not strbegins(s, "</", true) and *s) {
		Dynamic& el=next();
		el.fromXml(s, child);
		if (elementXmlName == NULL) {
			str close = strchr(child, '>');
			*close = '\0';
			elementXmlName = UniqueString(child+1);
		}
		if (attributes) {
			if (el.isStruct())
				((StructImplementation*)el.value)->parseXmlAttributes(attributes);
		}
		SkipWhiteSpace(s);
	}
	GobbleExpectedEndTag(s, tag);
}


void Dynamic::StructImplementation::parseXmlAttributes(kstr attributes)
{	char val[512], tag[512];

	do {
		char* d=tag;
		*d++ = '@';
		SkipWhiteSpace(attributes);
		if (*attributes == '/' or *attributes == '>')
			break;
		while (*attributes and not strchr("=/> ", *attributes))
			*d++ = *attributes++;
		*d = '\0';
		SkipWhiteSpace(attributes);
		if (*attributes == '=') {
			attributes++;
			SkipWhiteSpace(attributes);
			char quote = *attributes;
			if (quote == '"' or quote == '\'')
				attributes++;
			d = val;
			while (*attributes and *attributes != quote)
				*d++ = *attributes++;
			*d = '\0';
			if (*attributes == quote)
				attributes++;

			// Populate the field:
			Dynamic& fld = field(tag);
			fld.value = new StringImplementation(val);
		}
	} while (true);
}


void Dynamic::fromXml(kstr& s, char tag[])
{	kstr attributes;

	kstr peek = s;
	GobbleTag(peek, tag, &attributes);
	if (tag[0] == '\0')
		throw RestException("The returned text doesn't look like it's XML.");
	SkipWhiteSpace(peek);
	if (*peek == '<' and peek[1] != '/') {
		value = new StructImplementation;
		kstr rewind = s;
		value->fromXml(s, tag);
		if (attributes)
			((StructImplementation*)value)->parseXmlAttributes(attributes);
		if (tag[0] == '\1') {
			delete value;
			*tag = '<';
			s = rewind;
			value = new ArrayImplementation;
			value->fromXml(s, tag);
		}
	}
	else {
		kstr end = strchr(peek, '<');
		if (end == NULL)
			s = peek + strlen(peek);
		else {
			value = new StringImplementation(xmlDecode(peek, end), true);
			s = end;
			GobbleExpectedEndTag(s, tag);
			if (attributes) {
				str close = strchr(tag, '>');
				*close = '\0';
				StructImplementation *outer = new StructImplementation(tag+1);
				outer->field(tag+1).value = value;
				outer->parseXmlAttributes(attributes);
				value = outer;
				*close = '>';
			}
		}
	}
}



//----------------------------------- Unit test: --------------------------------

Dynamic record;


static void error()
{
	printf("Error\n");
}


static void testJson()
{
	std::ostringstream ostr;
	record.toJson(ostr);
	std::string json = ostr.str().c_str();
	kstr s = json.c_str();
	Dynamic record2;
	record2.fromJson(s);
	if (record != record2)
		error();
}


void testXmlRpc()
{
	std::ostringstream ostr;
	record.toXmlRpc(ostr);
	std::string xmlRpc = ostr.str();
	kstr s = xmlRpc.c_str();
	Dynamic record2;
	record2.fromXmlRpc(s);
	if (record != record2)
		error();
}


void testXml()
{	char tag[512];

	std::ostringstream ostr;
	record.toXml(ostr);
	std::string xml = ostr.str();
	kstr s = xml.c_str();
	Dynamic record2;
	record2.fromXml(s, tag);
	if (record != record2)
		error();
}


void testXmlRpcBase64()
{
	Dynamic data = Dynamic::Blob("ABWGQWRH", 7);
	std::ostringstream ostr5;
	data.toXmlRpc(ostr5);
	std::string xml5 = ostr5.str();
	Dynamic dataAgain;
	kstr xml5s = xml5.c_str();
	dataAgain.fromXmlRpc(xml5s);
	if (dataAgain != data)
		error();
}


void Test2()
{	char tmp[512];

	// Internal storage:
	record.asStruct();
	record["@j"] = 6;
	record["hello"] = "fine";
	record["limit"] = 1116.25;
	Dynamic servers = record["servers"];
	servers.asArray();
	servers.add(6);
	servers.add(33.3);
	servers.add("hi");
	record["servers"] = servers;
	int j = record["@j"];
	double f = record["servers"][1];
	const char* s = record["servers"][2].asString(tmp);
	printf("%s\n", s);
	if (record != record)
		error();
	if (record == servers)
		error();

	//
	testJson();
	testXmlRpc();
	testXml();
	testXmlRpcBase64();
}

