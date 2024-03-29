#pragma once

#include "auto.hpp"

namespace Horizon::Memory
{

template<typename T>
T VCall( const void* instance, const std::uint32_t index )
{
	return ( T )( ( *( void*** )instance )[index] );
}

}