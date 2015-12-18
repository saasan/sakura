// arguments.h
#ifndef __ARGUMENTS_H__
#define __ARGUMENTS_H__

#include <string>
#include <vector>

namespace s2lib
{

template <typename T> struct arguments_traits;
template <> struct arguments_traits<char>
{
	static const char space;
	static const char quot;
};
template <> struct arguments_traits<wchar_t>
{
	static const wchar_t space;
	static const wchar_t quot;
};

const char arguments_traits<char>::space = ' ';
const char arguments_traits<char>::quot = '"';
const wchar_t arguments_traits<wchar_t>::space = L' ';
const wchar_t arguments_traits<wchar_t>::quot = L'\"';

template <class T, class CT = std::char_traits<T>, class AT = std::allocator<T> >
class arguments : public std::vector<std::basic_string<T, CT, AT> >
{
private:
	typedef arguments_traits<T> traits_type;
	typedef std::basic_string<T, CT, AT> string_type;

public:
	arguments(const T *i_cmdline)
	{
		string_type cmdline = i_cmdline;
		string_type::iterator it = cmdline.begin();
		string_type::iterator itend = cmdline.end();

		while (it != itend)
		{
			string_type temp;

			if (*it != traits_type::space)
			{
				T end = (*it == traits_type::quot ? traits_type::quot : traits_type::space);

				if (*it == traits_type::quot)
				{
					it++;
				}

				while ((it != itend) && (*it != end))
				{
					temp += *it;
					it++;
				}

				if (!temp.empty())
				{
					push_back(temp);
				}

				if (it == itend)
				{
					break;
				}
			}
			it++;
		}
	}
};

} // namespace s2lib

#endif // __ARGUMENTS_H__
