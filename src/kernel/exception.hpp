#pragma once

namespace Exception {
enum PrivilegeLevel { Unknown, User, Kernel, Hypervisor };
PrivilegeLevel current_privilege_level(const char **el_string);
void handlingInit();

} // namespace Exception
