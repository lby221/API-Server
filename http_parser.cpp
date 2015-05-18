#include "http_parser.h"

HttpParser::HttpParser(Str& str) {
	parse(str);
	ref = NULL;
}

HttpParser::~HttpParser() {
	if (ref) { delete ref; ref = 0; }
	if (api) { delete api; api = 0; }
}

type_api HttpParser::checkType(Str &str) {
	try {
        ptree pt;
        read_json("api_settings.json", pt);
        
        Str searchstr;
        searchstr.append(str);
        searchstr.append(".APIType");

        return pt.get<int>(searchstr);
    } catch(std::exception& e) {
        return -1;
    }
}


int HttpParser::parse(Str& str) {
	api = NULL;
	api_type = -1;
	if (str.size() < 3) {
		return BAD_API;
	}
	
	if (str[0] != '/') return BAD_API;	
	int pointer = 1;
	
	Str type_str;
	while (str[pointer] != '/' && str[pointer] != '?') {
		type_str.push_back(str[pointer++]);
	}
	
	while (str[pointer] == '/' || str[pointer] == '?')
		pointer++;
	
	bool flag = false;
	Str tname;
	Str tvalue;
	
	ref = new APIMap();
	
	for (int i = pointer; i < str.size(); i++) {
		if (str[i] == '&') {
			(*ref)[tname] = tvalue;
			flag = false;
			tname.clear();
			tvalue.clear();
		} else if (str[i] == '=') {
			flag = true;
		} else {
			if (flag) tvalue.push_back(str[i]);
			else tname.push_back(str[i]);
		}
	}
	(*ref)[tname] = tvalue;
	
	api_type = checkType(type_str);
	
	switch(api_type) {
		case 1: api = new API1(ref); break;
		//case 2: api = new API2(ref); break;
		//case 3: api = new API3(ref); break;
		//case 4: api = new API4(ref); break;
		//case 5: api = new API5(ref); break;
	}
	
	if (api_type == -1) return NO_API;
	return SUCCESS;
}

bool HttpParser::isReady() {
	return !(api_type == -1);
}

RestAPI *HttpParser::getAPI() {
	return api;
}

Str HttpParser::getDebugStr() {
	Str debugstr;
	for ( APIMapIt it = ref->begin(); it != ref->end(); ++it ) {
		debugstr.append(" ");
		debugstr.append(it->first);
		debugstr.append(" : ");
		debugstr.append(it->second);
		debugstr.append(" ; ");
     }
     
     return debugstr;
 }
