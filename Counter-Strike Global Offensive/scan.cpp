#include "scan.hpp"
#include "print.hpp"

#define IS_IN_RANGE( value, max, min ) ( value >= max && value <= min )
#define GET_BITS( value ) ( IS_IN_RANGE( value, '0', '9' ) ? ( value - '0' ) : ( ( value & ( ~0x20 ) ) - 'A' + 0xA ) )
#define GET_BYTE( value ) ( GET_BITS( value[0] ) << 4 | GET_BITS( value[1] ) )

namespace Horizon::Memory
{

std::uintptr_t Scan( const std::string& image_name, const std::string& signature )
{
	auto image = GetModuleHandleA( image_name.c_str() );

	if( !image )
	{
#ifdef DEBUG
		Win32::Error( "GetModuleHandleA failed (Horizon::Memory::%s)", __FUNCTION__ );
#endif // DEBUG
		return 0u;
	}

	auto image_base = ( std::uintptr_t )( image );
	auto image_dos_hdr = ( IMAGE_DOS_HEADER* )( image_base );

	if( image_dos_hdr->e_magic != IMAGE_DOS_SIGNATURE )
	{
#ifdef DEBUG
		Win32::Error( "IMAGE_DOS_HEADER::e_magic is invalid (Horizon::Memory::%s)", __FUNCTION__ );
#endif // DEBUG
		return 0u;
	}

	auto image_nt_hdrs = ( IMAGE_NT_HEADERS* )( image_base + image_dos_hdr->e_lfanew );

	if( image_nt_hdrs->Signature != IMAGE_NT_SIGNATURE )
	{
#ifdef DEBUG
		Win32::Error( "IMAGE_NT_HEADERS::Signature is invalid (Horizon::Memory::%s)", __FUNCTION__ );
#endif // DEBUG
		return 0u;
	}

	auto scan_begin = ( std::uint8_t* )( image_base );
	auto scan_end = ( std::uint8_t* )( image_base + image_nt_hdrs->OptionalHeader.SizeOfImage );

	std::uint8_t* scan_result = nullptr;
	std::uint8_t* scan_data = ( std::uint8_t* )( signature.c_str() );

	for( auto current = scan_begin; current < scan_end; current++ )
	{
		if( *( std::uint8_t* )scan_data == '\?' || *current == GET_BYTE( scan_data ) )
		{
			if( !scan_result )
				scan_result = current;

			if( !scan_data[2] )
				return ( std::uintptr_t )( scan_result );

			scan_data += ( *( std::uint16_t* )scan_data == '\?\?' || *( std::uint8_t* )scan_data != '\?' ) ? 3 : 2;

			if( !*scan_data )
				return ( std::uintptr_t )( scan_result );
		}
		else if( scan_result )
		{
			current = scan_result;
			scan_data = ( std::uint8_t* )( signature.c_str() );
			scan_result = nullptr;
		}
	}

#ifdef DEBUG
	Win32::Warning( "Signature '%s' in module '%s' not found (Horizon::Memory::%s)", signature.c_str(), image_name.c_str(), __FUNCTION__ );
#endif // DEBUG

	return 0u;
}

std::uintptr_t Scan(const DWORD base_addr, const std::string& signature)
{
	if (!base_addr)
	{
#ifdef DEBUG
		Win32::Error("GetModuleHandleA failed (Horizon::Memory::%s)", __FUNCTION__);
#endif // DEBUG
		return 0u;
	}

	auto image_base = (std::uintptr_t)(base_addr);
	auto image_dos_hdr = (IMAGE_DOS_HEADER*)(image_base);

	if (image_dos_hdr->e_magic != IMAGE_DOS_SIGNATURE)
	{
#ifdef DEBUG
		Win32::Error("IMAGE_DOS_HEADER::e_magic is invalid (Horizon::Memory::%s)", __FUNCTION__);
#endif // DEBUG
		return 0u;
	}

	auto image_nt_hdrs = (IMAGE_NT_HEADERS*)(image_base + image_dos_hdr->e_lfanew);

	if (image_nt_hdrs->Signature != IMAGE_NT_SIGNATURE)
	{
#ifdef DEBUG
		Win32::Error("IMAGE_NT_HEADERS::Signature is invalid (Horizon::Memory::%s)", __FUNCTION__);
#endif // DEBUG
		return 0u;
	}

	auto scan_begin = (std::uint8_t*)(image_base);
	auto scan_end = (std::uint8_t*)(image_base + image_nt_hdrs->OptionalHeader.SizeOfImage);

	std::uint8_t* scan_result = nullptr;
	std::uint8_t* scan_data = (std::uint8_t*)(signature.c_str());

	for (auto current = scan_begin; current < scan_end; current++)
	{
		if (*(std::uint8_t*)scan_data == '\?' || *current == GET_BYTE(scan_data))
		{
			if (!scan_result)
				scan_result = current;

			if (!scan_data[2])
				return (std::uintptr_t)(scan_result);

			scan_data += (*(std::uint16_t*)scan_data == '\?\?' || *(std::uint8_t*)scan_data != '\?') ? 3 : 2;

			if (!*scan_data)
				return (std::uintptr_t)(scan_result);
		}
		else if (scan_result)
		{
			current = scan_result;
			scan_data = (std::uint8_t*)(signature.c_str());
			scan_result = nullptr;
		}
	}

#ifdef DEBUG
	Win32::Warning("Signature '%s' not found (Horizon::Memory::%s)", signature.c_str(), __FUNCTION__);
#endif // DEBUG

	return 0u;
}


//DWORD ScanString(DWORD dwAddress, DWORD dwEndAddress, const char* pszString)
//{
//	DWORD dwSize = strlen(pszString) + 2;
//
//	PBYTE pPattern = new BYTE[dwSize];
//	pPattern[0] = NULL;
//	strcpy_s((char*)pPattern + 1, dwSize - 1, pszString);
//
//	DWORD dwResult = Scan(dwAddress, pPattern);
//
//	delete[] pPattern;
//
//	if (!dwResult)
//		return NULL;
//
//	return dwResult + 1;
//}

}