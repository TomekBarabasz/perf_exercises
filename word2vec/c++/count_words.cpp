#include <iostream>
#include <fstream>
#include <string_view>
#include <unordered_map>
#include <map>
#include <tuple>
#include <cctype>
#include <vector>

#include <mem_mapped_file.h>
#include <perf.h>

using namespace std;
using namespace utils;

// comile:
// g++ count_words.cpp -I../../common/c++/include -O3 -o count_words

template<typename KeyType, typename ValueType>
using map_type = std::unordered_map<KeyType, ValueType>;
int count_distinct_words_mmap_stringview(const char* filename, PerfCounter& pc)
{
    map_type<uint64_t,string_view> words;
    MemMappedFile file(filename);
    
    const char *data = file.data;
    auto size = file.size;

    auto isletter = [](char c) -> bool { return isalpha(c) || c == '\''; };
    auto add_word = [&](uint64_t hash_value, const char* data, size_t cnt) __attribute__((always_inline)) {
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
    uint64_t tot_add_word_ticks = 0;
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
                auto t0 = pc.timestamp();
                add_word(hash_value, word_start, data-word_start);
                tot_add_word_ticks += pc.timestamp() - t0;
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
    cout << "tot_add_word time " << pc.dt_to_string(tot_add_word_ticks) << endl;
    return words.size();
}

int count_distinct_words_mmap(const char* filename, PerfCounter& pc)
{
    map_type<uint64_t,string> words;
    MemMappedFile file(filename);
    
    const char *data = file.data;
    auto size = file.size;

    auto isletter = [](char c) -> bool { return isalpha(c) || c == '\''; };
    auto add_word = [&](uint64_t hash_value, const char* data, size_t cnt) __attribute__((always_inline)) {
        string s {data,cnt};
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
    uint64_t tot_add_word_ticks = 0;
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
                auto t0 = pc.timestamp();
                add_word(hash_value, word_start, data-word_start);
                tot_add_word_ticks += pc.timestamp() - t0;
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
    cout << "tot_add_word time " << pc.dt_to_string(tot_add_word_ticks) << endl;
    return words.size();
}

int count_distinct_words(const char* filename, PerfCounter& pc)
{
    map_type<uint64_t,string_view> words;
    ifstream input(filename);
    
    input.seekg(0,std::ios::end);
    // read by chunks
    size_t size = input.tellg();
    input.seekg(0,std::ios::beg);
    std::vector<char> buffer(size);
    const auto t0 = pc.timestamp();
    input.read(buffer.data(), size);
    cout << "data read time " << pc.dt_to_string(pc.timestamp()-t0) << endl;
    if (input.gcount() != size) {
        throw std::runtime_error("read error");
    }

    char *data = buffer.data();

    auto isletter = [](char c) -> bool { return isalpha(c) || c == '\''; };
    auto add_word = [&](uint64_t hash_value, const char* data, size_t cnt) __attribute__((always_inline)) {
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
    uint64_t tot_add_word_ticks = 0;

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
                auto t0 = pc.timestamp();
                add_word(hash_value, word_start, data-word_start);
                tot_add_word_ticks += pc.timestamp() - t0;
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
        cout << "add last word" << endl;
        add_word(hash_value, word_start, data-word_start);
    }
    cout << "tot_add_word time " << pc.dt_to_string(tot_add_word_ticks) << endl;
    return words.size();
}

int main(int argc,const char** argv)
{
    PerfCounter pc;

    const char* filename {argc > 1 ? argv[1] : "text_short.txt"};
    const unsigned int test_sel = argc > 2 ? std::stoi(argv[2]) : (unsigned)-1;

    cout << "using file " << filename << endl;
    if (test_sel & 1)
    {
        cout << "count_distinct_words_mmap [using stringview]" << endl;
        auto t0 = pc.timestamp();
        auto cnt = count_distinct_words_mmap_stringview(filename,pc);
        auto t1 = pc.timestamp();
        cout << "exec time " << pc.dt_to_string(t1-t0) << endl;
        cout << "distinct words in a text : " << cnt << endl;
        cout << endl;
    }
    
    if (test_sel & 2)
    {
        cout << "count_distinct_words_mmap" << endl;
        auto t0 = pc.timestamp();
        auto cnt = count_distinct_words_mmap(filename,pc);
        auto t1 = pc.timestamp();
        cout << "exec time " << pc.dt_to_string(t1-t0) << endl;
        cout << "distinct words in a text : " << cnt << endl;
        cout << endl;
    }

    if (test_sel & 4)
    {
        cout << "count_distinct_words [normal read,string_view]" << endl;
        auto t0 = pc.timestamp();
        auto cnt = count_distinct_words(filename,pc);
        auto t1 = pc.timestamp();
        cout << "exec time " << pc.dt_to_string(t1-t0) << endl;
        cout << "distinct words in a text : " << cnt << endl;
    }

    return 0;
}