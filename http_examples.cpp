#include "server_http.hpp"
#include "client_http.hpp"
#include "http_parser.h"

//Added for the default_resource example

#include<fstream>
#include <curl/curl.h> //your directory may be different

using namespace std;
//Added for the json-example:
using namespace boost::property_tree;

typedef SimpleWeb::Server<SimpleWeb::HTTP> HttpServer;
typedef SimpleWeb::Client<SimpleWeb::HTTP> HttpClient;

string data; //will hold the url's contents

size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up)
    { //callback must have this declaration
        //buf is a pointer to the data that curl has for us
        //size*nmemb is the size of the buffer
    
        for (int c = 0; c<size*nmemb; c++)
        {
            data.push_back(buf[c]);
        }
        return size*nmemb; //tell curl how many bytes we handled
    }

int main() {
    //HTTP-server at port 8080 using 4 threads
    HttpServer server(8000, 4);
    
    //Add resources using path-regex and method-string, and an anonymous function
    //POST-example for the path /string, responds the posted string
    server.resource["^/string$"]["POST"]=[](HttpServer::Response& response, shared_ptr<HttpServer::Request> request) {
        //Retrieve string from istream (request->content)
        stringstream ss;
        request->content >> ss.rdbuf();
        string content=ss.str();
        
        response << "HTTP/1.1 200 OK\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
    };
    
    //POST-example for the path /json, responds firstName+" "+lastName from the posted json
    //Responds with an appropriate error message if the posted json is not valid, or if firstName or lastName is missing
    //Example posted json:
    //{
    //  "firstName": "John",
    //  "lastName": "Smith",
    //  "age": 25
    //}
    server.resource["^/json$"]["POST"]=[](HttpServer::Response& response, shared_ptr<HttpServer::Request> request) {
        try {
            ptree pt;
            read_json(request->content, pt);

            string name=pt.get<string>("name");
            string token = pt.get<string>("token");
            
            string url = "https://graph.facebook.com/oauth/access_token?grant_type=fb_exchange_token&client_id=980137892017050&client_secret=4eeffa3691366a4b46bca9aa250e089a&fb_exchange_token=" + token;
            
            CURL* curl; //our curl object

            curl_global_init(CURL_GLOBAL_ALL); //pretty obvious
            curl = curl_easy_init();
        
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); //tell curl to output its progress
        
            curl_easy_perform(curl);
            
            response << "HTTP/1.1 200 OK\r\nContent-Length: " << data.length() << "\r\n\r\n" << data;
        
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            data.clear();
        }
        catch(exception& e) {
            response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
        }
    };
    
    //GET-example for the path /info
    //Responds with request-information
    server.resource["^/info$"]["GET"]=[](HttpServer::Response& response, shared_ptr<HttpServer::Request> request) {
        stringstream content_stream;
        content_stream << "<h1>Request:</h1>";
        content_stream << request->method << " " << request->path << " HTTP/" << request->http_version << "<br>";
        for(auto& header: request->header) {
            content_stream << header.first << ": " << header.second << "<br>";
        }
        
        //find length of content_stream (length received using content_stream.tellp())
        content_stream.seekp(0, ios::end);
        
        response <<  "HTTP/1.1 200 OK\r\nContent-Length: " << content_stream.tellp() << "\r\n\r\n" << content_stream.rdbuf();
    };
    
    //GET-example for the path /match/[number], responds with the matched string in path (number)
    //For instance a request GET /match/123 will receive: 123
    server.resource["^/[a-z]+/?\\?([a-z0-9A-Z]+=[a-z0-9A-Z]*)(&[a-z0-9A-Z]+=[a-z0-9A-Z]*)*$"]["GET"]=[](HttpServer::Response& response, shared_ptr<HttpServer::Request> request) {
        HttpParser parser;
        
        Str *number = NULL;
        RestAPI *api = NULL;
        string uri = request->path_match[0];
        int status = parser.parse(uri);
        
        
        if (status == SUCCESS) {
			api = parser.getAPI();
			api->getResponse(number);
		}
        else if (status == BAD_API) number = new Str("Error: Bad API! Please check the document for help!");
        else if (status == NO_API) number = new Str("Error: No API available for your request");
        else number = new Str("Unknown error, please check later");
        
        //response << "HTTP/1.1 200 OK\r\nContent-Length: " << number.length() + user.length() << "\r\n\r\n" << number << "   " << user;
        response << "HTTP/1.1 200 OK\r\nContent-Length: " << number->length() << "\r\n\r\n" << *number;
        
        delete number;
    };
    
    //Default GET-example. If no other matches, this anonymous function will be called. 
    //Will respond with content in the web/-directory, and its subdirectories.
    //Default file: index.html
    //Can for instance be used to retrieve an HTML 5 client that uses REST-resources on this server
    server.default_resource["GET"]=[](HttpServer::Response& response, shared_ptr<HttpServer::Request> request) {
        string filename="web";
        
        string path=request->path;
        
        //Replace all ".." with "." (so we can't leave the web-directory)
        size_t pos;
        while((pos=path.find(".."))!=string::npos) {
            path.erase(pos, 1);
        }
        
        filename+=path;
        ifstream ifs;
        //A simple platform-independent file-or-directory check do not exist, but this works in most of the cases:
        if(filename.find('.')==string::npos) {
            if(filename[filename.length()-1]!='/')
                filename+='/';
            filename+="index.html";
        }
        ifs.open(filename, ifstream::in);
        
        if(ifs) {
            ifs.seekg(0, ios::end);
            size_t length=ifs.tellg();
            
            ifs.seekg(0, ios::beg);

            response << "HTTP/1.1 200 OK\r\nContent-Length: " << length << "\r\n\r\n";
            
            //read and send 128 KB at a time if file-size>buffer_size
            size_t buffer_size=131072;
            if(length>buffer_size) {
                vector<char> buffer(buffer_size);
                size_t read_length;
                while((read_length=ifs.read(&buffer[0], buffer_size).gcount())>0) {
                    response.stream.write(&buffer[0], read_length);
                    response << HttpServer::flush;
                }
            }
            else
                response << ifs.rdbuf();

            ifs.close();
        }
        else {
            string content="Could not open file "+filename;
            response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
        }
    };
    
    thread server_thread([&server](){
        //Start server
        server.start();
    });
    
    //Wait for server to start so that the client can connect
    // this_thread::sleep_for(chrono::seconds(1));
    /*
    //Client examples
    HttpClient client("localhost:8080");
    auto r1=client.request("GET", "/match/123");
    cout << r1->content.rdbuf() << endl;

    string json="{\"firstName\": \"John\",\"lastName\": \"Smith\",\"age\": 25}";
    stringstream ss(json);    
    auto r2=client.request("POST", "/string", ss);
    cout << r2->content.rdbuf() << endl;
    
    ss.str(json);
    auto r3=client.request("POST", "/json", ss);
    cout << r3->content.rdbuf() << endl;
        */
    server_thread.join();
    
    return 0;
}
