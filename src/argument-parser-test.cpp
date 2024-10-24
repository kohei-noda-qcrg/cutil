#include "argument-parser.hpp"
#include "split.hpp"

struct Pos {
    int x;
    int y;
};

namespace args {
template <>
auto from_string<Pos>(const char* str) -> std::optional<Pos> {
    auto elms = split(str, ",");
    if(elms.size() != 2) {
        return std::nullopt;
    }
    auto x = from_chars<int>(elms[0]);
    auto y = from_chars<int>(elms[1]);
    if(!x || !y) {
        return std::nullopt;
    }
    return Pos{*x, *y};
}

template <>
auto to_string<Pos>(const Pos& data) -> std::string {
    return build_string(data.x, ",", data.y);
}
} // namespace args

namespace test {
auto parse(auto& parser, const char* const str) -> bool {
    auto strs = std::vector<std::string>();
    auto argv = std::vector<const char*>();
    for(const auto elm : split(str, " ")) {
        strs.emplace_back(elm);
    }
    for(const auto& str : strs) {
        argv.emplace_back(str.data());
    }
    return parser.parse(argv.size(), argv.data());
}

auto all_type_test() -> void {
    auto pint    = int();
    auto kint    = int();
    auto pdouble = double();
    auto kdouble = double();
    auto pstr    = (const char*)(nullptr);
    auto kstr    = (const char*)(nullptr);
    auto pflag   = bool();
    auto kflag   = bool();
    auto ppos    = Pos();
    auto kpos    = Pos();

    auto parser = args::Parser<Pos>();
    parser.arg(&pint, "INT", "positional interger value");
    parser.kwarg(&kint, {"-n", "--num"}, "INT", "keyword interger value");
    parser.arg(&pdouble, "FLOAT", "positional float value");
    parser.kwarg(&kdouble, {"-f", "--float"}, "FLOAT", "keyword float value");
    parser.arg(&pstr, "STR", "positional string value");
    parser.kwarg(&kstr, {"-s", "--string"}, "STR", "keyword string value");
    parser.arg(&pflag, "BOOL", "positional boolean value");
    parser.kwarg(&kflag, {"-b", "--bool"}, "BOOL", "keyword boolean value");
    parser.arg(&ppos, "X,Y", "positional custom value");
    parser.kwarg(&kpos, {"-p", "--pos"}, "FLOAT", "keyword custom value");
    print("usage: test ", parser.get_help());
    print("parse: ", parse(parser, "test -n 2 -f 2.0 -s hello -b true -p 100,200 1 1.0 world false 300,400") ? "ok" : "error");
    if(pint != 1 || kint != 2 ||
       pdouble != 1.0 || kdouble != 2.0 ||
       std::string_view(kstr) != "hello" || std::string_view(pstr) != "world" ||
       pflag != false || kflag != true ||
       ppos.x != 300 || ppos.y != 400 || kpos.x != 100 || kpos.y != 200) {
        print("args: error");
    } else {
        print("args: ok");
    }
    print("pint: ", pint);
    print("kint: ", kint);
    print("pdouble: ", pdouble);
    print("kdouble: ", kdouble);
    print("pstr: ", pstr);
    print("kstr: ", kstr);
    print("pflag: ", pflag);
    print("kflag: ", kflag);
    print("ppos: ", ppos.x, ",", ppos.y);
    print("kpos: ", kpos.x, ",", kpos.y);
}

auto flag_test() -> void {
    auto flag   = bool();
    auto invert = bool();

    auto parser = args::Parser<>();
    parser.kwflag(&flag, {"-1"}, "normal");
    parser.kwflag(&invert, {"-2"}, "invert", {.invert_flag_value = true});
    print("usage: test ", parser.get_help());
    print("parse: ", parse(parser, "test -1 -2") ? "ok" : "error");
    print("args:", flag && !invert ? "ok" : "error");
}

auto state_test() -> void {
    auto num = 8086;
    // default value
    {
        auto parser = args::Parser<>();
        parser.kwarg(&num, {"-n"}, "INT", "number", {.state = args::State::DefaultValue});
        print("usage: test ", parser.get_help());
        print("parse: ", parse(parser, "test") ? "ok" : "error");
        print("args:", num == 8086 ? "ok" : "error");
    }
    // initialized
    {
        auto parser = args::Parser<>();
        parser.kwarg(&num, {"-n"}, "INT", "number", {.state = args::State::Initialized});
        print("usage: test ", parser.get_help());
        print("parse: ", parse(parser, "test") ? "ok" : "error");
        print("args:", num == 8086 ? "ok" : "error");
    }
    // required
    {
        auto parser = args::Parser<>();
        parser.kwarg(&num, {"-n"}, "INT", "number", {.state = args::State::Uninitialized});
        print("usage: test ", parser.get_help());
        print("parse: ", !parse(parser, "test") ? "ok" : "error");
        print("parse: ", parse(parser, "test -n 1") ? "ok" : "error");
        print("args:", num == 1 ? "ok" : "error");
    }
}

auto minus_int_test() -> void {
    auto i8  = int8_t();
    auto u8  = uint8_t();
    auto i64 = ssize_t();
    auto u64 = size_t();

    auto parser = args::Parser<int8_t, uint8_t, ssize_t, size_t>();
    parser.arg(&i8, "INT", "i8");
    parser.arg(&u8, "INT", "u8");
    parser.arg(&i64, "INT", "i64");
    parser.arg(&u64, "INT", "u64");
    print("usage: test ", parser.get_help());
    print("parse: ", parse(parser, "test -1 2 -3 4") ? "ok" : "error");
    print("args:", i8 == -1 && u8 == 2 && i64 == -3 && u64 == 4 ? "ok" : "error");
}

auto out_of_range_test() -> void {
    auto i8 = int8_t();
    auto u8 = uint8_t();

    auto parser = args::Parser<int8_t, uint8_t, ssize_t, size_t>();
    parser.arg(&i8, "INT", "i8");
    parser.arg(&u8, "INT", "u8");
    print("parse: ", !parse(parser, "test 1 -2") ? "ok" : "error");
}

auto no_error_check_test() -> void {
    auto required = bool();
    auto help     = false;

    auto parser = args::Parser<>();
    parser.arg(&required, "BOOL", "required");
    parser.kwflag(&help, {"-h", "--help"}, "help", {.no_error_check = true});
    print("parse: ", parse(parser, "test -h") ? "ok" : "error");
}
} // namespace test

auto main() -> int {
    test::all_type_test();
    test::flag_test();
    test::state_test();
    test::minus_int_test();
    test::out_of_range_test();
    test::no_error_check_test();
    return 0;
}
