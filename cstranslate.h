// cstranslate.h
#ifndef __CSTRANSLATE_H__
#define __CSTRANSLATE_H__

#include <string>
#include <stdlib.h>	// mbstowcs(), wcstombs()
#include <locale.h>	// setlocale()

namespace s2lib
{

size_t cs_translator(char *dest, const wchar_t *src, size_t n)
{
	return wcstombs(dest, src, n);
}

size_t cs_translator(wchar_t *dest, const char *src, size_t n)
{
	return mbstowcs(dest, src, n);
}

template<class from_string, class to_string>
class cs_translate_base
{
public:
	typedef cs_translate_base<from_string, to_string> this_type;
	typedef from_string from_string_type;
	typedef to_string to_string_type;

protected:
	from_string untranslated_buffer;
	to_string translated_buffer;

public:
	virtual void translate() = 0;

	virtual void clear()
	{
		untranslated_buffer.erase();
		translated_buffer.erase();
	}

	const to_string_type & ignite(const from_string_type &X)
	{
		untranslated_buffer = X;
		translate();
		return translated_buffer;
	}
};

template<class from_string, class to_string>
class cs_translate_engine : cs_translate_base<from_string, to_string>
{
public:
	typedef cs_translate_engine this_type;
	typedef cs_translate_base<from_string, to_string> base_type;
	typedef from_string from_string_type;
	typedef to_string to_string_type;

protected:
	cs_translate_engine() { }

protected:
	void translate()
	{
		translated_buffer.erase();

		if (untranslated_buffer.empty())
		{
			return;
		}

		setlocale(LC_ALL, "Japanese");

		size_t len;
		to_string_type::value_type *pstr = NULL;

		// 必要なバイト数を得る
		len = cs_translator(NULL, untranslated_buffer.c_str(), 0);
		// バッファの確保
		pstr = new to_string_type::value_type[(len / sizeof(to_string_type::value_type)) + 1];
		// 確保に失敗
		if (!pstr)
		{
			return;
		}

		cs_translator(pstr, untranslated_buffer.c_str(), len + 1);
		translated_buffer = pstr;

		delete [] pstr;
	}

public:
	static const to_string_type ignite(const from_string_type &X)
	{
		return this_type().base_type::ignite(X);
	}

};

typedef cs_translate_engine<std::string, std::wstring> mbcs_to_dbcs_engine;
typedef cs_translate_engine<std::wstring, std::string> dbcs_to_mbcs_engine;

const std::wstring (* const mbcs_to_dbcs)(const std::string &) = mbcs_to_dbcs_engine::ignite;
const std::string (* const dbcs_to_mbcs)(const std::wstring &) = dbcs_to_mbcs_engine::ignite;

} // namespace s2lib

#endif // __CSTRANSLATE_H__
