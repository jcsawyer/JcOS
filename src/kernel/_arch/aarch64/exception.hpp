#pragma once

#include "../../exception.hpp"

namespace Exception {
PrivilegeLevel current_privilege_level(const char **el_string);
} // namespace Exception
