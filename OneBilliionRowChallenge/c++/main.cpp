#include <fstream>
#include <iostream>
#include <cstdint>
#include <immintrin.h>
#include <map>
#include <unordered_map>
#include <string>
#include <string_view>
#include <sstream>
#include <utility>
#include <unistd.h>
#include <charconv>
#include <perf.h>

using utils::PerfCounter;

struct Record {
    uint32_t cnt;
    float sum;
    float min;
    float max;
};

void test_load_1(std::ifstream & input)
{
    using Database_t = std::map<std::string,Record>;
    Database_t db;

    std::string line;
    std::string name;
    float value;

    size_t cnt_loaded {0};
    while (std::getline(input, line)) {
        if (line[0] == '#') continue;
        const auto isep = line.find(';');
        if (isep != std::string::npos) {
            name = line.substr(0,isep);
            value = std::stof(line.substr(isep+1));

            auto it = db.find(name);
            if (it == db.end()) {
                db.emplace(name,Record{1,value,value,value});
            } else {
                auto & r = it->second;
                ++r.cnt;
                r.sum += value;
                if (value < r.min)
                    r.min = value;
                if (value > r.max)
                    r.max = value;
            }
            ++cnt_loaded;
        }
    }
}

void test_load_2(std::ifstream & input)
{
    using Database_t = std::map<std::string,Record,std::less<>>;
    Database_t db;

    std::string line;
    size_t cnt_loaded {0};

    while (std::getline(input, line)) {
        if (line[0] == '#') continue;
        const auto isep = line.find(';');
        if (isep != std::string::npos) {
            std::string_view name {line.data(),isep};
            std::string_view svalue {line.data()+isep+1, line.size()-isep};
            float value;
            const auto [ptr, ec] = std::from_chars(svalue.data(), svalue.data() + svalue.size(), value);
            if (ec == std::errc::invalid_argument) {
                std::cout << "invalid line " << line << " svalue " << svalue << std::endl;
            }

            auto it = db.find(name);
            if (it == db.end()) {
                db.emplace(name,Record{1,value,value,value});
            } else {
                auto & r = it->second;
                ++r.cnt;
                r.sum += value;
                if (value < r.min)
                    r.min = value;
                if (value > r.max)
                    r.max = value;
            }
            ++cnt_loaded;
        }
    }
}

void test_load_3(std::ifstream & input)
{
    using Database_t = std::unordered_map<std::string,Record>;
    Database_t db;

    std::string line;
    size_t cnt_loaded {0};

    while (std::getline(input, line)) {
        if (line[0] == '#') continue;
        const auto isep = line.find(';');
        if (isep != std::string::npos) {
            std::string name = line.substr(0,isep);
            std::string_view svalue {line.data()+isep+1, line.size()-isep};
            float value;
            const auto [ptr, ec] = std::from_chars(svalue.data(), svalue.data() + svalue.size(), value);
            if (ec == std::errc::invalid_argument) {
                std::cout << "invalid line " << line << " svalue " << svalue << std::endl;
            }

            auto it = db.find(name);
            if (it == db.end()) {
                db.emplace(name,Record{1,value,value,value});
            } else {
                auto & r = it->second;
                ++r.cnt;
                r.sum += value;
                if (value < r.min)
                    r.min = value;
                if (value > r.max)
                    r.max = value;
            }
            ++cnt_loaded;
        }
    }
}

void test_load_4(std::ifstream & input)
{
    struct StringHash {
        using is_transparent = void;  // Allows heterogeneous lookup

        // Hash function for std::string
        std::size_t operator()(std::string const& s) const noexcept {
            return std::hash<std::string>{}(s);
        }

        // Hash function for std::string_view
        std::size_t operator()(std::string_view s) const noexcept {
            return std::hash<std::string_view>{}(s);
        }
    };

    // Custom equality comparator that works with std::string and std::string_view
    struct StringEqual {
        using is_transparent = void;  // Allows heterogeneous lookup

        bool operator()(std::string const& lhs, std::string const& rhs) const noexcept {
            return lhs == rhs;
        }

        bool operator()(std::string_view lhs, std::string_view rhs) const noexcept {
            return lhs == rhs;
        }

        bool operator()(std::string const& lhs, std::string_view rhs) const noexcept {
            return lhs == rhs;
        }

        bool operator()(std::string_view lhs, std::string const& rhs) const noexcept {
            return lhs == rhs;
        }
    };
    using Database_t = std::unordered_map<std::string_view,Record,StringHash,StringEqual>;
    Database_t db;

    std::string line;
    size_t cnt_loaded {0};

    while (std::getline(input, line)) {
        if (line[0] == '#') continue;
        const auto isep = line.find(';');
        if (isep != std::string::npos) {
            std::string_view name {line.data(),isep};
            std::string_view svalue {line.data()+isep+1, line.size()-isep};
            float value;
            const auto [ptr, ec] = std::from_chars(svalue.data(), svalue.data() + svalue.size(), value);
            if (ec == std::errc::invalid_argument) {
                std::cout << "invalid line " << line << " svalue " << svalue << std::endl;
            }

            auto it = db.find(name);
            if (it == db.end()) {
                db.emplace(name,Record{1,value,value,value});
            } else {
                auto & r = it->second;
                ++r.cnt;
                r.sum += value;
                if (value < r.min)
                    r.min = value;
                if (value > r.max)
                    r.max = value;
            }
            ++cnt_loaded;
        }
    }
}

void test()
{
    std::string line {"name;3.1245926"};
    const auto isep = line.find(';');
    auto name = line.substr(0,isep);
    auto value = std::stof(line.substr(isep+1));
    std::cout << "name=" << name << " value=" << value << std::endl;

    PerfCounter pc;
    std::cout << "ticks_per_sec " << pc.ticks_per_sec << std::endl;
    uint64_t ticks_per_sec = static_cast<uint64_t>( pc.ticks_per_sec );
    std::cout << "pretty_print 1s   :" << pc.dt_to_string(ticks_per_sec) << std::endl;
    std::cout << "pretty_print 1/2s :" << pc.dt_to_string(ticks_per_sec/2 + 1) << std::endl;
    std::cout << "pretty_print 1ms  :" << pc.dt_to_string(ticks_per_sec/1000+1) << std::endl;
    std::cout << "pretty_print 1us  :" << pc.dt_to_string(ticks_per_sec/1000000+1) << std::endl;
}

void test1()
{
    std::string line {"Changhua;46.64617051711254"};
    const auto isep = line.find(';');
    std::string_view name {line.data(),isep};
    std::string_view svalue {line.data()+isep+1, line.size()-isep};
    float value;
    const auto [ptr, ec] = std::from_chars(svalue.data(), svalue.data() + svalue.size(), value);
    if (ec == std::errc::invalid_argument) {
        std::cout << "invalid line " << line << std::endl;
    }
}

int main(int argc, const char** argv)
{
    test1();
    PerfCounter pc;
    
    if (argc < 2) {
        std::cout << "provide input filename" << std::endl;
        return 1;
    }
    std::ifstream input {argv[1]};

    if (!input.is_open()) {
        std::cout << "input file opening error" << std::endl;
        return 1;
    }
    const auto impl_idx = argc > 2 ? atoi(argv[2]) : 1;
    std::cout << "using implementation idx " << impl_idx << std::endl;
    if (1 == impl_idx) {
        std::cout << "using basic implementation [1] std::map + std::string" << std::endl;
    } else if (2 == impl_idx ) {
        std::cout << "using basic implementation [2] std::map + std::string_view" << std::endl;
    } else if (3 == impl_idx ) {
        std::cout << "using basic implementation [3] std::unordered_map + std::string" << std::endl;
    } else if (4 == impl_idx ) {
        std::cout << "using basic implementation [4] std::unordered_map + std::string_view" << std::endl;
    }
    const auto repeat_cnt = argc > 3 ? atoi(argv[3]) : 1;
    for (int i=0;i<repeat_cnt;++i)
    {
        auto tm_start = pc.timestamp();
        if (1 == impl_idx) {
            test_load_1(input);
        } else if (2 == impl_idx ) {
            test_load_2(input);
        } else if (3 == impl_idx ) {
            test_load_3(input);
        } else if (4 == impl_idx ) {
            test_load_4(input);
        }
        auto dt = pc.timestamp() - tm_start;
        std::cout << "iter[" << i << "] " << pc.dt_to_string(dt) << std::endl;
        input.clear();
        input.seekg(0,std::ios_base::beg);
    }
    return 0;
}