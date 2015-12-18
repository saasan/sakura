#include <stdio.h>
#include "ygg.cpp"

using namespace yggdrasil;

void test_xml()
{
	xml_file root("cdftest.xml");
	root.read();

	if (root.parse_error.is_error())
	{
		printf("Parse Error!!\n");
		return;
	}

	ygg_node node_arc = root["/adf/archiver"];
	int section_num = node_arc.get_size();

	for (ygg_iterator it_arc = node_arc.begin(); it_arc.is_not_end(); ++it_arc)
	{
		ygg_node node_list;

		node_list = (*it_arc)["./file"];
		for (ygg_iterator it_name = node_list.begin(); it_name.is_not_end(); ++it_name)
		{
			printf("file      : %s\n", (*it_name).get_value().c_str());
		}

		printf("function  : %s\n", (*it_arc)["./function"].get_value().c_str());

		node_list = (*it_arc)["./extension"];
		for (ygg_iterator it_ext = node_list.begin(); it_ext.is_not_end(); ++it_ext)
		{
			printf("extension : %s\n", (*it_ext).get_value().c_str());
		}

		printf("wildcard  : %s\n", (*it_arc)["./wildcard"].get_value().c_str());

		node_list = (*it_arc)["./decode/command"];
		for (ygg_iterator it_com = node_list.begin(); it_com.is_not_end(); ++it_com)
		{
			printf("decode/command : %s\n", (*it_com).get_value().c_str());
		}
	}
}

int main()
{
	test_xml();
	return 0;
}
