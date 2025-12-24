#ifndef PLATFORM_H
#define PLATFORM_H

// Arduino-compatible types and functions for simulator

#include <cstdint>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <SDL2/SDL.h>

// Arduino types
using uint8_t = std::uint8_t;
using uint16_t = std::uint16_t;
using uint32_t = std::uint32_t;
using int8_t = std::int8_t;
using int16_t = std::int16_t;
using int32_t = std::int32_t;

// Arduino timing (uses SDL)
inline uint32_t millis() {
  return SDL_GetTicks();
}

inline void delay(uint32_t ms) {
  SDL_Delay(ms);
}

// Arduino math
#ifndef TWO_PI
#define TWO_PI 6.283185307179586476925286766559f
#endif

#ifndef PI
#define PI 3.1415926535897932384626433832795f
#endif

template<typename T>
inline T constrain(T value, T low, T high) {
  return std::max(low, std::min(value, high));
}

template<typename T>
inline T map(T x, T in_min, T in_max, T out_min, T out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Serial mock for debug output
class SerialMock {
public:
  void begin(int) {}
  void print(const char* s) { std::cout << s; }
  void print(int v) { std::cout << v; }
  void print(float v, int decimals = 2) {
    std::cout << std::fixed;
    std::cout.precision(decimals);
    std::cout << v;
  }
  void print(unsigned int v) { std::cout << v; }
  void println(const char* s = "") { std::cout << s << std::endl; }
  void println(int v) { std::cout << v << std::endl; }
  void println(float v, int decimals = 2) {
    print(v, decimals);
    std::cout << std::endl;
  }
};

extern SerialMock Serial;

#endif // PLATFORM_H
