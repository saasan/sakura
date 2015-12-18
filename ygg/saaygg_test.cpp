#include <stdio.h>
#include "ygg.cpp"

using namespace yggdrasil;

template <class N, class S>
class CException
{
public:
	CException(N nCode, S strMessage)
	{
		m_nCode = nCode;
		m_strMessage = strMessage;
	}

	virtual ~CException()
	{
	}

	virtual N GetCode() const
	{
		return m_nCode;
	}

	virtual void GetMessage(S &strMessage) const
	{
		strMessage = m_strMessage;
	}

private:
	N m_nCode;
	S m_strMessage;
};

class error_print: public sax_handler
{
public:
	void on_error(ygg_error *parse_error);
};

void error_print::on_error(ygg_error *parse_error)
{
	if (parse_error->is_error())
	{
		CException<int, ygg_string> e(parse_error->get_category(), parse_error->get_message().c_str());
		throw(e);

#if 0
		printf("On Error !!!\n"
			"\tlocation %d:%d\n"
			"\tcode     %02x:%02x:%02x:%02x\n"
			"\tmessage  %s\n",
			parse_error->get_line(),
			parse_error->get_row(),
			parse_error->get_genre(),
			parse_error->get_category(),
			parse_error->get_sequence(),
			parse_error->get_sequence_case(),
			parse_error->get_message().c_str());
#endif
	}
}

void test_xml()
{
	error_print sax;
	xml_file root("cdftest.xml");
	//sox_file root("lzh.sox");

#if 0
	try
	{
		root.read(&sax);
	}
	catch (const CException<int, ygg_string> &e)
	{
		ygg_string msg;
		e.GetMessage(msg);
		printf("Error : code:%d : %s\n", e.GetCode(), msg.c_str());
		return;
	}
#else
	root.read();
#endif

	if (root.parse_error.is_error())
	{
		printf("On Error !!!\n"
			"\tlocation %d:%d\n"
			"\tcode     %02x:%02x:%02x:%02x\n"
			"\tmessage  %s\n",
			root.parse_error.get_line(),
			root.parse_error.get_row(),
			root.parse_error.get_genre(),
			root.parse_error.get_category(),
			root.parse_error.get_sequence(),
			root.parse_error.get_sequence_case(),
			root.parse_error.get_message().c_str());
	}
	else
	{

		ygg_node section = root["/converter/section"];
		int section_num = section.get_size();

		for (int i = 0; i < section_num; i++)
		{
			ygg_node node_list;
			printf("/converter/section/\n\n");

			printf("type/ : %s\n",		section[i]["./type/@value"].begin()->get_value().c_str());

			node_list = section[i]["./file/"];
			for (ygg_iterator j = node_list.begin(); j.is_not_end(); ++j)
			{
				printf("file/ : %s\n", (*j)["@value"].get_value().c_str());
			}

			printf("function/ : %s\n",	section[i]["./function/@value"].begin()->get_value().c_str());

			node_list = section[i]["./extension/"];
			for (ygg_iterator k = node_list.begin(); k.is_not_end(); ++k)
			{
				printf("extension/ : %s\n", (*k)["@value"].get_value().c_str());
			}

			printf("icon/ : %s\n",		section[i]["./icon/@value"].begin()->get_value().c_str());
			printf("wildcard/ : %s\n",	section[i]["./wildcard/@value"].begin()->get_value().c_str());
			printf("decode/command : %s\n",	section[i]["./decode/command/@value"].begin()->get_value().c_str());

			printf("\n");
		}
	}
}

int main()
{
	test_xml();
	return 0;
}
