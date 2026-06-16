#ifndef CAT_APPS_MESSAGE_CODEC_H__
#define CAT_APPS_MESSAGE_CODEC_H__

#include <algorithm>
#include <chrono>
#include <climits>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace cat_apps {

inline std::vector<uint8_t> ToUint32Bytes(uint32_t const raw)
{
  std::vector<uint8_t> data;
  for (std::size_t bitshift = (sizeof(uint32_t) - 1) * 8; bitshift > 0; bitshift -= 8) {
    data.push_back(static_cast<uint8_t>(raw >> bitshift));
  }
  data.push_back(static_cast<uint8_t>(raw));
  return data;
}

inline uint32_t FromUint32Bytes(std::vector<uint8_t> const & vector)
{
  auto value = std::numeric_limits<uint32_t>::min();
  for (std::size_t index = 0; index < std::min(sizeof(uint32_t), vector.size()); ++index) {
    value |= static_cast<uint32_t>(vector.at(index)) <<
      ((sizeof(uint32_t) - 1 - index) * CHAR_BIT);
  }
  return value;
}

inline std::string GetSystemTime()
{
  static constexpr char FORMAT[] = "%Y-%m-%d %X";
  using CLOCK = std::chrono::system_clock;
  auto in_time_t = CLOCK::to_time_t(CLOCK::now());
  std::stringstream ss;
  struct tm buf {};
#ifdef _WIN32
  gmtime_s(&buf, &in_time_t);
#else
  gmtime_r(&in_time_t, &buf);
#endif
  ss << std::put_time(&buf, FORMAT);
  return ss.str();
}

}  // namespace cat_apps

#endif  // CAT_APPS_MESSAGE_CODEC_H__
