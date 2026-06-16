// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ObfuscatedKey.h
// Description: Provides compile-time key obfuscation (maps unique integers to less intuitive unique integers).

#ifndef ObfuscatedKey_H
#define ObfuscatedKey_H

#include <chrono>

namespace middleware
{

// indexShuffleLUT<4>() -> {1 2 3 4} -> return {2 4 1 3}
// not wholly constexpr, could yield different results for different cpp files due to using __TIME__ for random numbers
template <size_t L>
constexpr auto indexShuffleLUT()
{
	// https://gcc.gnu.org/onlinedocs/cpp/Standard-Predefined-Macros.html
	// Mersenne twister isn't constexpr, just cycle numbers in clock time
	char seeds[] = {__TIME__[7], __TIME__[6], __TIME__[4], __TIME__[3]};
	int seed     = 0;
	std::array<size_t, L> indices{};
	std::array<bool, L> used{};
	used.fill(false);
	for(size_t i = 0; i < L; i++)
	{
		auto random = seeds[seed];
		seed        = (seed + 1) % (sizeof(seeds) / sizeof(seeds[0]));
		random      = random % (L - i);
		size_t j;
		decltype(random) skipped = 0;
		for(j = 0; skipped < random; j++)
		{
			if(!used[j])
			{
				skipped++;
			}
		}
		while(used[j])
		{
			j++;
		}
		indices[i] = j;
		used[j]    = true;
	}
	return indices;
}

// shuffle bytes of uniqueSeed according to an indexShuffleLUT
// obfuscated keys generated in different cpp files could have collisions for non-matching inputs
template <typename E>
inline constexpr E bytesObfuscatedKey(E uniqueSeed)
{
	auto const lut = indexShuffleLUT<sizeof(E) / sizeof(uint8_t)>();
	E ret          = 0;
	for(size_t i = 0; i < lut.size(); i++)
	{
		ret |= ((uniqueSeed >> (8 * lut[i])) & 0xFF) << (8 * i);
	}
	return ret;
}

// shuffle bits of uniqueSeed according to an indexShuffleLUT
// obfuscated keys generated in different cpp files could have collisions for non-matching inputs
template <typename E>
inline constexpr E bitsObfuscatedKey(E uniqueSeed)
{
	auto const lut = indexShuffleLUT<8 * sizeof(E) / sizeof(uint8_t)>();
	E ret          = 0;
	for(size_t i = 0; i < lut.size(); i++)
	{
		ret |= ((uniqueSeed >> lut[i]) & 0x1) << i;
	}
	return ret;
}

}  // namespace middleware

#endif  // ObfuscatedKey_H
