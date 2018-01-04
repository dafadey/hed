#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;



string remove_spaces_around(string& s)
{
	string out;
	size_t spaces(0);
	for(size_t i=0;i!=s.size();i++)
	{
		if(s[i] != ' ' && s[i] != '\t')
		{
			if(out.size()!=0)
				out.append(spaces,' ');
			out.append(1,s[i]);
			spaces = 0;
		}
		else
			spaces++;
	}
	return out;
}


void process_line(string line, bool enumerated, ostream& out = cout)
{
	static size_t id(0);
	enum eMode {ID, AUTH, TITLE, JOURNAL, VOL, PAGES, YEAR};
	eMode m;
	if(enumerated)
		m=ID;
	else
		m=AUTH;
	vector<string> auths;
	string sid;
	string title;
	string journal;
	string vol;
	string pages;
	string year;
	string auth;
	for(size_t i=0;i!=line.size();i++)
	{
		char c=line[i];
		//manage state
		if(m==ID && c=='.')
		{
			m=AUTH;
			c=line[++i];
		}
		if(m==AUTH && c=='\"')
		{
			m=TITLE;
			c=line[++i];
		}
		else if(m==TITLE && c=='\"')
		{
			m=JOURNAL;
			c=line[++i];
		}
		else if(m==JOURNAL && c>=48 && c<=57)
			m=VOL;
		else if(m==VOL && c==',')
			m=PAGES;
		else if(m==PAGES && c=='(')
			m=YEAR;

		if(m==ID)
			sid.append(1, c);
		if(m==AUTH)
		{
			if(c != ',')
				auth.append(1, c);
			if(c == ',')
			{
				auths.push_back(auth);
				auth.clear();
			}
		}		
		if(m==TITLE && c!='\"')
			title.append(1, c);
		if(m==JOURNAL && c!=',')
			journal.append(1, c);
		if(m==VOL && c!=' ' && c!=',')
			vol.append(1, c);
		if(m==PAGES && c!=' ' && c!=',')
			pages.append(1, c);
		if(m==YEAR && c!=' ' && c!=',' && c!='(' && c!=')')
			year.append(1, c);
	}
	
	sid = remove_spaces_around(sid);
	for(size_t i=0; i!=auths.size(); i++)
		auths[i]=remove_spaces_around(auths[i]);
	title = remove_spaces_around(title);
	journal = remove_spaces_around(journal);
	vol = remove_spaces_around(vol);
	pages = remove_spaces_around(pages);
	year = remove_spaces_around(year);
  
	if(enumerated)
		out << "@article{c" << sid << "," << endl;	
	else
		out << "@article{c" << id << "," << endl;	
		
	out << "author = {";
	for(size_t i=0; i!= auths.size(); i++)
		cout << auths[i] << (i != auths.size() - 1 ? " and " : "");
	out << "}," << endl;
	
	out << "title = {" << title << "}," << endl;
	
	out << "journal = {" << journal << "}," << endl;
	
	out << "volume = {" << vol << "}," << endl;
	
	out << "pages = {" << pages << "}," << endl;
	
	out << "year = {" << year << "}," << endl;
	
	out << "}" << endl << endl;
	
	id++;
}

int main(int argc, char* argv[])
{
	bool enumerated=false;
  if(argc < 2)
		cout << "leaving. please specify one input utf8 file" << endl;
	if(argc == 3)
	{
		if(string(argv[2])=="enumerated")
			enumerated=true;
	}
	ifstream inf(argv[1]);	
	string line;
	while (std::getline(inf, line))
		process_line(line, enumerated);
	inf.close();
  return 0;
}
