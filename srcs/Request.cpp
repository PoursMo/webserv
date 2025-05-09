#include "Request.hpp"

Request::Request(/* args */)
{
}

Request::~Request()
{
}

bool isMethod(char *lstart, char *lend, std::string method)
{

}

void Request::parseFirstLine(char *lstart, char *lend)
{
	if (*lstart == 'G')
	{
		if (!isMethod(lstart, lend, "GET"))
			//ERROR
	}
	else if (*lstart == 'P')
	{
		if (!isMethod(lstart, lend, "POST"))
			//ERROR
	}
	else if (*lstart == 'D')
	{
		if (!isMethod(lstart, lend, "DELETE"))
			//ERROR
	}
	else
		//Throw Error 501, unsupported method or 401 error
	}
}

void Request::parseHeaderLine(char *lstart, char *lend)
{

}

void Request::parseRequestLine(char *lstart, char *lend)
{
	if (this->method == NULL)
		parseFirstLine(lstart, lend);
	else
		parseHeaderLine(lstart, lend);
}

int main()
{


	return 0;
}