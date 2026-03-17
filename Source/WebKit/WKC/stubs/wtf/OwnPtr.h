#pragma once
// OwnPtr was removed from WTF — alias to std::unique_ptr
#include <memory>
namespace WTF {
template<typename T>
using OwnPtr = std::unique_ptr<T>;
template<typename T>
using PassOwnPtr = std::unique_ptr<T>;
template<typename T, typename... Args>
std::unique_ptr<T> adoptPtr(T* ptr) { return std::unique_ptr<T>(ptr); }
}
using WTF::OwnPtr;
using WTF::PassOwnPtr;
