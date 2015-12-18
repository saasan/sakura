// converter.h
#ifndef __CONVERTER_H__
#define __CONVERTER_H__

#include <wtypes.h>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>

#define __USING_SJIS__
#include "ygg/ygg.h"

namespace s2lib
{

class converter
{
public:
	converter();
	virtual ~converter() {}

	// 対応ファイルか調べる
	virtual bool is_file_supported(LPCTSTR filePath) = 0;
	// 対応拡張子か調べる
	virtual bool is_extension_supported(LPCTSTR filePath) = 0;
	// 対応拡張子の取得
	virtual void get_extension(std::vector<std::string> &extlist) = 0;
	// デコード
	virtual bool decode(LPCTSTR sourcefile, LPCTSTR targetdir = NULL) = 0;
	// 必須ファイルのチェック
	virtual bool enable() = 0;

	// コンバータタイプ
	enum converter_type
	{
		unknown = 0,
		arc,
		spi,
		exe
	};

protected:
	HWND m_hParentWnd;						// 親ウィンドウのハンドル
	converter_type m_type;					// コンバータタイプ
	std::vector<std::string> m_file;		// ファイル名
	std::string m_function;					// 関数名
	std::vector<std::string> m_extension;	// 拡張子
	std::string m_icon;						// アイコンファイル名
	int m_icon_num;							// アイコン番号
	std::string m_wildcard;					// ワイルドカード
	// デコードコマンド
	std::vector<std::string> m_decode;
	// エンコードコマンド
	std::vector<pair<std::string, std::string> > m_encode;

};

/////////////////////////////////////////////////////////////////////////////
class common_archiver : public converter
{
public:
	common_archiver() : converter()
	{
		m_hDll = NULL;
		m_returnCode = -1;
	}

	common_archiver(const common_archiver &conv) : converter(conv)
	{
		m_hDll = conv.m_hDll;
		m_returnCode = conv.m_returnCode;
	}

	~common_archiver()
	{
		if (m_hDll)
		{
			FreeLibrary(m_hDll);
			m_hDll = NULL;
		}
	}

	const int FNAME_MAX32 = 512;

	typedef HGLOBAL HARC;

	enum check_type
	{
		CHECKARCHIVE_RAPID		= 0x00000000,	// 簡易(最初の３ヘッダまで)
		CHECKARCHIVE_BASIC		= 0x00000001,	// 標準(全てのヘッダ)
		CHECKARCHIVE_FULLCRC	= 0x00000002	// 完全(格納ファイルの CRC チェック)
	};

	enum error_code
	{
		ERROR_START				= 0x8000,
		// Warnings
		ERROR_DISK_SPACE		= 0x8005,
		ERROR_READ_ONLY			= 0x8006,
		ERROR_USER_SKIP			= 0x8007,
		ERROR_UNKNOWN_TYPE		= 0x8008,
		ERROR_METHOD			= 0x8009,
		ERROR_PASSWORD_FILE		= 0x800A,
		ERROR_VERSION			= 0x800B,
		ERROR_FILE_CRC			= 0x800C,
		ERROR_FILE_OPEN			= 0x800D,
		ERROR_MORE_FRESH		= 0x800E,
		ERROR_NOT_EXIST			= 0x800F,
		ERROR_ALREADY_EXIST		= 0x8010,
		ERROR_TOO_MANY_FILES	= 0x8011,
		// Errors
		// ERROR_DIRECTORY			= 0x8012,
		ERROR_MAKEDIRECTORY		= 0x8012,
		ERROR_CANNOT_WRITE		= 0x8013,
		ERROR_HUFFMAN_CODE		= 0x8014,
		ERROR_COMMENT_HEADER	= 0x8015,
		ERROR_HEADER_CRC		= 0x8016,
		ERROR_HEADER_BROKEN		= 0x8017,
		ERROR_ARCHIVE_FILE_OPEN	= 0x8018,
		ERROR_ARC_FILE_OPEN		= ERROR_ARCHIVE_FILE_OPEN,
		ERROR_NOT_ARC_FILE		= 0x8019,
		ERROR_NOT_ARCHIVE_FILE	= ERROR_NOT_ARC_FILE,
		ERROR_CANNOT_READ		= 0x801A,
		ERROR_FILE_STYLE		= 0x801B,
		ERROR_COMMAND_NAME		= 0x801C,
		ERROR_MORE_HEAP_MEMORY	= 0x801D,
		ERROR_ENOUGH_MEMORY		= 0x801E,
		ERROR_ALREADY_RUNNING	= 0x801F,
		ERROR_USER_CANCEL		= 0x8020,
		ERROR_HARC_ISNOT_OPENED	= 0x8021,
		ERROR_NOT_SEARCH_MODE	= 0x8022,
		ERROR_NOT_SUPPORT		= 0x8023,
		ERROR_TIME_STAMP		= 0x8024,
		// ERROR_NULL_POINTER		= 0x8025,
		// ERROR_ILLEGAL_PARAMETER	= 0x8026,
		ERROR_TMP_OPEN			= 0x8025,
		ERROR_LONG_FILE_NAME	= 0x8026,
		ERROR_ARC_READ_ONLY		= 0x8027,
		ERROR_SAME_NAME_FILE	= 0x8028,
		ERROR_NOT_FIND_ARC_FILE	= 0x8029,
		ERROR_RESPONSE_READ		= 0x802A,
		ERROR_NOT_FILENAME		= 0x802B,
		ERROR_TMP_COPY			= 0x802C,
		ERROR_EOF				= 0x802D,
		ERROR_ADD_TO_LARC		= 0x802E,
		ERROR_CANNOT_CREATE		= 0x8030,
		ERROR_CANNOT_OPEN		= 0x8031,
		ERROR_TMP_BACK_SPACE	= 0x802F,
		ERROR_SHARING			= 0x8030,
		ERROR_NOT_FIND_FILE		= 0x8031,
		ERROR_LOG_FILE			= 0x8032,
		ERROR_UNEXPECTED		= 0x8032,
		ERROR_BUF_TOO_SMALL		= 0x8033,
		ERROR_END				= ERROR_BUF_TOO_SMALL
	};

protected:
	HINSTANCE m_hDll;
	int m_returnCode;

	bool m_checkArchive(LPCTSTR filePath);
	int m_commandLine(HWND hWnd, LPCTSTR cmdLine);

private:
	typedef int (WINAPI *PCOMMAND)(const HWND hWnd, LPCSTR szCmdLine, LPSTR szOutput, const DWORD dwSize);
	typedef BOOL (WINAPI *PCHECKARC)(LPCSTR szFileName, const int iMode);
};

/////////////////////////////////////////////////////////////////////////////
class converter_manager
{
public:
	converter_manager();
	virtual ~converter_manager()
	{
		unload();
	}

	virtual bool load(LPCTSTR newSpiPath = NULL, LPCTSTR newXmlPath = NULL);
	virtual void unload();
	virtual bool add(converter &i_converter);
	virtual void get_converter(vector<converter> o_converter_list);
	virtual converter get_converter_extension(LPCTSTR fileName);
	virtual converter get_converter_file(LPCTSTR fileName);
	virtual int get_count();

protected:
	std::vector<converter> m_converter_list;
	std::string xml_path;
	std::string spi_path;
};

} // namespace s2lib

#endif // __CONVERTER_H__
