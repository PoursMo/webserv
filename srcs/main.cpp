#include "ft_json.hpp"

// Debug function for json
void print_json_value(const ft_json::JsonValue &value)
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
		for (std::map<std::string, ft_json::JsonValue>::const_iterator it = obj.begin(); it != obj.end(); ++it)
		{
			if (it != obj.begin())
				std::cout << ", ";
			std::cout << "\"" << it->first << "\": ";
			print_json_value(it->second);
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

void func(ft_json::JsonValue json);


int main(int argc, char **argv)
{
	const char *config_path;
	if (argc > 1)
		config_path = argv[1];
	else
		config_path = "default"; // create a default config file
	std::ifstream file(config_path);
	if (!file)
	{
		std::cerr << config_path << ": Could not open file " << std::endl;
		return 1;
	}
	try
	{
		ft_json::JsonValue parsed = ft_json::parse_json(file);
		print_json_value(parsed);
		std::cout << std::endl << std::endl;
		func(parsed);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
	}
}