#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <stdint.h>
#include <stdlib.h>

namespace ft_json
{

	class Value
	{
	public:
		enum Type
		{
			STRING,
			NUMBER,
			OBJECT,
			ARRAY,
			BOOLEAN,
			NULLTYPE
		};

		// Constructors
		Value() : type(NULLTYPE) {}
		Value(const std::string &str) : type(STRING) { string_value = new std::string(str); }
		Value(int64_t num) : type(NUMBER) { number_value = num; }
		Value(const std::map<std::string, Value> &obj) : type(OBJECT) { object_value = new std::map<std::string, Value>(obj); }
		Value(const std::vector<Value> &arr) : type(ARRAY) { array_value = new std::vector<Value>(arr); }
		Value(bool b) : type(BOOLEAN) { bool_value = b; }

		// Copy constructor
		Value(const Value &rhs) : type(rhs.type)
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
				object_value = new std::map<std::string, Value>(*rhs.object_value);
				break;
			case ARRAY:
				array_value = new std::vector<Value>(*rhs.array_value);
				break;
			case BOOLEAN:
				bool_value = rhs.bool_value;
				break;
			default:
				break;
			}
		}

		// Copy assignment operator
		Value &operator=(const Value &rhs)
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
					object_value = new std::map<std::string, Value>(*rhs.object_value);
					break;
				case ARRAY:
					array_value = new std::vector<Value>(*rhs.array_value);
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
		~Value()
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
		Type getType() const { return type; }
		const std::string &asString() const
		{
			if (type != STRING)
				throw std::runtime_error("Value is not a string");
			return *string_value;
		}
		int64_t asNumber() const
		{
			if (type != NUMBER)
				throw std::runtime_error("Value is not a number");
			return number_value;
		}
		const std::map<std::string, Value> &asObject() const
		{
			if (type != OBJECT)
				throw std::runtime_error("Value is not an object");
			return *object_value;
		}
		const std::vector<Value> &asArray() const
		{
			if (type != ARRAY)
				throw std::runtime_error("Value is not an array");
			return *array_value;
		}
		bool asBoolean() const
		{
			if (type != BOOLEAN)
				throw std::runtime_error("Value is not a boolean");
			return bool_value;
		}

	private:
		union
		{
			std::string *string_value;
			int64_t number_value;
			std::map<std::string, Value> *object_value;
			std::vector<Value> *array_value;
			bool bool_value;
		};
		Type type;
	};

	bool is_whitespace(const char &c)
	{
		return c == ' ' || c == '\n' || c == '\r' || c == '\t';
	}

	void skip_whitespaces(std::string::iterator &it)
	{
		while (is_whitespace(*it))
			it++;
	}

	Value parse_value(std::string::iterator &it);
	Value parse_string(std::string::iterator &it);
	Value parse_number(std::string::iterator &it);
	Value parse_object(std::string::iterator &it);
	Value parse_array(std::string::iterator &it);
	Value parse_keyword(std::string::iterator &it);

	// Doesnt handle backslash "\"
	Value parse_string(std::string::iterator &it)
	{
		std::string str;
		while (*it != '"')
		{
			str += *it;
			it++;
		}
		it++;
		return Value(str);
	}

	// Only handles integers
	Value parse_number(std::string::iterator &it)
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
		return Value(std::atol(num_str.c_str()));
	}

	Value parse_object(std::string::iterator &it)
	{
		std::map<std::string, Value> obj;
		while (1)
		{
			skip_whitespaces(it);
			if (*it == '}')
			{
				it++;
				break;
			}
			std::string key = parse_string(++it).asString();
			skip_whitespaces(it);
			if (*it != ':')
				throw std::runtime_error("Expected ':' after key");
			Value value = parse_value(++it);
			obj[key] = value;
			skip_whitespaces(it);
			if (*it == ',')
				it++;
			else if (*it != '}')
				throw std::runtime_error("Expected ',' or '}' in object");
		}
		return Value(obj);
	}

	Value parse_array(std::string::iterator &it)
	{
		std::vector<Value> arr;
		while (1)
		{
			skip_whitespaces(it);
			if (*it == ']')
			{
				it++;
				break;
			}
			arr.push_back(parse_value(it));
			skip_whitespaces(it);
			if (*it == ',')
				it++;
			else if (*it != ']')
				throw std::runtime_error("Expected ',' or ']' in array");
		}
		return Value(arr);
	}

	Value parse_keyword(std::string::iterator &it)
	{
		std::string keyword;
		while (std::isalpha(*it))
		{
			keyword += *it;
			it++;
		}
		if (keyword == "true")
			return Value(true);
		if (keyword == "false")
			return Value(false);
		if (keyword == "null")
			return Value();
		throw std::runtime_error("Unknown keyword");
	}

	Value parse_value(std::string::iterator &it)
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
		throw std::runtime_error("Unexpected character");
	}
}

#include <fstream>
#include <iostream>

// Simple printing function for demonstration purposes
void print_value(const ft_json::Value &value)
{
	switch (value.getType())
	{
	case ft_json::Value::STRING:
		std::cout << "\"" << value.asString() << "\"";
		break;
	case ft_json::Value::NUMBER:
		std::cout << value.asNumber();
		break;
	case ft_json::Value::OBJECT:
	{
		std::cout << "{";
		const std::map<std::string, ft_json::Value> &obj = value.asObject();
		for (std::map<std::string, ft_json::Value>::const_iterator it = obj.begin(); it != obj.end(); ++it)
		{
			if (it != obj.begin())
				std::cout << ", ";
			std::cout << "\"" << it->first << "\": ";
			print_value(it->second);
		}
		std::cout << "}";
		break;
	}
	case ft_json::Value::ARRAY:
	{
		std::cout << "[";
		const std::vector<ft_json::Value> &arr = value.asArray();
		for (size_t i = 0; i < arr.size(); ++i)
		{
			if (i > 0)
				std::cout << ", ";
			print_value(arr[i]);
		}
		std::cout << "]";
		break;
	}
	case ft_json::Value::BOOLEAN:
		std::cout << (value.asBoolean() ? "true" : "false");
		break;
	case ft_json::Value::NULLTYPE:
		std::cout << "null";
		break;
	}
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
		return 1;
	}

	std::ifstream file(argv[1]);
	if (!file)
	{
		std::cerr << "Error: Could not open file " << argv[1] << std::endl;
		return 1;
	}

	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	std::string::iterator iterator = content.begin();
	ft_json::Value parsed = ft_json::parse_value(iterator);

	print_value(parsed);
	std::cout << std::endl;
}