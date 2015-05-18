#include "RestAPI.hpp"

class API1 : public RestAPI {
private:	
	int a;
public:
	API1(APIMap *ref) : RestAPI(ref) {}
};
