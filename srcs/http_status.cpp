#include "http_status.hpp"

std::map<int, std::string> http_status::statuses;

const std::string &http_status::get(int code)
{
	if (http_status::statuses.count(code))
		return http_status::statuses.at(code);
	return http_status::statuses.at(0);
}

void http_status::init()
{
	// Internal
	http_status::statuses[0x0] = "Unknown status code";

	// Informational
	http_status::statuses[100] = "Continue"; // <-- Cursed
	http_status::statuses[101] = "Switching Protocols";

	// Successful
	http_status::statuses[200] = "OK";
	http_status::statuses[201] = "Created";
	http_status::statuses[202] = "Accepted";
	http_status::statuses[203] = "Non-Autoritative Information";
	http_status::statuses[204] = "No Content";
	http_status::statuses[205] = "Reset Content";
	http_status::statuses[206] = "Partial Content";

	// Redirection
	http_status::statuses[300] = "Multiple Choices";
	http_status::statuses[301] = "Moved Permanently";
	http_status::statuses[302] = "Found";
	http_status::statuses[303] = "See other";
	http_status::statuses[304] = "Not modified";
	http_status::statuses[307] = "Temporary Redirect";
	http_status::statuses[308] = "Permanent Redirect";

	// Client Error
	http_status::statuses[400] = "Bad Request";
	http_status::statuses[401] = "Unauthorized";
	http_status::statuses[402] = "Payment Required";
	http_status::statuses[403] = "Forbidden";
	http_status::statuses[404] = "Not Found";
	http_status::statuses[405] = "Method Not Allowed";
	http_status::statuses[406] = "Not Acceptable";
	http_status::statuses[407] = "Proxy Authentication Required";
	http_status::statuses[408] = "Request Timeout";
	http_status::statuses[409] = "Conflict";
	http_status::statuses[410] = "Gone";
	http_status::statuses[411] = "Length Required";
	http_status::statuses[412] = "Precondition Failed";
	http_status::statuses[413] = "Payload Too Large";
	http_status::statuses[414] = "URI Too Long";
	http_status::statuses[415] = "Unsupported Media Type";
	http_status::statuses[416] = "Range Not Satisfiable";
	http_status::statuses[417] = "Expectation Failed";
	http_status::statuses[418] = "Im A Teapot";
	http_status::statuses[421] = "Misdirected Request";
	http_status::statuses[422] = "Unprocessable Entity";
	http_status::statuses[423] = "Locked";
	http_status::statuses[424] = "Failed Dependency";
	http_status::statuses[425] = "Too Early";
	http_status::statuses[426] = "Upgrade Required";
	http_status::statuses[428] = "Precondition Required";
	http_status::statuses[429] = "Too Many Requests";
	http_status::statuses[431] = "Request Header Fields Too Large";
	http_status::statuses[451] = "Unavailable For Legal Reasons";

	// Server Error
	http_status::statuses[500] = "Internal Server Error";
	http_status::statuses[501] = "Not Implemented";
	http_status::statuses[502] = "Bad Gateway";
	http_status::statuses[503] = "Service Unavailable";
	http_status::statuses[504] = "Gateway Timeout";
	http_status::statuses[505] = "HTTP Version Not Supported";
	http_status::statuses[506] = "Variant Also Negotiates";
	http_status::statuses[507] = "Insufficient Storage";
	http_status::statuses[508] = "Loop Detected";
	http_status::statuses[509] = "Bandwidth Limit Exceeded";
	http_status::statuses[510] = "Not Extended";
	http_status::statuses[511] = "Network Authentication Required";
}
