#include "ft_json.hpp"

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
ft_json::Type ft_json::JsonValue::getType() const { return type; }
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

static bool is_whitespace(const char &c)
{
	return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

static void skip_whitespaces(std::string::iterator &it)
{
	while (is_whitespace(*it))
		it++;
}

static ft_json::JsonValue parse_value(std::string::iterator &it);

static ft_json::JsonValue parse_string(std::string::iterator &it)
{
	std::string str;
	while (*it != '"')
	{
		if (!(*it))
			throw std::runtime_error("Expected \" to close string: " + str);
		str += *it;
		it++;
	}
	it++;
	return ft_json::JsonValue(str);
}

static ft_json::JsonValue parse_number(std::string::iterator &it)
{
	std::string num_str;
	if (*it == '-')
	{
		num_str += *it;
		it++;
	}
	while (std::isdigit(*it))
	{
		num_str += *it;
		it++;
	}
	return ft_json::JsonValue(std::atol(num_str.c_str()));
}

static ft_json::JsonValue parse_object(std::string::iterator &it)
{
	std::map<std::string, ft_json::JsonValue> obj;
	bool value_expected = true;
	while (1)
	{
		skip_whitespaces(it);
		if (*it == '}')
		{
			if (value_expected)
				throw std::runtime_error("Expected value in object");
			it++;
			break;
		}
		if (*it != '"')
			throw std::runtime_error("Expected string key in object");
		std::string key = parse_string(++it).asString();
		if (obj.count(key))
			throw std::runtime_error("Duplicate key in object: " + key);
		skip_whitespaces(it);
		if (*it != ':')
			throw std::runtime_error("Expected ':' after key: " + key);
		ft_json::JsonValue value = parse_value(++it);
		obj[key] = value;
		skip_whitespaces(it);
		if (*it == ',')
		{
			it++;
			value_expected = true;
		}
		else
		{
			value_expected = false;
			if (*it != '}')
				throw std::runtime_error("Expected ',' or '}' in object");
		}
	}
	return ft_json::JsonValue(obj);
}

static ft_json::JsonValue parse_array(std::string::iterator &it)
{
	std::vector<ft_json::JsonValue> arr;
	bool value_expected = true;
	while (1)
	{
		skip_whitespaces(it);
		if (*it == ']')
		{
			if (value_expected)
				throw std::runtime_error("Expected value in array");
			it++;
			break;
		}
		arr.push_back(parse_value(it));
		skip_whitespaces(it);
		if (*it == ',')
		{
			it++;
			value_expected = true;
		}
		else
		{
			value_expected = false;
			if (*it != ']')
				throw std::runtime_error("Expected ',' or ']' in array");
		}
	}
	return ft_json::JsonValue(arr);
}

static ft_json::JsonValue parse_keyword(std::string::iterator &it)
{
	std::string keyword;
	while (std::isalpha(*it))
	{
		keyword += *it;
		it++;
	}
	if (keyword == "true")
		return ft_json::JsonValue(true);
	if (keyword == "false")
		return ft_json::JsonValue(false);
	if (keyword == "null")
		return ft_json::JsonValue();
	throw std::runtime_error("Unknown keyword: " + keyword);
}

static ft_json::JsonValue parse_value(std::string::iterator &it)
{
	skip_whitespaces(it);
	if (*it == '"')
		return parse_string(++it);
	if (*it == '-' || std::isdigit(*it))
		return parse_number(it);
	if (*it == '{')
		return parse_object(++it);
	if (*it == '[')
		return parse_array(++it);
	if (std::isalpha(*it))
		return parse_keyword(it);
	throw std::runtime_error("Unexpected character: " + *it);
}

ft_json::JsonValue ft_json::parse_json(std::string file_content)
{
	std::string::iterator iterator = file_content.begin();
	ft_json::JsonValue parsed = parse_value(iterator);
	skip_whitespaces(iterator);
	if (*iterator)
		throw std::runtime_error("Trailing characters in file");
	return parsed;
}

ft_json::JsonValue ft_json::parse_json(std::ifstream &file)
{
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return parse_json(content);
}
