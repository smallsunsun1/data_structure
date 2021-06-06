#ifndef CONCEPT_
#define CONCEPT_

#include <concepts>
#include <string_view>

template <typename Comparator>
concept is_string_view_comparator = requires (Comparator compare, std::string_view s1, std::string_view s2) {
    {compare(s1, s2)} -> std::same_as<bool>;
};

#endif /* CONCEPT_ */
