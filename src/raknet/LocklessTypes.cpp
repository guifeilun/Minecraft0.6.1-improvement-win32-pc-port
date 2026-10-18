#include "LocklessTypes.h"

using namespace RakNet;

LocklessUint32_t::LocklessUint32_t()
{
	value=0;
}
LocklessUint32_t::LocklessUint32_t(uint32_t initial)
{
	value=initial;
}
uint32_t LocklessUint32_t::Increment(void)
{
	return (uint32_t) InterlockedIncrement(&value);
}
uint32_t LocklessUint32_t::Decrement(void)
{
	return (uint32_t) InterlockedDecrement(&value);
}
