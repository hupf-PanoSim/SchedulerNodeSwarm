#pragma once

#include "format_string.hpp"

#ifdef min
#undef min
#endif

#define DEFINE_BUS_FORMAT(FormatName, FormatString)             \
struct FormatName {};                                           \
template<>                                                      \
struct panosim_bus::BusFormatTrait<FormatName> {                \
    static constexpr const char(&str())[sizeof(FormatString)] { \
        return FormatString;                                    \
    }                                                           \
};

#define BusFormat(s)                                                        \
[]() {                                                                      \
    struct bus_format_ : panosim_bus::bus_format {                          \
        static constexpr const char (&str())[sizeof(s)]  { return s; }      \
        static constexpr std::size_t size() { return std::size(str()) - 1; }\
        static constexpr auto at(std::size_t i) { return str()[i]; };       \
    };                                                                      \
    return bus_format_{};                                                   \
}()

#define SET_FORMAT_CHAR(c, s, std_type)                 \
template <> struct FormatChar<c> {                      \
    static constexpr std::size_t size() { return s; }   \
    using StdType = std_type;                           \
}

namespace panosim_bus {

    template <typename T>
    struct bus_data {
        constexpr bus_data(T* b) : ptr(b), size(0) {}
        std::size_t size;
        T* ptr;
    };

    namespace accessor {
        template <std::size_t N>
        struct unsinged_integer;
        template <>
        struct unsinged_integer<1> {
            using type = std::uint8_t;
        };
        template <>
        struct unsinged_integer<4> {
            using type = std::uint32_t;
        };
        template <>
        struct unsinged_integer<8> {
            using type = std::uint64_t;
        };
        template <typename T>
        using integer_type = std::conditional_t<
            std::is_signed_v<T>,
            typename std::make_signed_t<typename unsinged_integer<sizeof(T)>::type>,
            typename unsinged_integer<sizeof(T)>::type
        >;

        template <typename T>
        constexpr T read_(const bus_data<const char>& data);
        template <>
        constexpr std::uint8_t read_(const bus_data<const char>& data) {
            return static_cast<std::uint8_t>(data.ptr[0] & '\xFF');
        }
        template <>
        constexpr std::uint32_t read_(const bus_data<const char>& data) {
            std::uint32_t v = 0;
            v += static_cast<std::uint32_t>(static_cast<std::uint8_t>(data.ptr[0] & 0xFF));
            v += static_cast<std::uint32_t>(static_cast<std::uint8_t>(data.ptr[1] & 0xFF) << 8);
            v += static_cast<std::uint32_t>(static_cast<std::uint8_t>(data.ptr[2] & 0xFF) << 16);
            v += static_cast<std::uint32_t>(static_cast<std::uint8_t>(data.ptr[3] & 0xFF) << 24);
            return v;
        }
        template <>
        constexpr std::int8_t read_(const bus_data<const char>& data) {
            std::uint8_t v = read_<unsigned char>(data);
            return static_cast<std::int8_t>(v - 0xFFULL - 1);
        }
        template <>
        constexpr std::int32_t read_(const bus_data<const char>& data) {
            std::uint32_t v = read_<std::uint32_t>(data);
            return static_cast<std::int32_t>(v - 0xFFFFFFFFULL - 1);
        }
        template <>
        inline double read_(const bus_data<const char>& data) {
            return *(double*)data.ptr;
        }
        template <>
        inline float read_(const bus_data<const char>& data) {
            return *(float*)data.ptr;
        }

        template <typename T>
        constexpr auto read(const bus_data<const char>& data) {
            if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
                return read_<integer_type<T>>(data);
            }
            else {
                return read_<T>(data);
            }
        }

        constexpr void write_(bus_data<char>& data, std::uint8_t v) {
            data.ptr[0] = char(v & 0xFF);
        }
        constexpr void write_(bus_data<char>& data, std::uint32_t v) {
            data.ptr[0] = char(v & 0xFF);
            data.ptr[1] = char((v >> 8) & 0xFF);
            data.ptr[2] = char((v >> 16) & 0xFF);
            data.ptr[3] = char((v >> 24) & 0xFF);
        }
        constexpr void write_(bus_data<char>& data, std::int8_t v) {
            std::uint8_t b = 0;
            if (v > 0) {
                b = v;
            }
            else {
                b = 0xFF + v + 1;
            }
            write_(data, b);
        }
        constexpr void write_(bus_data<char>& data, std::int32_t v) {
            std::uint32_t b = 0;
            if (v > 0) {
                b = v;
            }
            else {
                b = static_cast<std::uint32_t>(0xFFFFFFFFULL + static_cast<std::uint32_t>(v) + 1ull);
            }
            write_(data, b);
        }
        inline void write_(bus_data<char>& data, double v) {
            *(double*)data.ptr = v;
        }
        inline void write_(bus_data<char>& data, float v) {
            *(float*)data.ptr = v;
        }
        template <typename T>
        constexpr void write(bus_data<char>& data, T v) {
            if constexpr (std::is_integral_v<T>) {
                return write_(data, static_cast<integer_type<T>>(v));
            }
            else {
                return write_(data, v);
            }
        }
    } // namespace accessor

    template<typename T>
    struct BusFormatTrait;

    struct bus_format {};

    template <char c>
    struct FormatChar {
        static constexpr std::size_t size() { return 0; }
    };

    template <char c>
    using StdType = typename FormatChar<c>::StdType;

    SET_FORMAT_CHAR('b', 1, std::int8_t);
    SET_FORMAT_CHAR('i', 4, std::int32_t);
    SET_FORMAT_CHAR('f', 4, float);
    SET_FORMAT_CHAR('d', 8, double);

    struct FormatType {
        char formatChar;
        std::size_t size;
    };

    template <std::size_t Idx, std::size_t Size>
    constexpr char getItem(char(&formats)[Size]);
    template <std::size_t Idx, typename Fmt>
    constexpr FormatType getHeaderTypeOfItem(Fmt);
    template <std::size_t Idx, typename Fmt>
    constexpr FormatType getBodyTypeOfItem(Fmt);
    template <std::size_t Idx, typename Fmt>
    constexpr std::size_t getHeaderBinaryOffset(Fmt);
    template <std::size_t Idx, typename Fmt>
    constexpr std::size_t getBodyBinaryOffset(Fmt);
    template <typename T>
    constexpr T readItem(const char* bus, std::size_t size);

    template <typename Fmt, std::size_t... Seq1, std::size_t... Seq2>
    constexpr auto read_header_(std::index_sequence<Seq1...>, std::index_sequence<Seq2...>, const char* bus) {
        constexpr FormatType formats[] = { getHeaderTypeOfItem<Seq1>(Fmt{})... };
        using Types = std::tuple<
            typename panosim_bus::StdType<formats[Seq2].formatChar>
            ...
        >;
        constexpr std::size_t offsets[] = { getHeaderBinaryOffset<Seq2>(Fmt{})... };
        return std::make_tuple(
            readItem<std::tuple_element_t<Seq2, Types>>(bus + offsets[Seq2], formats[Seq2].size)
            ...
        );
    }
    template <typename Fmt>
    constexpr auto read_header(Fmt, const char* bus) {
        constexpr std::size_t header_item_count = format_string(Fmt::str()).header_item_count();
        static_assert(header_item_count > 0, "format no header");
        return read_header_<Fmt>(
            std::make_index_sequence<Fmt::size()>(),
            std::make_index_sequence<header_item_count>(),
            bus);
    }

    template <typename Fmt>
    constexpr std::size_t calc_header_size(Fmt);
    template <typename Fmt>
    constexpr std::size_t calc_header_size(Fmt) {
        constexpr auto numItems = format_string(Fmt::str()).header_item_count();
        constexpr auto lastItem = getHeaderTypeOfItem<numItems - 1>(Fmt{});
        return getHeaderBinaryOffset<numItems - 1>(Fmt{}) + lastItem.size;
    }

    template <typename Fmt>
    constexpr std::size_t calc_body_size(Fmt);
    template <typename Fmt>
    constexpr std::size_t calc_body_size(Fmt) {
        constexpr auto numItems = format_string(Fmt::str()).body_item_count();
        constexpr auto lastItem = getBodyTypeOfItem<numItems - 1>(Fmt{});
        return getBodyBinaryOffset<numItems - 1>(Fmt{}) + lastItem.size;
    }

    template <typename Type>
    constexpr int writeItem(char* bus, FormatType format, Type v) {
        if constexpr (std::is_same_v<Type, std::string_view>) {
            v = std::string_view(v.data(), std::min(v.size(), format.size));
        }
        else {
            (void)format;
        }
        bus_data<char> item(bus);
        accessor::write(item, v);
        return 0;
    }

    template <typename Type, typename T>
    constexpr Type convert(const T& v) {
        if constexpr (
            std::is_array_v<T>
            && std::is_same_v<std::remove_extent_t<T>, char>
            && std::is_same_v<Type, std::string_view>) {
            return Type(std::data(v), std::size(v));
        }
        else {
            return static_cast<Type>(v);
        }
    }

    template <typename Fmt, std::size_t... Seq1, std::size_t... Seq2, typename... Args>
    constexpr void write_header_(char* bus_header, std::index_sequence<Seq1...>, std::index_sequence<Seq2...>, Args&&... args) {
        static_assert(sizeof...(args) == sizeof...(Seq2), "write header parameter count error");
        constexpr FormatType formats[] = { panosim_bus::getHeaderTypeOfItem<Seq1>(Fmt{})... };
        using Types = std::tuple<typename panosim_bus::StdType<formats[Seq2].formatChar>...>;
        constexpr std::size_t offsets[] = { getHeaderBinaryOffset<Seq2>(Fmt{})... };
        Types t = std::make_tuple(convert<std::tuple_element_t<Seq2, Types>>(std::forward<Args>(args))...);
        using ArrayType = std::array<char, panosim_bus::calc_header_size(Fmt{}) > ;
        int _[] = { 0, writeItem(bus_header + offsets[Seq2], formats[Seq2], std::get<Seq2>(t))... };
        (void)_;
    }

    template <typename Fmt, typename... Args>
    constexpr void write_header(Fmt, char* bus_header, Args&&... args) {
        constexpr std::size_t header_item_count = format_string(Fmt::str()).header_item_count();
        write_header_<Fmt>(
            bus_header,
            std::make_index_sequence<Fmt::size()>(),
            std::make_index_sequence<header_item_count>(),
            std::forward<Args>(args)...);
    }

    template <typename Fmt, std::size_t... Seq1, std::size_t... Seq2, typename... Args>
    constexpr void write_body_(char* bus_body, std::index_sequence<Seq1...>, std::index_sequence<Seq2...>, Args&&... args) {
        static_assert(sizeof...(args) == sizeof...(Seq2), "write body parameter count error");
        constexpr FormatType formats[] = { panosim_bus::getBodyTypeOfItem<Seq1>(Fmt{})... };
        using Types = std::tuple<typename panosim_bus::StdType<formats[Seq2].formatChar>...>;
        constexpr std::size_t offsets[] = { getBodyBinaryOffset<Seq2>(Fmt{})... };
        Types t = std::make_tuple(convert<std::tuple_element_t<Seq2, Types>>(std::forward<Args>(args))...);
        using ArrayType = std::array<char, panosim_bus::calc_body_size(Fmt{}) > ;
        int _[] = { 0, writeItem(bus_body + offsets[Seq2], formats[Seq2], std::get<Seq2>(t))... };
        (void)_;
    }

    template <typename Fmt, typename... Args>
    constexpr void write_body(Fmt, char* bus_body, Args&&... args) {
        constexpr std::size_t body_item_count = format_string(Fmt::str()).body_item_count();
        write_body_<Fmt>(
            bus_body,
            std::make_index_sequence<Fmt::size()>(),
            std::make_index_sequence<body_item_count>(),
            std::forward<Args>(args)...);
    }

    template <typename Fmt, std::size_t... Seq1, std::size_t... Seq2>
    constexpr auto read_body_(std::index_sequence<Seq1...>, std::index_sequence<Seq2...>, const char* bus) {
        constexpr FormatType formats[] = { getBodyTypeOfItem<Seq1>(Fmt{})... };
        using Types = std::tuple<
            typename panosim_bus::StdType<formats[Seq2].formatChar>
            ...
        >;
        constexpr std::size_t offsets[] = { getBodyBinaryOffset<Seq2>(Fmt{})... };
        return std::make_tuple(
            readItem<std::tuple_element_t<Seq2, Types>>(bus + offsets[Seq2], formats[Seq2].size)
            ...
        );
    }
    template <typename Fmt>
    constexpr auto read_body(Fmt, const char* bus) {
        constexpr std::size_t body_item_count = format_string(Fmt::str()).body_item_count();
        static_assert(body_item_count > 0, "format no body");
        return read_body_<Fmt>(
            std::make_index_sequence<Fmt::size()>(),
            std::make_index_sequence<body_item_count>(),
            bus);
    }

    template <typename T>
    constexpr T readItem(const char* bus, std::size_t size) {
        bus_data<const char> item(bus);
        item.size = size;
        return accessor::read<T>(item);
    }

    constexpr bool nextIsType(char c) {
        return c == '@';
    }

    constexpr bool isBodyFlag(char c) {
        return c == '[';
    }

    template <char c>
    constexpr std::size_t getSize() {
        return FormatChar<c>::size();
    }

    template <std::size_t Idx, std::size_t Size>
    constexpr char getItem(char(&formats)[Size]) {
        std::size_t currentItem = 0;
        for (std::size_t i = 0; i < Size; i++) {
            if (currentItem == Idx) {
                return formats[i];
            }
            currentItem++;
        }

        return { 0 };
    }

    template <std::size_t Idx, typename Fmt, std::size_t... Seq>
    constexpr char getHeaderTypeOfItem_(std::index_sequence<Seq...>) {
        constexpr char fomratString[] = { Fmt::at(Seq)... };
        char types[Fmt::size()]{};
        std::size_t currentType = 0;
        bool nextTypeFlag = false;
        for (std::size_t i = 0; i < sizeof...(Seq); ++i) {
            if (isBodyFlag(fomratString[i])) {
                types[currentType] = 'i';
                break;
            }
            if (nextIsType(fomratString[i])) {
                nextTypeFlag = true;
            }
            else {
                if (nextTypeFlag) {
                    types[currentType] = fomratString[i];
                    currentType++;
                    nextTypeFlag = false;
                }
            }
        }
        return types[Idx];
    }
    template <std::size_t Idx, typename Fmt>
    constexpr FormatType getHeaderTypeOfItem(Fmt) {
        constexpr char format = getHeaderTypeOfItem_<Idx, Fmt>(std::make_index_sequence<Fmt::size()>());
        constexpr FormatType sizedFormat = { format, getSize<format>() };
        return sizedFormat;
    }

    template <std::size_t Idx, typename Fmt, std::size_t... Seq>
    constexpr char getBodyTypeOfItem_(std::index_sequence<Seq...>) {
        constexpr char fomratString[] = { Fmt::at(Seq)... };
        char types[Fmt::size()]{};
        std::size_t currentType = 0;
        bool findSplitFlag = false;
        bool nextTypeFlag = false;
        for (std::size_t i = 0; i < sizeof...(Seq); ++i) {
            if (findSplitFlag) {
                if (nextIsType(fomratString[i])) {
                    nextTypeFlag = true;
                }
                else {
                    if (nextTypeFlag) {
                        types[currentType] = fomratString[i];
                        currentType++;
                        nextTypeFlag = false;
                    }
                }
            }
            else {
                if (isBodyFlag(fomratString[i])) {
                    findSplitFlag = true;
                }
            }
        }
        return types[Idx];
    }
    template <std::size_t Idx, typename Fmt>
    constexpr FormatType getBodyTypeOfItem(Fmt) {
        constexpr char format = getBodyTypeOfItem_<Idx, Fmt>(std::make_index_sequence<Fmt::size()>());
        constexpr FormatType sizedFormat = { format, getSize<format>() };
        return sizedFormat;
    }

    template <typename Fmt, std::size_t... Seq>
    constexpr std::size_t getHeaderBinaryOffset_(Fmt, std::index_sequence<Seq...>) {
        constexpr FormatType itemTypes[] = { getHeaderTypeOfItem<Seq>(Fmt{})... };
        std::size_t size = 0;
        for (std::size_t i = 0; i < sizeof...(Seq) - 1; i++) {
            size += itemTypes[i].size;
        }
        return size;
    }
    template <std::size_t Idx, typename Fmt>
    constexpr std::size_t getHeaderBinaryOffset(Fmt) {
        return getHeaderBinaryOffset_(Fmt{}, std::make_index_sequence<Idx + 1>());
    }

    template <typename Fmt, std::size_t... Seq>
    constexpr std::size_t getBodyBinaryOffset_(Fmt, std::index_sequence<Seq...>) {
        constexpr FormatType itemTypes[] = { getBodyTypeOfItem<Seq>(Fmt{})... };
        std::size_t size = 0;
        for (std::size_t i = 0; i < sizeof...(Seq) - 1; i++) {
            size += itemTypes[i].size;
        }
        return size;
    }
    template <std::size_t Idx, typename Fmt>
    constexpr std::size_t getBodyBinaryOffset(Fmt) {
        return getBodyBinaryOffset_(Fmt{}, std::make_index_sequence<Idx + 1>());
    }
} // namespace panosim_bus
