#ifndef MiddlewareTestDefines_H
#define MiddlewareTestDefines_H

#include <stddef.h>
#include <tuple>

#define IGNORE_ANY(...) EXPECT_CALL(__VA_ARGS__).Times(::testing::AnyNumber())

// remove after a feature release past 1.16.0
// googletest/4a00a24fff3cf82254de382437bf840cab1d3993
namespace testing
{
template <size_t k, typename Ptr>
struct SaveArgByMoveAction
{
	Ptr& pointer;
	template <typename... Args>
	void operator()(Args const&... args) const
	{
		pointer = std::get<k>(std::tie(args...));
	}
};
template <size_t k, typename Ptr>
auto SaveArgByMove(Ptr& pointer)
{
	return SaveArgByMoveAction<k, Ptr>{pointer};
}
}  // namespace testing

#endif  // MiddlewareTestDefines_H
