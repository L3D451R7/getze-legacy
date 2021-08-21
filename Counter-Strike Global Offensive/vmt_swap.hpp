#pragma once

#include "auto.hpp"

namespace Horizon::Memory
{

	class VFTableHook {
		VFTableHook(const VFTableHook&) = delete;
	public:

		template<class Type>
		static Type HookManual(uintptr_t *instance, int offset, Type hook)
		{
			DWORD Dummy;
			Type fnOld = (Type)instance[offset];
			VirtualProtect((void*)(instance + offset * 0x4), 0x4, PAGE_EXECUTE_READWRITE, &Dummy);
			instance[offset] = (uintptr_t)hook;
			VirtualProtect((void*)(instance + offset * 0x4), 0x4, Dummy, &Dummy);
			return fnOld;
		}

	private:

		static int Unprotect(void *region)
		{
			MEMORY_BASIC_INFORMATION mbi;
			VirtualQuery((LPCVOID)region, &mbi, sizeof(mbi));
			VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &mbi.Protect);
			return mbi.Protect;
		}

		static void Protect(void *region, int protection)
		{
			MEMORY_BASIC_INFORMATION mbi;
			VirtualQuery((LPCVOID)region, &mbi, sizeof(mbi));
			VirtualProtect(mbi.BaseAddress, mbi.RegionSize, protection, &mbi.Protect);
		}
	};

class VmtSwap
{
public:
	using Shared = std::shared_ptr<VmtSwap>;
public:
	VmtSwap( const void* instance = nullptr )
	{
		if( instance )
			Create( instance );
	}

public:
	~VmtSwap()
	{
		Destroy();
	}

public:
	bool Create( const void* instance )
	{
		m_ppInstance = ( std::uintptr_t** )instance;
		m_pRestore = *m_ppInstance;

		if( !m_pRestore )
			return false;

		while( m_pRestore[m_nSize] )
			m_nSize++;

		if( !m_nSize )
			return false;

		m_pReplace = std::make_unique<std::uintptr_t[]>( m_nSize );
		memcpy( m_pReplace.get(), m_pRestore, m_nSize * sizeof( std::uintptr_t ) );

		Replace();
		hooked = true;

		return true;
	}

	void Destroy()
	{
		Restore();

		m_ppInstance = nullptr;
		m_pRestore = nullptr;
		m_pReplace.reset();
		m_nSize = 0u;
		hooked = false;
	}

	void Replace()
	{
		if( !m_ppInstance || !m_pReplace )
			return;

		*m_ppInstance = m_pReplace.get();
	}

	void Restore()
	{
		if( !m_ppInstance || !m_pRestore )
			return;

		*m_ppInstance = m_pRestore;
	}

	DWORD Hook( const void* hooked, const std::uint32_t index )
	{
		if( !m_pReplace )
			return 0;

		m_pReplace[index] = ( std::uintptr_t )hooked;

		return m_pRestore[index];
	}

	template<typename T>
	T VCall( const std::size_t index )
	{
		if (!this)
			return nullptr;

		return ( T )( m_pRestore[index] );
	}

	bool hooked = false;
//private:
	std::uintptr_t** m_ppInstance = nullptr;
	std::uintptr_t* m_pRestore = nullptr;
	std::unique_ptr<std::uintptr_t[]> m_pReplace = nullptr;
	std::uint32_t m_nSize = 0u;
};

}