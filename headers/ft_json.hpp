#ifndef FT_JSON_HPP
#define FT_JSON_HPP

#include <map>
#include <vector>
#include <string>
#include <stdint.h>

namespace ft_json
{
	enum Type
	{
		STRING,
		NUMBER,
		OBJECT,
		ARRAY,
		BOOLEAN,
		NULLTYPE
	};

	class JsonValue
	{
	public:
		// Constructors
		JsonValue();
		JsonValue(const std::string &str);
		JsonValue(int64_t num);
		JsonValue(const std::map<std::string, JsonValue> &obj);
		JsonValue(const std::vector<JsonValue> &arr);
		JsonValue(bool b);

		// Copy constructor
		JsonValue(const JsonValue &rhs);

		// Copy assignment operator
		JsonValue &operator=(const JsonValue &rhs);

		// Destructor
		~JsonValue();

		// Accessors
		Type getType() const;
		const std::string &asString() const;
		int64_t asNumber() const;
		const std::map<std::string, JsonValue> &asObject() const;
		const std::vector<JsonValue> &asArray() const;
		bool asBoolean() const;

	private:
		union
		{
			std::string *string_value;
			int64_t number_value;
			std::map<std::string, JsonValue> *object_value;
			std::vector<JsonValue> *array_value;
			bool bool_value;
		};
		Type type;
	};

	std::ostream &operator<<(std::ostream &os, const ft_json::JsonValue &value);

	typedef std::map<std::string, JsonValue> JsonObject;
	typedef std::vector<JsonValue> JsonArray;

	JsonValue parse_json(std::string file_content);
	JsonValue parse_json(std::ifstream &file);

	void print_json_value(const ft_json::JsonValue &value);
}

#endif