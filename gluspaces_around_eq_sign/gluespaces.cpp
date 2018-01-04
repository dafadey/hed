#include <string>
#include <iostream>

void glue(std::string& a)
{
  size_t pos(0);
	while (true)
  {
    pos = a.find("=", pos);
		if(pos == std::string::npos)
      break;
    size_t i1(1);
    while(a[pos-i1]==' ' || a[pos-i1]=='\t') i1++;
    size_t i2(0);
    while(a[pos+1+i2]==' ' || a[pos+1+i2]=='\t') i2++;
    std::cout << pos << ", " << i1 << ", " << i2 << std::endl;
    a.replace(pos-i1+1,i2+i1,"=");
    std::cout << a << std::endl;
    pos = pos - i1 + 2;
    std::cout << pos << std::endl;
   //break;   
  }
}

using namespace std;
int main()
{
  string a(" =hi there a=b c = d d= e n =  q ooo=	tfb");
  cout << a << endl;
  glue(a);
	cout << a << endl;
}
