#include <iostream>
#include <fstream>
#include <string_view>
#include <unordered_map>
#include <tuple>
#include <cctype>

#include <mem_mapped_file.h>
#include <perf.h>

using namespace std;
using namespace utils;

// comile:
// g++ count_words.cpp -I../../common/c++/include -O3 -o count_words


int count_distinct_words(const char* filename)
{
    unordered_map<uint64_t,string_view> words;
    MemMappedFile file(filename);

    const char *data = file.data;
    auto size = file.size;

    auto isletter = [](char c) -> bool { return isalpha(c) || c == '\''; };
    auto add_word = [&](uint64_t hash_value, const char* data, size_t cnt) {
        string_view s {data,cnt};
        auto it = words.find(hash_value);
        if (it == words.end()) {
            words.emplace(hash_value,s);
        }else{
            const bool equal = it->second == s;
            if (!equal) {
                throw std::runtime_error("hash collision");
            }
        }
    };
    bool prev_is_letter = false;
    uint64_t hash_value = 5381;
    const char *word_start = data;
    while (size > 0)
    {
        char c = *data;
        if (!prev_is_letter)
        {
            if (isletter(c)) {
                prev_is_letter = true;
                hash_value = (hash_value * 33) + c;
                word_start = data;
            }
        }
        else
        {
            if(!isletter(c)) {
                prev_is_letter = false;
                add_word(hash_value, word_start, data-word_start);
                hash_value = 5381;
            }else{
                hash_value = (hash_value * 33) + c;
            }
        }
        ++data;
        --size;
    }
    if (prev_is_letter) {
        //add last word
        add_word(hash_value, word_start, data-word_start);
    }
    return words.size();
}

int main(int argc,const char** argv)
{
    PerfCounter pc;

    const char* filename {argc > 1 ? argv[1] : "text_short.txt"};
    cout << "using file " << filename << endl;
    auto t0 = pc.timestamp();
    const auto cnt = count_distinct_words(filename);
    auto t1 = pc.timestamp();
    cout << "exec time " << pc.dt_to_string(t1-t0) << endl;
    cout << "distinct words in a text : " << cnt << endl;
    return 0;
}