#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include "dirent.h"
#include "sys/stat.h"

struct s_entrie {
	std::string name;
	std::string path;
	std::string uri;
	off_t size;
};

void initDirectoryEntries(
	const std::string& root,
	const std::string& path,
	std::vector<struct s_entrie>& folders,
	std::vector<struct s_entrie>& files)
{
	std::string dirPath = root + path;
	DIR* dirp;
	dirent *dp;
	struct stat statBuf;
	struct s_entrie ent;
	
	dirp = opendir(dirPath.c_str());
	if (!dirp)
		return ;
	while ((dp = readdir(dirp)))
	{
		ent.name = dp->d_name;
		if (ent.name == ".")
			continue;
		ent.path = dirPath + "/" + ent.name;
		ent.uri = path + "/" + ent.name;
		if (stat(ent.path.c_str(), &statBuf) == -1)
			continue;
		ent.size = statBuf.st_size;
		if (statBuf.st_mode & S_IFDIR)
		{
			ent.name += "/";
			folders.push_back(ent);
			continue;
		}
		files.push_back(ent);
	}
	closedir(dirp);
}

std::string getIndexHtml(const std::string &root, const std::string &path)
{
	std::stringstream html;
	std::vector<struct s_entrie> folders;
	std::vector<struct s_entrie> files;

	initDirectoryEntries(root, path, folders, files);
	html << "<!DOCTYPE html>";
	html << "<html lang=\"en\">";
	html << "<head>";
	html << "<meta charset=\"UTF-8\">";
	html << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
	html << "<title>Index of " << path << "</title>";
	html << "</head>";
	html << "</body>";
	html << "<h1>Index of " << path << "</h1>";
	html << "<hr>";
	html << "<div>";

	for (std::vector<struct s_entrie>::const_iterator it = folders.begin(); it != folders.end(); it++)
	{
		html << "<div>";
		html << "<a href=\"" << it->uri << "\">";
		html << it->name;
		html << "</a>";
		html << "<span>";
		html << it->size;
		html << "</span>";
		html << "</div>";
	}
	for (std::vector<struct s_entrie>::const_iterator it = files.begin(); it != files.end(); it++)
	{
		html << "<div>";
		html << "<a href=\"" << it->uri << "\">";
		html << it->name;
		html << "</a>";
		html << "<span>";
		html << it->size;
		html << "</span>";
		html << "</div>";
	}
	
	html << "</div>";
	html << "</body>";
	html << "</html>";
	
	return html.str();
}

int main()
{
	std::string root = "/app";

	std::string html = getIndexHtml(root, "/tests");
	std::cout << html << std::endl;
}
