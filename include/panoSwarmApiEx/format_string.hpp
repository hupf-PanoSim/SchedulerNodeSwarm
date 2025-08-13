#pragma once

#include <string_view>

namespace panosim_bus {

    template <std::size_t N>
    struct format_string {
        template <typename T>
        constexpr format_string(const T(&str)[N + 1]) noexcept {
            if constexpr (std::is_same_v<T, char>) {
                for (std::size_t i{ 0 }; i < N; ++i) {
                    format_str[i] = static_cast<int8_t>(str[i]);
                    if ((i == (N - 1)) && (str[i] == 0)) {
                        break;
                    }
                    format_str_size++;
                }

                bool exist_body{ false };
                std::size_t last_comma_position{ 0 };
                header_item_cnt = 0;
                body_item_cnt = 0;

                for (std::size_t i{ 0 }; i < N - 1; ++i) {
                    if (exist_body) {
                        if (is_equal(format_str[i], '@')) {
                            body_fmt[body_item_cnt++] = format_str[i + 1];
                        }
                    }
                    else {
                        if (is_equal(format_str[i], '@')) {
                            if (is_equal(format_str[i + 1], '[')) {
                                std::size_t index{ 0 };
                                for (auto pos{ last_comma_position + 1 }; pos < i; ++pos) {
                                    width[index++] = format_str[pos];
                                }
                                width[index] = '\0';
                                exist_body = index > 0;
                                header_fmt[header_item_cnt++] = 'i';
                            }
                            else {
                                header_fmt[header_item_cnt++] = format_str[i + 1];
                                last_comma_position = i + 2;
                            }
                        }
                    }
                }
            }
            else if constexpr (std::is_same_v<T, wchar_t> || std::is_same_v<T, char>) {
                for (std::size_t i{ 0 }; i < N; ++i) {
                    format_str[i] = static_cast<char>(str[i]);
                    if ((i == (N - 1)) && (str[i] == 0)) {
                        break;
                    }
                    format_str_size++;
                }
            }
        }

        constexpr format_string(const format_string& str) noexcept {
            for (std::size_t i{ 0 }; i < N; ++i) {
                format_str[i] = str.format_str[i];
                header_fmt[i] = str.header_fmt[i];
                body_fmt[i] = str.body_fmt[i];
                width[i] = str.width[i];
            }
            format_str_size = str.format_str_size;
            header_item_cnt = str.header_item_cnt;
            body_item_cnt = str.body_item_cnt;
        }

        constexpr operator std::basic_string_view<char>() const noexcept {
            return std::basic_string_view<char>{format_str, size()};
        }
        constexpr const std::size_t header_item_count() const noexcept {
            return header_item_cnt;
        }
        constexpr const std::basic_string_view<char> header() const noexcept {
            return std::basic_string_view<char>(header_fmt, header_item_cnt);
        }
        constexpr const std::size_t body_item_count() const noexcept {
            return body_item_cnt;
        }
        constexpr const std::basic_string_view<char> body() const noexcept {
            return std::basic_string_view<char>(body_fmt, body_item_cnt);
        }
        constexpr const auto body_width() const noexcept {
            return std::stoi(std::basic_string<char>(width));
        }
        constexpr const std::size_t size() const noexcept {
            return N;
        }
        constexpr const char* begin() const noexcept {
            return format_str;
        }
        constexpr const char* end() const noexcept {
            return format_str + size();
        }
        constexpr char operator[](std::size_t i) const noexcept {
            return format_str[i];
        }

        template <std::size_t Size>
        constexpr bool is_same_as(const format_string<Size>& str) const noexcept {
            if (format_str_size != str.size()) {
                return false;
            }
            for (std::size_t i{ 0 }; i != format_str_size; ++i) {
                if (format_str[i] != str[i]) {
                    return false;
                }
            }
            return true;
        }

    protected:
        template <typename T>
        constexpr bool is_equal(T c1, char c2) const noexcept {
            return static_cast<char>(c1) == c2;
        }

    private:
        char format_str[N] = { '\0' };
        size_t format_str_size{ 0 };

        size_t header_item_cnt{ 0 };
        char header_fmt[N] = { '\0' };
        size_t body_item_cnt{ 0 };
        char body_fmt[N] = { '\0' };
        char width[N] = { '\0' };
    };

    template <>
    struct format_string<0> {
        static constexpr char empty[1] = { 0 };
    public:
        template <typename T>
        constexpr format_string(const T*) noexcept {}
        constexpr format_string(std::initializer_list<char>) noexcept {}
        constexpr format_string(const format_string&) noexcept {}
        constexpr operator std::basic_string_view<char>() const noexcept {
            return std::basic_string_view<char>{empty, 0};
        }
        constexpr const std::size_t header_item_count() const noexcept {
            return 0;
        }
        constexpr const std::basic_string_view<char> header() const noexcept {
            return std::basic_string_view<char>(empty);
        }
        constexpr const std::size_t body_item_count() const noexcept {
            return 0;
        }
        constexpr const std::basic_string_view<char> body() const noexcept {
            return std::basic_string_view<char>(empty);
        }
        constexpr const std::basic_string_view<char> body_width() const noexcept {
            return std::basic_string_view<char>(empty);
        }
        constexpr std::size_t size() const noexcept {
            return 0;
        }
        constexpr const char* begin() const noexcept {
            return empty;
        }
        constexpr const char* end() const noexcept {
            return empty + size();
        }
        constexpr char operator[]([[maybe_unused]] size_t i) const noexcept {
            return '\0';
        }
    };

    template <typename T, std::size_t N>
    format_string(const T(&)[N])->format_string<N - 1>;

    template <std::size_t N>
    format_string(format_string<N>)->format_string<N>;

} // namespace panosim_bus
