#include "print.hpp"

namespace Horizon::Win32
{

const char* m_pPrintTypes[] =
{
	"[Trace]",
	"[Warning]",
	"[Error]",
};

const wchar_t* m_pWidePrintTypes[] =
{
	L"[Trace]",
	L"[Warning]",
	L"[Error]",
};

void DebugPrint( PrintType print_type, const char* message )
{
	char output[4096] = { };
	
	auto type = m_pPrintTypes[static_cast<int>( print_type )];
	sprintf_s( output, "%s %s\r\n", type, message );

	printf( output );
}

void DebugPrint( PrintType print_type, const wchar_t* message )
{
	wchar_t output[4096] = { };

	auto type = m_pWidePrintTypes[static_cast<int>( print_type )];
	wsprintfW( output, L"%s %s\r\n", type, message );

	OutputDebugStringW( output );
}

void Trace( const char* format, ... )
{
	char message[2048] = { };

	va_list args;
	va_start( args, format );
	int length = vsprintf_s( message, format, args );
	va_end( args );

	DebugPrint( PrintType::Trace, message );
}

void Trace( const wchar_t* format, ... )
{
	wchar_t message[2048] = { };

	va_list args;
	va_start( args, format );
	int length = wvsprintfW( message, format, args );
	va_end( args );

	DebugPrint( PrintType::Trace, message );
}

void Warning( const char* format, ... )
{
	char message[2048] = { };

	va_list args;
	va_start( args, format );
	int length = vsprintf_s( message, format, args );
	va_end( args );

	DebugPrint( PrintType::Warning, message );
}

void Warning( const wchar_t* format, ... )
{
	wchar_t message[2048] = { };

	va_list args;
	va_start( args, format );
	int length = wvsprintfW( message, format, args );
	va_end( args );

	DebugPrint( PrintType::Warning, message );
}

void Error( const char* format, ... )
{
	char message[2048] = { };

	va_list args;
	va_start( args, format );
	int length = vsprintf_s( message, format, args );
	va_end( args );

	DebugPrint( PrintType::Error, message );
}

void Error( const wchar_t* format, ... )
{
	wchar_t message[2048] = { };

	va_list args;
	va_start( args, format );
	int length = wvsprintfW( message, format, args );
	va_end( args );

	DebugPrint( PrintType::Error, message );
}

}