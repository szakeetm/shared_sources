#include "GLTypes.h"

LockWrapper::LockWrapper(bool& _flag) : flag(_flag) {
    if (flag) throw Error("LockWrapper: Trying to lock an already locked guard."); //Exposes hard to debug race condition
    flag = true;

}
LockWrapper::~LockWrapper() {
    flag = false;
};
bool LockWrapper::IsLocked() {
    return flag;
}
