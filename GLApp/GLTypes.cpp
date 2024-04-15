#include "GLTypes.h"
#include "GLTypes.h"

LockWrapper::LockWrapper(size_t& _lockCount) : lockCount(_lockCount) {
    owner = lockCount == 0;
    lockCount++;
}
LockWrapper::~LockWrapper() {
    if (lockCount == 0) throw Error("LockWrapper: Tried to unlock unlocked lock");
    lockCount--;
}
bool LockWrapper::IsLocked() {
    return lockCount>0;
}

bool LockWrapper::IsOwner()
{
    return owner;
}
