#pragma once

#include <cstdint>
namespace fetch {
namespace meta {

template <uint64_t N>
struct Log2
{
  enum
  {
    value = 1 + Log2<(N >> 1)>::value
  };
};
template <>
struct Log2<1>
{
  enum
  {
    value = 0
  };
};
}  // namespace meta
}  // namespace fetch