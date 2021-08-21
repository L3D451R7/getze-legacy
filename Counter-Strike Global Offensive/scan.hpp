#pragma once

#include "auto.hpp"

namespace Horizon::Memory
{

std::uintptr_t Scan( const std::string& image_name, const std::string& signature );

}