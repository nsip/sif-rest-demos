#ifndef DYNAMICTYPE_H
#define DYNAMICTYPE_H

#include <string.h>
#include <time.h>
#include <sstream>
#include <string>

#define NoNum           -2147483648.0


class RestException {
	char *message;

public:
	RestException(const char *_message) { message = strdup(_message); }
	RestException(std::string _message) { message = strdup(_message.c_str()); }
	const char* getMessage() const { return message; }
};


typedef char* string_t;

typedef int (*compar_dynamic_fn)(class Dynamic *Ap, class Dynamic *Bp);



class Dynamic {

	class Implementation {
	public:
		virtual void toJson(std::ostringstream &ostr) const = 0;
		virtual void fromXml(const char* &s, char tag[]) = 0;
		virtual void toXml(std::ostringstream &ostr) const = 0;
		virtual void fromXmlRpc(const char* &s) = 0;
		virtual void toXmlRpc(std::ostringstream &ostr) const = 0;
		virtual Dynamic& at(int i) { throw RestException("It's not an array"); }
		virtual Dynamic& field(const char *s) { throw RestException("It's not a struct"); }
		virtual void add(Dynamic &el) { throw RestException("It's not an array"); }
		virtual Implementation* cloneMe() = 0;
		virtual operator int() { throw RestException("It's not an int."); }
		virtual operator double() { throw RestException("It's not a float."); }
		virtual operator bool() { throw RestException("It's not a bool."); }
		virtual operator string_t() { throw RestException("It's not a string."); }
		virtual bool operator==(Implementation& other) const = 0;
		bool operator!=(Implementation& other) const { return ! (*this == other); }
		virtual bool isArray() const { return false; }
		virtual bool isStruct() const { return false; }
		virtual bool isBool() const { return false; }
		virtual bool isInt() const { return false; }
		virtual bool isFloat() const { return false; }
		virtual bool isBinary() const { return false; }
		virtual bool isTimestamp() const { return false; }
		virtual bool isString() const { return false; }
		virtual bool isNull() const { return false; }
		virtual bool asBool() const { return false; }
		virtual int asInt() const { return (int)NoNum; }
		virtual double asFloat() const { return NoNum; }
		virtual const char* asString(char dest[]) const { return NULL; }
		virtual long asTimestamp() const { return 0; }
		virtual int size() const { return 0; }
		virtual bool isEmpty() const { return false; }
		virtual Implementation* DifferencesFrom(Implementation *other) { return NULL; }
		virtual ~Implementation() { }
	};

	class NullImplementation : public Implementation {
	public:
		void toJson(std::ostringstream &ostr) const { ostr << "null"; }
		void fromXml(const char* &s, char tag[]);
		void toXml(std::ostringstream &ostr) const;
		void fromXmlRpc(const char* &s);
		void toXmlRpc(std::ostringstream &ostr) const;
		Implementation* cloneMe() { return new NullImplementation(); }
		bool operator==(Implementation& other) const;
		bool isNull() const { return true; }
	};

	class IntImplementation : public Implementation {
		int i;

	public:
		void toJson(std::ostringstream &ostr) const { ostr << i; }
		void fromXml(const char* &s, char tag[]);
		void toXml(std::ostringstream &ostr) const;
		void fromXmlRpc(const char* &s);
		void toXmlRpc(std::ostringstream &ostr) const;
		IntImplementation(int _i) { i = _i; }
		operator int() { return i; }
		Implementation* cloneMe() { return new IntImplementation(i); }
		bool operator==(Implementation& other) const;
		bool isInt() const { return true; }
		int asInt() const { return i; }
		const char* asString(char dest[]) const;
	};

	class BoolImplementation : public Implementation {
		bool b;

	public:
		void toJson(std::ostringstream &ostr) const { ostr << (b?"true":"false"); }
		void fromXml(const char* &s, char tag[]);
		void toXml(std::ostringstream &ostr) const;
		void fromXmlRpc(const char* &s);
		void toXmlRpc(std::ostringstream &ostr) const;
		BoolImplementation(int _b) { b = _b; }
		operator bool() { return b; }
		Implementation* cloneMe() { return new BoolImplementation(b); }
		bool operator==(Implementation& other) const;
		bool isBool() const { return true; }
		bool asBool() const { return b; }
		const char* asString(char dest[]) const { return b ? "Y":"N"; }
	};

	class FloatImplementation : public Implementation {
		double f;

	public:
		void toJson(std::ostringstream &ostr) const;
		void fromXml(const char* &s, char tag[]);
		void toXml(std::ostringstream &ostr) const;
		void fromXmlRpc(const char* &s);
		void toXmlRpc(std::ostringstream &ostr) const;
		FloatImplementation(double _f) { f = _f; }
		operator double() { return f; }
		Implementation* cloneMe() { return new FloatImplementation(f); }
		bool operator==(Implementation& other) const;
		bool isFloat() const { return true; }
		double asFloat() const { return f; }
		const char* asString(char dest[]) const;
	};

	class StringImplementation : public Implementation {
		char *s;

	public:
		void toJson(std::ostringstream &ostr) const;
		void fromXml(const char* &s, char tag[]);
		void toXml(std::ostringstream &ostr) const;
		void fromXmlRpc(const char* &s);
		void toXmlRpc(std::ostringstream &ostr) const;
		StringImplementation() { s = NULL; }
		StringImplementation(const char *_s) { s = strdup(_s); }
		StringImplementation(const char *_s, bool strdupdAlready) { if (strdupdAlready) s = (char*)_s; else s = strdup(_s); }
		operator string_t() { return s; }
		Implementation* cloneMe() { return new StringImplementation(s); }
		~StringImplementation() { free(s); }
		bool operator==(Implementation& other) const;
		bool isString() const { return true; }
		const char* asString(char dest[]) const { return s; }
		int asInt() const;
		double asFloat() const;
		long asTimestamp() const;
		bool asBool() const;
	};

	/** BinaryData has a standard representation in XmlRpc, but not JSON. 
	In JSON we represent it as base64-encoded strings. */
	class BinaryDataImplementation : public Implementation {
		unsigned char *data;
		unsigned int len;

	public:
		BinaryDataImplementation()
		{
			data = NULL;
			len = 0;
		}

		BinaryDataImplementation(void *_data, int _len)
		{
			len = _len;
			data = (unsigned char*)memcpy(malloc(_len), _data, _len);
		}

		void toJson(std::ostringstream &ostr) const;
		void fromXml(const char* &s, char tag[]);
		void toXml(std::ostringstream &ostr) const;
		void fromXmlRpc(const char* &s);
		void toXmlRpc(std::ostringstream &ostr) const;
		Implementation* cloneMe() { return new BinaryDataImplementation(data, len); }
		bool operator==(Implementation& other) const;
		bool isBinary() const { return true; }
	};

	/** Timestamps have a standard representation in XmlRpc, but not JSON. */
	class TimestampImplementation : public Implementation {
		struct tm timestamp;

	public:
		TimestampImplementation()
		{
		}

		TimestampImplementation(struct tm *_timestamp)
		{
			timestamp = *_timestamp;
		}

		char* toIso8601(char dest[], int sizeofdest) const;
		void fromIso8601(const char* s) const;
		void toJson(std::ostringstream &ostr) const;
		void fromXml(const char* &s, char tag[]);
		void toXml(std::ostringstream &ostr) const;
		void fromXmlRpc(const char* &s);
		void toXmlRpc(std::ostringstream &ostr) const;
		Implementation* cloneMe() { return new TimestampImplementation(&timestamp); }
		bool operator==(Implementation& other) const;
		bool isTimestamp() const { return true; }
		long asTimestamp() const;
	};

	class ArrayImplementation : public Implementation {
		// tco> I'm implementing my own 'ValueArray' instead of the original
		// std::vector<> because resizing the std::vector<> calls 100's of 
		// constructors and destructors.  Using 'vector::reserve()' is not sufficient
		// to prevent these constructors/destructors from being called, because
		// the C++ standard requires constructors and destructors be called whenever
		// an object changes its address, as happens when the std::vector resizes.
		int allocated;
		Dynamic *A;
		int _size;

	public:
		const char* elementXmlName;		// don't free() this

		ArrayImplementation() { A = NULL; _size = allocated = 0; elementXmlName = "el"; }
		ArrayImplementation(int n) { 
					A = NULL; _size = allocated = 0;
					elementXmlName = "el";
					resize(n); }
		ArrayImplementation(ArrayImplementation &other);
		Implementation* cloneMe() { return new ArrayImplementation(*this); }
		void resize(int n);
		Dynamic& at(int i);
		bool operator==(Implementation &other) const;
		void toXml(std::ostringstream &ostr) const;
		void toJson(std::ostringstream &ostr) const;
		void fromXmlRpc(const char* &s);
		void toXmlRpc(std::ostringstream &ostr) const;
		void fromXml(const char* &s, char tag[]);
		void add(Dynamic &val) { at(_size) = val; }
		void sort(compar_dynamic_fn cmp);
		Dynamic& next() { return at(_size); }
		bool isArray() const { return true; }
		int size() const { return _size; }
		bool isEmpty() const { return _size == 0; }
		~ArrayImplementation();
	};

	class StructImplementation : public Implementation {
		struct StructImplementationField *A;
		int size;
		int allocated;

	public:
		const char* xmlName;	// Don't free() this.

		StructImplementation() { A = NULL; size = allocated = 0; xmlName = "struct"; }
		StructImplementation(const char *_xmlName) { A = NULL; size = allocated = 0; xmlName = _xmlName; }
		StructImplementation(StructImplementation &other);
		Implementation* cloneMe() { return new StructImplementation(*this); }
		void resize(int n);
		Dynamic& field(const char *s);
		StructImplementationField* findField(const char *s);
		bool operator==(Implementation &other) const;
		void toXml(std::ostringstream &ostr) const;
		void toJson(std::ostringstream &ostr) const;
		void fromXml(const char* &s, char tag[]);
		void fromXmlRpc(const char* &s);
		void toXmlRpc(std::ostringstream &ostr) const;
		bool isStruct() const { return true; }
		bool isEmpty() const { return size == 0; }
		void parseXmlAttributes(const char* attributes);
		Implementation* DifferencesFrom(Implementation *other);
		~StructImplementation();
	};

	Implementation *value;

public:
	Dynamic() { value = NULL; }
	//Dynamic(Implementation* _value) { value = _value; }
	void fromJson(const char* &sr);
	void fromXmlRpc(const char* &sr);
	void fromXml(const char* &sr, char tag[]);
	void toJson(std::ostringstream &ostr) const { if (value) value->toJson(ostr); }
	void toXmlRpc(std::ostringstream &ostr) const { if (value) value->toXmlRpc(ostr); }
	void toXml(std::ostringstream &ostr) const { if (value) value->toXml(ostr); }
	void asStruct() { if (value) delete value; value = new StructImplementation(); }
	void asStruct(const char* xmlName) { if (value) delete value; value = new StructImplementation(xmlName); }
	void asArray() {	
						if (value == NULL) 
							value = new ArrayImplementation(); 
						else if (value->isArray())
							;
						else {
							delete value;
							value = new ArrayImplementation();
						}
					}
	void asArray(const char* elementXmlName);
	void setXmlName(const char* xmlName);
	static Dynamic Blob(void *data, int len);
	Dynamic& operator=(Dynamic const& other) { 
				if (value) delete value; 
				value = other.value->cloneMe(); 
				return *this; }
	Dynamic& operator=(int i) { delete value; value = new IntImplementation(i); return *this; }
	Dynamic& operator=(bool b) { delete value; value = new BoolImplementation(b); return *this; }
	Dynamic& operator=(double f) { delete value; value = new FloatImplementation(f); return *this; }
	Dynamic& operator=(const char* s) { delete value; value = new StringImplementation(s); return *this; }
	Dynamic& operator[](int i) { if (value == NULL || ! value->isArray()) {
									if (value)
										delete value;
									value = new ArrayImplementation;
								}	
								return value->at(i); 
							}
	Dynamic& operator[](const char *s) { 
							if (value == NULL) 
								value = new StructImplementation;
							return value->field(s); 
						}
	operator int() const { return value == NULL ? (int)NoNum : value->asInt(); }
	operator double() const { return value == NULL ? NoNum : value->asFloat(); }
	operator bool() const { return value == NULL ? false : value->asBool(); }
	operator string_t() const { if (value == NULL) return NULL;
								char tmp[512];
								const char* s = value->asString(tmp);
								return s == tmp ? NULL : (string_t)s; 
							}
	bool isArray() const { return value != NULL && value->isArray(); }
	bool isStruct() const { return value != NULL && value->isStruct(); }
	bool isNull() const { return value != NULL && value->isNull(); }
	bool isString() const { return value ? value->isString() : false; }
	bool isEmpty() const { return value ? value->isEmpty() : false; }
	int size() const { return value ? value->size() : 0; }
	bool hasMember(const char* field) { 
				if (value == NULL || ! value->isStruct()) 
					return false;
				else return ((StructImplementation*)value)->findField(field) != NULL; }
	const char* asString(char dest[]) { return value->asString(dest); }
	long asTimestamp() { return value->asTimestamp(); }
	void add(Dynamic e) { return value->add(e); }
	Dynamic& next() { asArray(); return ((ArrayImplementation*)value)->next(); }
	Dynamic(int i) { value = new IntImplementation(i); }
	Dynamic(double f) { value = new FloatImplementation(f); }
	Dynamic(bool b) { value = new BoolImplementation(b); }
	Dynamic(const char* s) { value = new StringImplementation(s); }
	Dynamic(struct tm *timestamp) { value = new TimestampImplementation(timestamp); }
	bool operator==(Dynamic& other);
	bool operator!=(Dynamic& other) {	if (value == NULL || other.value == NULL)
											return value == other.value; 
										else return ! (*value == *other.value); }
	void DifferencesFrom(Dynamic &other, Dynamic &differences) { differences.value = value->DifferencesFrom(other.value); }
	void sort(compar_dynamic_fn fn) { if (! value->isArray()) return; ((ArrayImplementation*)value)->sort(fn); }
	~Dynamic() { if (value != NULL) delete value; }

	// Assign this to your own codepage converter if you want, otherwise leave it as NULL.
	static unsigned char (*utf8ToCodePageConverter)(const unsigned char* &s);

};


struct StructImplementationField {
	const char *name;			// This points to a UniqueString(), so don't free it or strdup it.
	Dynamic value;

	// When using the XML encoding, do you wish this field output as an attribute as opposed to an element?
	const char* xmlAttribute();
};


void parseXmlRpcMethodResponse(const char* s, Dynamic &output);
std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
std::string base64_decode(std::string const& encoded_string);


#endif DYNAMICTYPE_H
