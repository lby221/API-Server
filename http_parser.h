// Http Parser for parsing http request strings
// Version 0.1
// Baiyu Liu

#include <string>
#include <fstream>
#include <unordered_map>

#include "API1.h"

#define  BAD_API		1
#define  SUCCESS		0
#define  NO_API			2
#define  INTERNAL_ERR	3

class HttpParser
{
private:
	type_api api_type;
	APIMap *ref;
	RestAPI *api;
	
	type_api checkType(Str &str);
	
public:
	HttpParser() : api(NULL), api_type(-1), ref(NULL) {}
	~HttpParser();
	
	HttpParser(Str& str);
	
	int parse(Str& str);
	bool isReady();
	RestAPI *getAPI();
	
	Str getDebugStr();
};
