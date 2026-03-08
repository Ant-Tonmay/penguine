#pragma once

#include "vm/value.h"

namespace vm {

bool checkAccess(ClassObject* owner, ClassObject* context, AccessModifier access);

}  // namespace vm
