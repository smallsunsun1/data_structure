#include <string_view>
#include <iostream>
#include <string>
#include "skip_list.h"

int main() {
    auto compare = [](std::string_view a, std::string_view b) {
        if (a < b) return -1;
        else if (a == b) return 0;
        else {
            return 1;
        }
    };
    sss::SkipList<std::string, decltype(compare)> list(compare);
    list.Insert("sss");
    std::cout << list.Contains("sss") << std::endl;
    return 0;
}
