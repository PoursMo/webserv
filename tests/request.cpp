#include "../headers/Request.hpp"
#include <stdexcept>
#include <iostream>

int main()
{
	Request test_request;

	char test_line[]="POST / HTTP/1.1\r\n";
	char *first = &(test_line[0]);

	int i = 0;
	while (test_line[i])
		i++;
	char *last = first + i - 1;

	char test_line2[]="Content-Size: test\r\n";
	char *first2 = &(test_line2[0]);

	int i2 = 0;
	while (test_line2[i2])
		i2++;
	char *last2 = first2 + i2 - 1;

	test_request.parseRequestLine(first, last);
	test_request.parseRequestLine(first2, last2);
	std::string target = "Content-Size";
	std::cout << "valid first line" << std::endl;
	std::cout << "Header Value of " << target << " is : " << test_request.getHeaderValue(target) << std::endl;
	return 0;
}
