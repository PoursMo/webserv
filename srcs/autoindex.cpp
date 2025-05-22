#include "webserv.hpp"
#include "uri.hpp"

struct s_entrie {
	std::string name;
	std::string href;
	std::string path;
	off_t size;
};

void initDirectoryEntries(
	const std::string& path,
	std::vector<struct s_entrie>& folders,
	std::vector<struct s_entrie>& files)
{
	DIR* dirp;
	dirent *dp;
	struct stat statBuffer;
	struct s_entrie ent;

	dirp = opendir(path.c_str());
	if (!dirp)
		return ;
	while ((dp = readdir(dirp)))
	{
		ent.name = dp->d_name;
		if (ent.name == ".")
			continue;
		ent.href = encodeUri(ent.name);
		ent.path = path + "/" + ent.name;
		if (stat(ent.path.c_str(), &statBuffer) == -1)
			continue;
		ent.size = statBuffer.st_size;
		if (statBuffer.st_mode & S_IFDIR)
		{
			ent.name += "/";
			ent.href += "/";
			folders.push_back(ent);
			continue;
		}
		files.push_back(ent);
	}
	closedir(dirp);
}

std::string getAutoIndexHtml(const std::string &path, const std::string &root)
{
	std::stringstream html;
	std::vector<struct s_entrie> folders;
	std::vector<struct s_entrie> files;
	std::string loc = path.substr(root.size());
	
	initDirectoryEntries(path, folders, files);
	html << "<!DOCTYPE html>";
	html << "<html lang=\"en\">";
	html << "<head>";
	html << "<meta charset=\"UTF-8\">";
	html << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
	html << "<title>Index of " << loc << "</title>";
	html << "</head>";
	html << "</body>";
	html << "<div style=\"display: flex; flex-direction: column; gap: .2rem; max-width: 40rem; margin: auto;\">";
	html << "<h1>Index of " << loc << "</h1>";

	for (std::vector<struct s_entrie>::const_iterator it = folders.begin(); it != folders.end(); it++)
	{
		html << "<a href=\"" << it->href << "\">";
		html << "<span>" << "📂" << "</span>";
		html << "<span class=\"entrie-name\">" << it->name << "</span>";
		html << "</a>";
	}
	for (std::vector<struct s_entrie>::const_iterator it = files.begin(); it != files.end(); it++)
	{
		html << "<a href=\"" << it->href << "\">";
		html << "<span>" << "📄" << "</span>";
		html << "<span class=\"entrie-name\">" << it->name << "</span>";
		html << "<span>" << it->size << "</span>";
		html << "</a>";
	}
	html << "<style>";
	html << "a {";
	html << "text-decoration: none;";
	html << "color: black;";
	html << "display: flex;";
    html << "gap: .5rem;";
    html << "align-items: center;";
    html << "padding: .3rem .6rem;";
    html << "border: #eee solid 1px;";
    html << "border-radius: .2rem;";
	html << "}";
	html << "a:hover {";
	html << "border-color: #bbb;";
	html << "background-color: #eee;";
	html << "}";
	html << ".entrie-name {flex-grow: 1;}";
	html << "</style>";
	
	html << "</div>";
	html << "</body>";
	html << "</html>";
	
	return html.str();
}
