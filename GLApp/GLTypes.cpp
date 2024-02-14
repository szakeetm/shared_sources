#include "GLTypes.h"
#include "GLTypes.h"

LockWrapper::LockWrapper(size_t& _flag) : flag(_flag) {
    owner = flag == 0;
    flag++;
}
LockWrapper::~LockWrapper() {
    if (flag == 0) throw std::exception("LockWrapper: Tried to unlock unlocked lock");
    flag--;
};
bool LockWrapper::IsLocked() {
    return flag>0;
}

bool LockWrapper::IsOwner()
{
    return owner;
}
