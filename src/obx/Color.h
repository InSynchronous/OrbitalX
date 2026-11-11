#include <iostream>

namespace obx {
enum class Color {
    Reset = 0,
    Bold = 1,
    Dim = 2,
    Red = 31,
    Green = 32,
    Yellow = 33,
    Blue = 34,
    Magenta = 35,
    Cyan = 36,
    Gray = 90,
};

inline std::ostream &operator<<(std::ostream &os, Color c) {
    return os << "\033[" << static_cast<int>(c) << "m";
}
} // namespace obx
