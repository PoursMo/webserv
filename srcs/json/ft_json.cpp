#include "ft_json.hpp"
#include "utils.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>
#include <stdlib.h>

// ********************************************************************
// JsonValue class
// ********************************************************************

// Constructors
ft_json::JsonValue::JsonValue() : type(NULLTYPE)
{
}
ft_json::JsonValue::JsonValue(const std::string &str) : type(STRING)
{
	string_value = new std::string(str);
}
ft_json::JsonValue::JsonValue(int64_t num) : type(NUMBER)
{
	number_value = num;
}
ft_json::JsonValue::JsonValue(const std::map<std::string, JsonValue> &obj) : type(OBJECT)
{
	object_value = new std::map<std::string, JsonValue>(obj);
}
ft_json::JsonValue::JsonValue(const std::vector<JsonValue> &arr) : type(ARRAY)
{
	array_value = new std::vector<JsonValue>(arr);
}
ft_json::JsonValue::JsonValue(bool b) : type(BOOLEAN)
{
	bool_value = b;
}

// Copy constructor
ft_json::JsonValue::JsonValue(const JsonValue &rhs) : type(rhs.type)
{
	switch (type)
	{
	case STRING:
		string_value = new std::string(*rhs.string_value);
		break;
	case NUMBER:
		number_value = rhs.number_value;
		break;
	case OBJECT:
		object_value = new std::map<std::string, JsonValue>(*rhs.object_value);
		break;
	case ARRAY:
		array_value = new std::vector<JsonValue>(*rhs.array_value);
		break;
	case BOOLEAN:
		bool_value = rhs.bool_value;
		break;
	default:
		break;
	}
}

// Copy assignment operator
ft_json::JsonValue &ft_json::JsonValue::operator=(const JsonValue &rhs)
{
	if (this != &rhs)
	{
		type = rhs.type;
		switch (type)
		{
		case STRING:
			string_value = new std::string(*rhs.string_value);
			break;
		case NUMBER:
			number_value = rhs.number_value;
			break;
		case OBJECT:
			object_value = new std::map<std::string, JsonValue>(*rhs.object_value);
			break;
		case ARRAY:
			array_value = new std::vector<JsonValue>(*rhs.array_value);
			break;
		case BOOLEAN:
			bool_value = rhs.bool_value;
			break;
		default:
			break;
		}
	}
	return *this;
}

// Destructor
ft_json::JsonValue::~JsonValue()
{
	switch (type)
	{
	case STRING:
		delete string_value;
		break;
	case OBJECT:
		delete object_value;
		break;
	case ARRAY:
		delete array_value;
		break;
	default:
		break;
	}
}

// Accessors
ft_json::Type ft_json::JsonValue::getType() const
{
	return type;
}

const std::string &ft_json::JsonValue::asString() const
{
	if (type != STRING)
		throw std::runtime_error("JsonValue is not a string");
	return *string_value;
}
int64_t ft_json::JsonValue::asNumber() const
{
	if (type != NUMBER)
		throw std::runtime_error("JsonValue is not a number");
	return number_value;
}
const std::map<std::string, ft_json::JsonValue> &ft_json::JsonValue::asObject() const
{
	if (type != OBJECT)
		throw std::runtime_error("JsonValue is not an object");
	return *object_value;
}
const std::vector<ft_json::JsonValue> &ft_json::JsonValue::asArray() const
{
	if (type != ARRAY)
		throw std::runtime_error("JsonValue is not an array");
	return *array_value;
}
bool ft_json::JsonValue::asBoolean() const
{
	if (type != BOOLEAN)
		throw std::runtime_error("JsonValue is not a boolean");
	return bool_value;
}

// ********************************************************************
// Json parsing
// ********************************************************************

static int line = 1;

static bool is_whitespace(const char &c)
{
	return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

static void skip_whitespaces(std::string::iterator &i)
{
	while (is_whitespace(*i))
	{
		if (*i == '\n')
			line++;
		i++;
	}
}

static ft_json::JsonValue parse_value(std::string::iterator &i);

static ft_json::JsonValue parse_string(std::string::iterator &i)
{
	std::string str;
	while (*i != '"')
	{
		if ((*i) == '\n')
			throw std::runtime_error("Unexpected newline in string");
		if (!(*i))
			throw std::runtime_error("Expected \" to close string");
		str += *i;
		i++;
	}
	i++;
	return ft_json::JsonValue(str);
}

static ft_json::JsonValue parse_number(std::string::iterator &i)
{
	std::string num_str;
	if (*i == '-')
	{
		num_str += *i;
		i++;
	}
	while (std::isdigit(*i))
	{
		num_str += *i;
		i++;
	}
	return ft_json::JsonValue(std::strtol(num_str.c_str(), 0, 10));
}

static ft_json::JsonValue parse_object(std::string::iterator &i)
{
	std::map<std::string, ft_json::JsonValue> obj;
	bool value_expected = true;
	while (1)
	{
		skip_whitespaces(i);
		if (*i == '}')
		{
			if (value_expected)
				throw std::runtime_error("Expected value in object");
			i++;
			break;
		}
		if (*i != '"')
			throw std::runtime_error("Expected string key in object");
		std::string key = parse_string(++i).asString();
		if (obj.count(key))
			throw std::runtime_error("Duplicate key in object");
		skip_whitespaces(i);
		if (*i != ':')
			throw std::runtime_error("Expected ':' after key");
		ft_json::JsonValue value = parse_value(++i);
		obj[key] = value;
		skip_whitespaces(i);
		if (*i == ',')
		{
			i++;
			value_expected = true;
		}
		else
		{
			value_expected = false;
			if (*i != '}')
				throw std::runtime_error("Expected ',' or '}' in object");
		}
	}
	return ft_json::JsonValue(obj);
}

static ft_json::JsonValue parse_array(std::string::iterator &i)
{
	std::vector<ft_json::JsonValue> arr;
	bool value_expected = true;
	while (1)
	{
		skip_whitespaces(i);
		if (*i == ']')
		{
			if (value_expected)
				throw std::runtime_error("Expected value in array");
			i++;
			break;
		}
		arr.push_back(parse_value(i));
		skip_whitespaces(i);
		if (*i == ',')
		{
			i++;
			value_expected = true;
		}
		else
		{
			value_expected = false;
			if (*i != ']')
				throw std::runtime_error("Expected ',' or ']' in array");
		}
	}
	return ft_json::JsonValue(arr);
}

static ft_json::JsonValue parse_keyword(std::string::iterator &i)
{
	std::string keyword;
	while (std::isalpha(*i))
	{
		keyword += *i;
		i++;
	}
	if (keyword == "true")
		return ft_json::JsonValue(true);
	if (keyword == "false")
		return ft_json::JsonValue(false);
	if (keyword == "null")
		return ft_json::JsonValue();
	throw std::runtime_error("Unknown keyword: " + keyword);
}

static ft_json::JsonValue parse_value(std::string::iterator &i)
{
	skip_whitespaces(i);
	if (*i == '"')
		return parse_string(++i);
	if (*i == '-' || std::isdigit(*i))
		return parse_number(i);
	if (*i == '{')
		return parse_object(++i);
	if (*i == '[')
		return parse_array(++i);
	if (std::isalpha(*i))
		return parse_keyword(i);
	throw std::runtime_error("Unexpected character: " + std::string(1, *i));
}

ft_json::JsonValue ft_json::parse_json(std::string file_content)
{
	try
	{
		std::string::iterator iterator = file_content.begin();
		ft_json::JsonValue parsed = parse_value(iterator);
		skip_whitespaces(iterator);
		if (*iterator)
			throw std::runtime_error("Trailing characters in file");
		return parsed;
	}
	catch (const std::exception &e)
	{
		throw std::runtime_error("line " + int_to_str(line) + ": " + e.what());
	}
}

ft_json::JsonValue ft_json::parse_json(std::ifstream &file)
{
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return parse_json(content);
}

// ********************************************************************
// Debug
// ********************************************************************

// Debug function for json
void ft_json::print_json_value(const ft_json::JsonValue &value)
{
	switch (value.getType())
	{
	case ft_json::STRING:
		std::cout << "\"" << value.asString() << "\"";
		break;
	case ft_json::NUMBER:
		std::cout << value.asNumber();
		break;
	case ft_json::OBJECT:
	{
		std::cout << "{";
		const std::map<std::string, ft_json::JsonValue> &obj = value.asObject();
		for (std::map<std::string, ft_json::JsonValue>::const_iterator i = obj.begin(); i != obj.end(); ++i)
		{
			if (i != obj.begin())
				std::cout << ", ";
			std::cout << "\"" << i->first << "\": ";
			print_json_value(i->second);
		}
		std::cout << "}";
		break;
	}
	case ft_json::ARRAY:
	{
		std::cout << "[";
		const std::vector<ft_json::JsonValue> &arr = value.asArray();
		for (size_t i = 0; i < arr.size(); ++i)
		{
			if (i > 0)
				std::cout << ", ";
			print_json_value(arr[i]);
		}
		std::cout << "]";
		break;
	}
	case ft_json::BOOLEAN:
		std::cout << (value.asBoolean() ? "true" : "false");
		break;
	case ft_json::NULLTYPE:
		std::cout << "null";
		break;
	}
}