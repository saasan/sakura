#include <iostream>
#include <string>
#include "arguments.h"

int main()
{
	char cmd[] = "1st 2nd 3rd \"4th(1) 4th(2)\" 5th";

	s2lib::arguments<char> arg(cmd);
	s2lib::arguments<char>::iterator it = arg.begin();
	s2lib::arguments<char>::iterator itend = arg.end();

	while (it != itend)
	{
		std::cout << it->c_str() << std::endl;
		it++;
	}

	return 0;
}
