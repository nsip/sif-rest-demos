/** Written by Tim Cooper from Edval:  tim@edval.com.au  */


#undef UNICODE
#define _SECURE_SCL		0			// Disable "checked iterators".  STL is too slow otherwise.
#include <windows.h>
#include <wininet.h>
#include <fstream>
#include "DynamicType.h"
#include "RestLib.h"
#include "tfc.h"




#define not		!
#define	or		||
#define and		&&
extern int L;
extern void stop();



/*-------------------------- class RestClient: ----------------------*/

class RestImplementation {
	RestClient::protocol_enum protocol;
	bool ignoreCertificateAuthority;
	HINTERNET hInternet;
	HINTERNET hConnect;
	std::string object;
	DWORD HttpErrcode;
	int port;

	void hadError(str function);
	bool connect(const char* server);
	char* execute(const char* method, const char* RestVerb, const char* headers, 
						const char* acceptTypes, const char* input);

public:
	struct BasicAuth_node {
		getBasicAuth_UsernameAndPassword_fn FindUsernameAndPassword;
		char username[256];
		char password[256];

		BasicAuth_node() { 
			FindUsernameAndPassword = NULL;
			username[0] = '\0';
			password[0] = '\0';
		}
	} BasicAuth;
	RestCallback Callback;
	FILE *debugFile;
	void *context;
	int totalBytes;
	kstr URL;
	std::string errmsg;
	int httpStatus;
	bool isFault;
	bool shouldTryAnotherServerInTheCluster;

	RestImplementation(const char* server, int port, const char* object, RestClient::protocol_enum protocol);
	RestImplementation(const char* URI, FILE *debugFile);
	bool executeXmlRpc(const char* method, const char* headers, Dynamic& params, Dynamic& result);
	bool executeRestJson(const char* URI, const char* httpVerb, const char* headers, Dynamic const& input, Dynamic& result);
	bool executeRestXml(const char* URI, const char* httpVerb, const char* headers, Dynamic const& input, Dynamic& result);
	void setCallback(RestCallback Callback, void* context)
			{ this->Callback = Callback; this->context = context; }
	void setIgnoreCertificateAuthority(bool value) { ignoreCertificateAuthority = value; }
	void setDebugFile(FILE *_debugFile) { debugFile = _debugFile; }
	~RestImplementation();
};


RestClient::RestClient(const char* server, int port, const char* object, protocol_enum protocol)
{
	secret = new RestImplementation(server, port, object, protocol);
	headers = NULL;
}


RestClient::RestClient(const char* URI, FILE *debugFile)
{
	secret = new RestImplementation(URI, debugFile);
	headers = NULL;
}


void RestClient::changeURL(const char* URL)
{	
	FILE *debugFile = secret ? secret->debugFile : NULL;
	if (secret != NULL)
		delete secret;
	secret = new RestImplementation(URL, debugFile);
}


kstr RestClient::getUrl()
{
	return secret->URL;
}


bool RestClient::executeXmlRpc(const char* method, Dynamic & params, Dynamic& result)
{
	bool myresult = yes;
	TfcBusyPush();
	myresult = secret->executeXmlRpc(method, headers, params, result);
	TfcBusyPop();

	TfcWaitBox(NULL);
	return myresult;
}


int RestClient::executeRestJson(const char* object, const char* verb,
										Dynamic const& json, Dynamic& result)
{
	bool myresult = yes;

	TfcBusyPush();
	myresult = secret->executeRestJson(object, verb, headers, json, result);
	TfcBusyPop();

	TfcWaitBox(NULL);
	if (secret->httpStatus == 200 and not myresult)
		return 401;
	return secret->httpStatus;
}


int RestClient::executeRestXml(const char* object, const char* verb,
										Dynamic const& input, Dynamic& result)
{
	bool myresult = yes;

	TfcBusyPush();
	try {
		myresult = secret->executeRestXml(object, verb, headers, input, result);
	}
	catch (RestException e) {
		secret->errmsg = e.getMessage();
		secret->httpStatus = 600;
	}
	TfcBusyPop();

	TfcWaitBox(NULL);
	if (secret->httpStatus == 200 and not myresult)
		return 600;
	return secret->httpStatus;
}


std::string RestClient::getError()
{
	if (secret->errmsg.size() > 1254)
		secret->errmsg.resize(1254);
	return secret->errmsg;
}


void RestClient::setError(std::string msg)
{
	secret->errmsg = msg;
}


void RestClient::setCallback(RestCallback Callback, void* context)
{
	secret->setCallback(Callback, context);
}


void RestClient::setBasicAuth_Callback(getBasicAuth_UsernameAndPassword_fn fn)
{
	secret->BasicAuth.FindUsernameAndPassword = fn;
}


void RestClient::setBasicAuth_UsernameAndPassword(const char* username, const char* password)
{
	strcpy(secret->BasicAuth.username, username);
	strcpy(secret->BasicAuth.password, password);
}


void RestClient::setIgnoreCertificateAuthority(bool value)
{
	secret->setIgnoreCertificateAuthority(value);
}


void RestClient::setDebugFile(FILE *debugFile)
{
	secret->setDebugFile(debugFile);
}


bool RestClient::isFault() const
{
	return secret->isFault;
}


bool RestClient::shouldTryAnotherServerInTheCluster()
{
	return secret->shouldTryAnotherServerInTheCluster;
}


void RestClient::close()
{
	delete secret;
	secret = NULL;
}


RestImplementation::RestImplementation(const char* server, int _port, const char* _object, 
												RestClient::protocol_enum _protocol)
{
	port = _port;
	object = _object;
	if (_protocol == RestClient::XMLRPC_AUTO)
		protocol =	(port == 80) ? RestClient::XMLRPC_HTTP : 
					(port == 443) ? RestClient::XMLRPC_HTTPS : RestClient::XMLRPC_HTTP;
	else protocol = _protocol;
	ignoreCertificateAuthority = false;
	hConnect = NULL;
	debugFile = NULL;
	context = NULL;
	totalBytes = 0;
	isFault = false;
	HttpErrcode = 0;
	URL = NULL;
	connect(server);
}


RestImplementation::RestImplementation(const char* URI, FILE* debugFile)
{
	port = 0;
	ignoreCertificateAuthority = false;
	hConnect = NULL;
	context = NULL;
	Callback = NULL;
	debugFile = NULL;
	totalBytes = 0;
	isFault = false;
	HttpErrcode = 0;
	URL = strdup(URI);
	if (strbegins(URI, "https://", false)) {
		protocol = RestClient::XMLRPC_HTTPS;
		URI += 8;
		port = 443;
	}
	else if (strbegins(URI, "http://", false)) {
		protocol = RestClient::XMLRPC_HTTP;
		URI += 7;
		port = 80;
	}
	else {
		errmsg = "The URI must begin with \"https://\" or \"http://\".";
		return;
	}
	kstr t = URI;
	while (*t != ':' and *t != '\0' and *t != '/')
		t++;
	std::string server(URI, t - URI);
	if (*t == ':') {
		t++;
		port = atoi(t);
		while (*t >= '0' and *t <= '9')
			t++;
	}
	object = t;		// should start with '/'.
	this->debugFile = debugFile;
	connect(server.c_str());
}


bool RestImplementation::connect(const char* server)
{
	Callback = NULL;
	context = NULL;
	totalBytes = 0;
	hInternet = InternetOpen("Xml", 0, NULL, NULL, 0);
	if (hInternet == NULL) {
		hadError("InternetOpen");
		return false;
	}
	if (debugFile) {
		fprintf(debugFile, "Attempting to connect to server:  %s   port: %d\n", server, port);
		fflush(debugFile);
	}
	hConnect = InternetConnect(hInternet, server, port, 
					NULL, NULL, INTERNET_SERVICE_HTTP, 0, (DWORD_PTR)this);
	if (hConnect == NULL) {
		hadError("InternetConnect");
		return false;
	}
	DWORD dwTimeout=999000;		// 999 seconds
	InternetSetOption(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &dwTimeout, sizeof(DWORD)); 
	InternetSetOption(hConnect, INTERNET_OPTION_RECEIVE_TIMEOUT, &dwTimeout, sizeof(DWORD));
	if (debugFile) {
		fputs("Got a handle\n", debugFile);
		fflush(debugFile);
	}
	return true;
}


// Converts a GetLastError() code into a human-readable string.
void RestImplementation::hadError(str function)
{   
	errmsg = function;
	errmsg += " : ";

	int LastError = GetLastError();
	if (LastError == ERROR_INTERNET_TIMEOUT)
		errmsg += "Internet timeout";
	else if (LastError == ERROR_INTERNET_INVALID_CA)
		errmsg += "Invalid certificate authority";
	else if (LastError == ERROR_INTERNET_SECURITY_CHANNEL_ERROR)
		errmsg += "Talking HTTPS to an HTTP server?";
	else if (LastError == ERROR_INTERNET_CANNOT_CONNECT)
		errmsg += "Failed to connect";
	else if (LastError == ERROR_INTERNET_NAME_NOT_RESOLVED)
		errmsg += "Name not resolved";
	else if (LastError == ERROR_INTERNET_INVALID_URL)
		errmsg += "Invalid URL";
	else if (LastError == ERROR_INTERNET_CONNECTION_RESET)
		errmsg += "Connection reset";
	else if (LastError == ERROR_INTERNET_NOT_INITIALIZED)
		errmsg += "Internet not initialised";
	else if (LastError == ERROR_INTERNET_CONNECTION_ABORTED)
		errmsg += "Connection aborted";
	else if (LastError == ERROR_INTERNET_SEC_CERT_REV_FAILED)
		errmsg += "Unable to check whether security certificate was revoked or not. "
			"Is your system clock time correct? Is the revocation server offline? or inaccessible?\n"
			"Try again later, or uncheck the IE Advanced option 'Check for security certificate revocation'";
	else if (LastError == ERROR_HTTP_INVALID_SERVER_RESPONSE)
		errmsg += "Unspecific error (12152): network connectivity instability, internal Windows issues, "
				"or the presence of firewalls or routers. These result in the incapability to download an essential file.";
	else {
		static str buf;
    	FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				LastError,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(str)&buf,
				0,
				NULL
		);
		if (buf == NULL) {
			char tmp[512];
			sprintf(tmp, "Error %d", LastError);
			errmsg += tmp;
		}
		else {
			errmsg += buf;
		    LocalFree(buf);
		}
	}
	if (debugFile) {
		fprintf(debugFile, "Error: %s\n", errmsg.c_str());
		fflush(debugFile);
	}
}


static void CALLBACK myInternetCallback(HINTERNET hInternet,
				DWORD_PTR dwContext,
				DWORD dwInternetStatus,
				LPVOID lpvStatusInformation,
				DWORD dwStatusInformationLength)
{
	RestImplementation *connection = (RestImplementation*)dwContext;
	if (connection and connection->Callback) {
		static char buf[512];
		str status;
		switch (dwInternetStatus) {
			case INTERNET_STATUS_RECEIVING_RESPONSE:	if (connection->totalBytes == 0)
															status = "Waiting for response\nPlease don't rush me - may take a moment."; 
														else status = "Receiving response\n(Don't hold your breath or cancel)";
														break;
			case INTERNET_STATUS_RESPONSE_RECEIVED:		status = "Response received"; break;
			case INTERNET_STATUS_HANDLE_CLOSING:		status = "Handle closing"; break;
			case INTERNET_STATUS_REQUEST_SENT:			status = "Data sent"; break;
			case INTERNET_STATUS_SENDING_REQUEST:		status = "Sending data"; break;
			default:									status = buf; 
														sprintf(buf, "Status %d", dwInternetStatus);
														break;
		}
		connection->Callback(connection->context, status);
	}
}


char* RestImplementation::execute(const char* method, const char* RestVerb, const char* headers, 
										const char* acceptTypes, kstr input)
// If you supply 'RestVerb' then we use the JSON REST system.
{
	errmsg = "";
	shouldTryAnotherServerInTheCluster = no;

	if (hConnect == NULL) {
		errmsg = "No connection";
		return NULL;
	}

	// Create the HttpOpenRequest object:
	if (debugFile) {
		fprintf(debugFile, "\n__________________________\n%s %s\n", RestVerb, method);
		fflush(debugFile);
	}
	if (Callback)
		Callback(context, "Sending data");
	if (acceptTypes == NULL)
		acceptTypes = "*/*";
	const char* _acceptTypes[2] = { acceptTypes, NULL };// It'll be "text/xml" or "text/json" or "application/xml" or "application/json".
	int flags = INTERNET_FLAG_DONT_CACHE;
	flags |= INTERNET_FLAG_KEEP_CONNECTION;
	if (protocol != RestClient::XMLRPC_HTTP)
		flags |= INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;
RETRY:
	HINTERNET hHttpFile;

	char uribuf[512];
	strcpy(uribuf, object.c_str());
	str s = uribuf + strlen(uribuf);
	strcpy(s, method);
	hHttpFile = HttpOpenRequest(
				  hConnect,
				  RestVerb,
				  uribuf,
				  HTTP_VERSION,
				  NULL,
				  _acceptTypes,
				  flags, 
				  (DWORD_PTR)this);
	if (hHttpFile == NULL) {
		hadError("HttpOpenRequest");
		return NULL;
	}
	if (debugFile) {
		fputs("HttpOpenRequest() succeeded\n", debugFile);
		fflush(debugFile);
	}
	InternetSetStatusCallback(hHttpFile, myInternetCallback);

	if (ignoreCertificateAuthority) {
		DWORD dwFlags;
		DWORD dwBuffLen = sizeof(dwFlags);

		InternetQueryOption(hHttpFile, INTERNET_OPTION_SECURITY_FLAGS,
					(LPVOID)&dwFlags, &dwBuffLen);
		dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
		InternetSetOption(hHttpFile, INTERNET_OPTION_SECURITY_FLAGS,
							&dwFlags, sizeof (dwFlags) );
	}

	// Add the 'Content-Type' and 'Content-length' headers
	if (strbegins(input, "<")) { //?xml")) {
		char header[255];		// Thanks, Anthony Chan.
		sprintf(header, "Content-Type: application/xml\r\nContent-length: %d", strlen(input));
		HttpAddRequestHeaders(hHttpFile, header, strlen(header), HTTP_ADDREQ_FLAG_ADD);
	}

	// Add the REST headers:
	if (headers) {
		HttpAddRequestHeaders(hHttpFile, headers, strlen(headers), HTTP_ADDREQ_FLAG_ADD);
	}

	// Debug output:
	if (debugFile) {
		fputs(input, debugFile);
		fflush(debugFile);
	}

	// Send the request:
	if (! HttpSendRequest(hHttpFile, NULL, 0, (LPVOID)input, strlen(input))) {
		hadError("HttpSendRequest");
		shouldTryAnotherServerInTheCluster = yes;
		return NULL;
	}
	if (Callback)
		Callback(context, "Data sent...");
	if (debugFile) {
		fflush(debugFile);
	}

	// Read the response:
	char* buf = NULL;
	int len = 0;
	do {
		DWORD bytesAvailable;
		if (!InternetQueryDataAvailable(hHttpFile, &bytesAvailable, 0, (DWORD_PTR)this)) {
			hadError("InternetQueryDataAvailable");
			break;
		}
		if (bytesAvailable == 0)
			break;		// This is the EOF condition.

		buf = (char*)snazzyRealloc(buf, len+bytesAvailable+1);

		// Read the data from the HINTERNET handle.
		DWORD bytesRead;
		if (!InternetReadFile(hHttpFile,
							 (LPVOID)(buf + len),
							 bytesAvailable,
							 &bytesRead))
		{
			hadError("InternetReadFile");
			break;
		}

		len += bytesRead;
		buf[len] = '\0';
		totalBytes = len;

	} while (true);

	//
	if (debugFile) {
		fputs("Response:\n", debugFile);
		fputs(buf, debugFile);
		fflush(debugFile);
	}

	// Roel Vanhout's insertion:  Did we get a HTTP_STATUS_OK response?  If not, why not?
	// XMLRPC spec says always return 200 for a valid answer. So if it's anything other than
 	// 200, it's an error (i.e., no support for redirects etc.).
 	DWORD buf_size;
  	buf_size = sizeof(DWORD);
 	if (!HttpQueryInfo(hHttpFile, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &HttpErrcode, &buf_size, 0)) {
 		errmsg = "Could not query HTTP result status";
		if (debugFile) {
			fputs("Could not query HTTP result status\n", debugFile);
			fflush(debugFile);
		}
 		snazzyFree(buf);
 		return NULL;
 	}
	httpStatus = HttpErrcode;
  	if (HttpErrcode >= 200 and HttpErrcode < 300) {
		// good.
	}
  	else if (HttpErrcode == HTTP_STATUS_DENIED or HttpErrcode == HTTP_STATUS_PROXY_AUTH_REQ) {
		if (BasicAuth.FindUsernameAndPassword) {
			snazzyFree(buf);
			buf = NULL;
			if (BasicAuth.FindUsernameAndPassword(BasicAuth.username[0] != '\0', BasicAuth.username, BasicAuth.password)) {
				InternetSetOption(hConnect, INTERNET_OPTION_PROXY_USERNAME, 
										(LPVOID)BasicAuth.username, lstrlen(BasicAuth.username));
				InternetSetOption(hConnect, INTERNET_OPTION_PROXY_PASSWORD, 
										(LPVOID)BasicAuth.password, lstrlen(BasicAuth.password));
				goto RETRY;
			}
		}
		errmsg = "Http status code: ";
		errmsg += HttpErrcode;
 	}
 
	// Close the HttpRequest object:
    InternetCloseHandle(hHttpFile);

	// Parse the response:
	if (len == 0) {
		snazzyFree(buf);		// 'buf' will always be NULL unless for some strange reason,
						// InternetReadFile() returns 'false' on the first pass.
		errmsg = "The server responded with an empty message.";
		return NULL;
	}

	// Finished:
	return buf;
}


bool RestImplementation::executeXmlRpc(const char* method, const char* headers, 
											Dynamic& params, Dynamic& result)
{
	std::ostringstream ostr;
	ostr << "<?xml version=\"1.0\"?>\r\n";
	ostr << "<methodCall><methodName>" << method << "</methodName>\r\n<params>";
	if (params.isArray())	{
		for (int i=0; i < params.size(); ++i) {
			ostr << "<param>";
			params[i].toXmlRpc(ostr);
			ostr << "</param>";
		}
	}
	else if (params.isNull()) {
	}
	else {
		ostr << "<param>";
		params.toXmlRpc(ostr);
		ostr << "</param>\n";
	}
	ostr << "</params></methodCall>\r\n";
	char* buf = execute("", "POST", headers, "*/xml", ostr.str().c_str());
	if (buf == NULL)
		return no;
	kstr s = buf;
	parseXmlRpcMethodResponse(s, result);
	snazzyFree(buf);
	return yes;
}


bool RestImplementation::executeRestJson(const char* URI, const char* httpVerb, 
								const char* headers, Dynamic const& input, Dynamic& result)
{
	std::ostringstream ostr;
	input.toJson(ostr);
	char* buf = execute(URI, httpVerb, headers, "*/json", ostr.str().c_str());
	if (buf == NULL)
		return no;
	kstr s = buf;
	result.fromJson(s);
	snazzyFree(buf);
	return yes;
}


bool RestImplementation::executeRestXml(const char* URI, const char* httpVerb, 
								const char* headers, Dynamic const& input, Dynamic& result)
{	char tag[512];

	std::ostringstream ostr;
	input.toXml(ostr);
	char* buf = execute(URI, httpVerb, headers, "application/xml", ostr.str().c_str());
	if (buf == NULL)
		return no;
	if (httpStatus < 200 or httpStatus >= 300) {
		errmsg = buf;
		return no;
	}
	if (strieq(httpVerb, "DELETE"))
		return yes;
	kstr s = buf;
	if (strbegins(s, "<?")) {
		s = strchr(s, '>');
		if (s == NULL)
			return no;
		s++;
	}
	result.fromXml(s, tag);
	snazzyFree(buf);
	return yes;
}


RestImplementation::~RestImplementation()
{
	if (hConnect)
		InternetCloseHandle(hConnect);
	if (hInternet)
		InternetCloseHandle(hInternet);
}



/*------------------------------------------*/
#include <iostream>

void Test3()
{
	RestClient Connection("http://web.edval.com.au/rpc");
	Connection.setIgnoreCertificateAuthority();
	//Connection.setBasicAuth_Callback(PopUpAPrettyDialog);
	Connection.setBasicAuth_UsernameAndPassword("foo", "goo");

	//  Call:  arumate.getKilowatts(string, integer)   :
	Dynamic args, result;
	args[0] = "test";
	args[1] = 1;

	// Replace this function name with your own:
	if (! Connection.executeXmlRpc("getKilowatts", args, result)) {
		std::cout << Connection.getError();
	}
	else if (result.isString())
		std::cout << std::string(result);
	else std::cout << "Success\n";
}


void Test4()
{	char tag[512];

	kstr xml1 = "<environment id=\"59f384a1-c767-4581-a5e9-f1e083c9e4d1\" type=\"DIRECT\"><sessionToken>e924413a28a6b5ae3a63044c63f30d26</sessionToken><solutionId>testSolution</solutionId><defaultZoneId>schoolTestingZone</defaultZoneId><authenticationMethod>Basic</authenticationMethod><instanceId></instanceId><userToken></userToken><consumerName>Edval</consumerName><applicationInfo><applicationKey>Edval</applicationKey><supportedInfrastructureVersion>3.0</supportedInfrastructureVersion><supportedDataModel>SIF-AU</supportedDataModel><supportedDataModelVersion>3.0</supportedDataModelVersion><transport>REST</transport><applicationProduct><vendorName>Edval</vendorName><productName>Edval</productName><productVersion></productVersion></applicationProduct></applicationInfo><infrastructureServices><infrastructureService name=\"environment\">http://rest3api.sifassociation.org/api/solutions/testSolution/environments/59f384a1-c767-4581-a5e9-f1e083c9e4d1</infrastructureService><infrastructureService name=\"requestsConnector\">http://rest3api.sifassociation.org/api/solutions/testSolution/requestsConnector</infrastructureService><infrastructureService name=\"provisionRequests\">http://rest3api.sifassociation.org/api/solutions/testSolution/provisionRequests</infrastructureService><infrastructureService name=\"queues\">http://rest3api.sifassociation.org/api/solutions/testSolution/queues</infrastructureService><infrastructureService name=\"subscriptions\">http://rest3api.sifassociation.org/api/solutions/testSolution/subscriptions</infrastructureService></infrastructureServices><provisionedZones><provisionedZone id=\"schoolTestingZone\"><services><service name=\"alerts\" contextId=\"DEFAULT\"><rights><right type=\"QUERY\">APPROVED</right><right type=\"CREATE\">APPROVED</right><right type=\"UPDATE\">APPROVED</right><right type=\"DELETE\">APPROVED</right><right type=\"PROVIDE\">REJECTED</right><right type=\"SUBSCRIBE\">SUPPORTED</right><right type=\"ADMIN\">REJECTED</right></rights></service><service name=\"students\" contextId=\"DEFAULT\"><rights><right type=\"QUERY\">APPROVED</right><right type=\"CREATE\">REJECTED</right><right type=\"UPDATE\">REJECTED</right><right type=\"DELETE\">REJECTED</right><right type=\"PROVIDE\">REJECTED</right><right type=\"SUBSCRIBE\">SUPPORTED</right><right type=\"ADMIN\">REJECTED</right></rights></service><service name=\"zones\" contextId=\"DEFAULT\"><rights><right type=\"QUERY\">APPROVED</right><right type=\"CREATE\">APPROVED</right><right type=\"UPDATE\">APPROVED</right><right type=\"DELETE\">APPROVED</right><right type=\"PROVIDE\">REJECTED</right><right type=\"SUBSCRIBE\">SUPPORTED</right><right type=\"ADMIN\">REJECTED</right></rights></service></services></provisionedZone></provisionedZones></environment>";
	kstr xml = "<infrastructureServices><infrastructureService name=\"environment\">http://rest3api.sifassociation.org/api/solutions/testSolution/environments/59f384a1-c767-4581-a5e9-f1e083c9e4d1</infrastructureService><infrastructureService name=\"requestsConnector\">http://rest3api.sifassociation.org/api/solutions/testSolution/requestsConnector</infrastructureService><infrastructureService name=\"provisionRequests\">http://rest3api.sifassociation.org/api/solutions/testSolution/provisionRequests</infrastructureService><infrastructureService name=\"queues\">http://rest3api.sifassociation.org/api/solutions/testSolution/queues</infrastructureService><infrastructureService name=\"subscriptions\">http://rest3api.sifassociation.org/api/solutions/testSolution/subscriptions</infrastructureService></infrastructureServices>";
	Dynamic result;
	result.fromXml(xml, tag);
	stop();
}


void Test5()
{
	RestClient Connection("http://localhost:8080/");
	Connection.setIgnoreCertificateAuthority();
	Connection.headers = "Authorization: Basic MmU1ZGQzY2EyODJmYzhkZGIzZDA4ZGNhY2M0MDdlOGE6MDJiY2VjZjdlYjA5N2U3NzgzMTk1ZjBlZDJhNmEwNmIK\n";

	Dynamic args, result;
	args.asStruct("userListAccounts");
	Dynamic &auth = args["auth"];
	auth["school"] = "CORRIMAL";
	auth["user"] = "Tim";
	auth["webcode"] = "dr pullem";
	auth["YYYY"] = 2013;

	if (! Connection.executeRestXml("rpc", "POST", args, result)) {
		std::cout << Connection.getError();
	}
	else if (result.isString())
		std::cout << std::string(result);
	else std::cout << "Success\n";
}


void Test6()
// Testing Scott's server which doesn't check authentication
{
	RestClient Connection("http://siftraining.dd.com.au/api/");
	Connection.headers = "Authorization: Basic MmU1ZGQzY2EyODJmYzhkZGIzZDA4ZGNhY2M0MDdlOGE6MDJiY2VjZjdlYjA5N2U3NzgzMTk1ZjBlZDJhNmEwNmIK\n";
	Connection.setIgnoreCertificateAuthority();

	Dynamic args, result;

	// Replace this function name with your own:
	int httpStatus = Connection.executeRestXml("StudentPersonals", "GET", args, result);
	if (httpStatus < 200 or httpStatus >= 300) {
		std::cout << Connection.getError();
	}
	else if (result.isString())
		std::cout << std::string(result);
	else std::cout << "Success\n";
}


static kstr requestEnvironmentXml = 
"<environment>\n"
"  <solutionId>testSolution</solutionId>\n"
"  <authenticationMethod>Basic</authenticationMethod>\n"
"  <instanceId></instanceId>\n"
"  <userToken></userToken>\n"
"  <consumerName>Edval</consumerName>\n"
"  <applicationInfo>\n"
"    <applicationKey>Edval</applicationKey>\n"
"    <supportedInfrastructureVersion>3.0</supportedInfrastructureVersion>\n"
"    <supportedDataModel>SIF-AU</supportedDataModel>\n"
"    <supportedDataModelVersion>3.0</supportedDataModelVersion>\n"
"    <transport>REST</transport>\n"
"    <applicationProduct>\n"
"      <vendorName>Edval</vendorName>\n"
"      <productName>Edval</productName>\n"
"      <productVersion></productVersion>\n"
"    </applicationProduct>\n"
"  </applicationInfo>\n"
"</environment>\n"
;


void Test7a()
// Testing authentication using the US server
{
	RestClient Connection("http://rest3api.sifassociation.org/api/");
	Connection.headers = "Authorization: Basic bmV3Omd1ZXN0\n";
	Connection.setIgnoreCertificateAuthority();

	Dynamic args, result;

	// Replace this function name with your own:
	char tag[512];
	args.fromXml(requestEnvironmentXml, tag);
	int httpStatus = Connection.executeRestXml("environments/environment", "POST", args, result);
	if (httpStatus < 200 or httpStatus >= 300) {
		std::cout << Connection.getError();
	}
	else if (result.isString())
		std::cout << std::string(result);
	else std::cout << "Success\n";
}


void Test7b()
// Testing authentication using the US server
{
	RestClient Connection("http://rest3api.sifassociation.org/api/solutions/testSolution/requestsConnector/");
	Connection.headers = "Authorization: Basic YTBjMjVmOGE3YjBiNGZlNTJlZjhkN2M1NDliOTkzYTM6Z3Vlc3Q=\n";
	Connection.setIgnoreCertificateAuthority();

	Dynamic args, result;

	// Replace this function name with your own:
	int httpStatus = Connection.executeRestJson("students", "GET", args, result);
	if (httpStatus < 200 or httpStatus >= 300) {
		std::cout << Connection.getError();
	}
	else if (result.isString())
		std::cout << std::string(result);
	else std::cout << "Success\n";
}


