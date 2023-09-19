#include "GLTypes.h"

LockWrapper::LockWrapper(bool& _flag) : flag(_flag) {
    flag = true;
}
LockWrapper::~LockWrapper() {
    flag = false;
};
bool LockWrapper::IsLocked() {
    return flag;
}
