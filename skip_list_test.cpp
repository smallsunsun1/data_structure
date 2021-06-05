#include <string_view>
#include "skip_list.h"

int main() {
    auto compare = [](std::string_view a, std::string_view b) {
        return a < b;
    };
    sss::SkipList<int, decltype(compare)> list(compare);
    return 0;
}