#include "utils.hpp"

std::string int_to_str(int num)
{
	bool isNegative = (num < 0);
	if (isNegative)
		num = -num;
	std::string result;
	do
	{
		result += '0' + (num % 10);
		num /= 10;
	} while (num > 0);
	if (isNegative)
		result += '-';
	std::reverse(result.begin(), result.end());
	return result;
}