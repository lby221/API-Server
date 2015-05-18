default: http_examples

http_examples: http_examples.cpp http_parser.cpp
	g++ -O3 -std=c++11 http_examples.cpp http_parser.cpp -lboost_system -lboost_thread -lboost_coroutine -lboost_context -pthread -lcurl -o http_examples
