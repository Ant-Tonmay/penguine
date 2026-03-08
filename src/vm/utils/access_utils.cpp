#include "vm/utils/access_utils.h"

namespace vm {

bool checkAccess(ClassObject* owner, ClassObject* context, AccessModifier access) {
    if (access == AccessModifier::PUBLIC) return true;

    if (access == AccessModifier::PRIVATE) {
        return context == owner;
    }

    if (access == AccessModifier::PROTECTED) {
        if (!context) return false;

        ClassObject* current = context;
        while (current != nullptr) {
            if (current == owner) return true;
            current = current->parent;
        }

        current = owner;
        while (current != nullptr) {
            if (current == context) return true;
            current = current->parent;
        }
        return false;
    }

    return false;
}

}  // namespace vm
