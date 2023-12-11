#ifndef MEMORY_LITERALS_H
#define MEMORY_LITERALS_H

namespace sot::memory_literals {

constexpr unsigned long long operator""_B(const unsigned long long bytes) {
  return bytes;
}

constexpr unsigned long long operator""_KiB(const unsigned long long kibs) {
  return 1024 * kibs;
}

constexpr unsigned long long operator""_MiB(const unsigned long long mibs) {
  return mibs * 1024 * 1_KiB;
}

constexpr unsigned long long operator""_GiB(const unsigned long long gibs) {
  return gibs * 1024 * 1_MiB;
}

}  // namespace sot::memory_literals

#endif  // MEMORY_LITERALS_H
