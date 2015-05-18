// Rest API base class
// Version 0.1
// Baiyu Liu

#include <unordered_map>

//Added for the json-example
#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
using namespace boost::property_tree;

#define API_KEY_FILE	"api_keylist.json"

//------ErrorCode--------------//
#define RESPONSE_SUCCESS		0
#define RESPONSE_ERR_NO_KEY		1
#define RESPONSE_ERR_NO_PREV	2
#define RESPONSE_ERR_INTERNAL	-1

typedef  std::unordered_map<std::string, std::string>	APIMap;
typedef  APIMap::iterator								APIMapIt;
typedef  std::string	Str;
typedef  int 			type_api;

class RestAPI {
private:
	APIMap *ref;
	
	bool verifyKey() {
		APIMapIt it = ref->find("u");
		if (it == ref->end()) return RESPONSE_ERR_NO_PREV;
		Str u = it->second;
		
		it = ref->find("v");
		if (it == ref->end()) return RESPONSE_ERR_NO_KEY;
		Str v = it->second;
		
		Str rv;
		try {
			ptree pt;
			read_json(API_KEY_FILE, pt);

			rv = pt.get<Str>(u);
		} catch(std::exception& e) {
			return RESPONSE_ERR_NO_PREV;
		}
		
		// Needs encryption
		if (rv.compare(v) != 0) return RESPONSE_ERR_NO_PREV;
		else return RESPONSE_SUCCESS;
	}
public:
	RestAPI(APIMap *r) : ref(r) {}
	
	int getResponse(Str *&str) {
		int code = verifyKey();
		if (code != RESPONSE_SUCCESS) {
			str = new Str("No previledge to use this API");
			return code; 
		}
		
		return processResponse(str);
	}
	
protected:
	int processResponse(Str *&str) {
		str = new Str("API has not been implemented");
		return RESPONSE_ERR_INTERNAL;
	}
};
