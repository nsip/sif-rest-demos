/*
Plain Xml RPC C++ client for Windows
------------------------------------

Created by Dr Tim Cooper,  tco@smartsgroup.com,   Nov 2010.

This project grew out of the "XmlC4Win" sourceforge project when I 
decided that plain XML was a better protocol for RPC than Xml.

-------------------------Sample app:-------------------------

#include <iostream>
#include "RestLib.h"



void Test(std::string username, std::string password)
{
	Dynamic args, result;

	RestClient Connection("https://www.edval.com.au:9001/test.php");
	args[0] = username;
	args[1] = Md5(username + "." + password);		// salted Md5
	if (not Connection.execute("tetun.aprende", args, result)) {
		std::cerr << "Error: " << Connection.getError() << std::endl;
	}
	else {
		std::cout << "The answer is: " << std::string(result) << std::endl;
	}
}

*/


#ifndef RESTLIB_H
#define RESTLIB_H


#include <string>
#include <vector>
#include <map>
#include <sstream>

#ifndef DYNAMICTYPE_H
#include "DynamicType.h"
#endif


typedef char* str;
typedef const char* kstr;


extern void snazzyFree(void *ptr);
extern void* snazzyMalloc(size_t n);
extern char* snazzyStrdup(const char* s);
extern void* snazzyRealloc(void* ptr, size_t size);




typedef void (*RestCallback)(void* context, char* status);



/* A 'get username and password' function is used for HTTP basic authentication.
It will either get the (username,password) pair from some stored location, or
prompt the user for it. 
	Return 'true' if the user attempted to supply the credentials, or 'false'
if they want to cancel.
	Display a 'logon failed' message if 'retry'.
*/
typedef bool (*getBasicAuth_UsernameAndPassword_fn)(bool retry, char username[256], char password[256]);


class RestClient {
	class RestImplementation *secret;

public:
	char* headers;

	enum protocol_enum { XMLRPC_AUTO=0, XMLRPC_HTTP=1, XMLRPC_HTTPS=2 };

	//! Construct a client and attempt to connect to the server at the specified host:port address
	//!	@param host The name of the remote machine hosting the server
	//!	@param port The port on the remote machine where the server is listening
	//!	@param object	An optional object name to be sent in the HTTP GET header
	RestClient(kstr server, int port, kstr object, protocol_enum protocol=XMLRPC_AUTO);

	//! Construct a client and attempt to connect to the server at the specified URI.
	//!	@param URI  (Commonly and previously known as "URL"): e.g. "https://www.edval.com.au:9001/test.php"
	RestClient(kstr URI, FILE* debugFile=NULL);

	~RestClient() { close(); }

	//! Execute the named procedure on the remote server.
	//!	@param method The name of the remote procedure to execute
	//!	@param params An array of the arguments for the method
	//!	@param result The result value to be returned to the client
	//!	@return true if the request was sent and a result received 
	//!	 (although the result might be a fault).
	//!
	//! Currently this is a synchronous (blocking) implementation (execute
	//! does not return until it receives a response or an error). Use isFault()
	//! to determine whether the result is a fault response.
	bool executeXmlRpc(const char* method, Dynamic& params, Dynamic& result);

	//! This is how you access REST resources:
	int executeRestJson(const char* object, const char* verb, Dynamic const& json, Dynamic& result);
	int executeRestXml(const char* object, const char* verb, Dynamic const& input, Dynamic& result);

	//! Returns true if the result of the last execute() was a fault response.
	bool isFault() const;

	// Set the details for a callback function
	void setCallback(RestCallback Callback, void* context);

	// ignore the certificate authority on subsequent execute()'s.
	void setIgnoreCertificateAuthority(bool value=true);

	// Set stuff needed for Basic Authentication.  Either give this object a username and password,
	// or give it a function enabling it to get it from the user.
	void setBasicAuth_Callback(getBasicAuth_UsernameAndPassword_fn fn);
	void setBasicAuth_UsernameAndPassword(const char* username, const char* password);

	// Get and set error messages:
	std::string getError();
	void setError(std::string);

	// Enabling debug output:
	void setDebugFile(FILE *debug);

	// Should we try another server in the cluster?
	bool shouldTryAnotherServerInTheCluster();
	void changeURL(const char* URL);
	kstr getUrl();

	//! Close the connection
	void close();
};


#endif
