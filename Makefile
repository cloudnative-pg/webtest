http_test: http_test.cpp
	clang++ -o http_test -std=c++17 http_test.cpp -I$$(pg_config --includedir) $$(pg_config --ldflags) -L$$(pg_config  --libdir) -lpq -lpthread

fmt:
	clang-format -i http_test.cpp
