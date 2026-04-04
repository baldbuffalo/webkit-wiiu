#pragma once
#include <memory>

// OwnPtr was removed from WTF — map to std::unique_ptr
template<typename T>
using OwnPtr = std::unique_ptr<T>;

template<typename T>
std::unique_ptr<T> adoptPtr(T* ptr) { return std::unique_ptr<T>(ptr); }
