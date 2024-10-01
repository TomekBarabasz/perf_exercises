#include <iostream>
#include <fstream>
#include <string_view>
#include <unordered_map>
#include <tuple>
#include <cctype>

#include <mem_mapped_file.h>

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

            }

        }
        ++data;
        --size;
    }
    return 0;
}

int main(int argc,const char** argv)
{
    const char* filename {argc > 1 ? argv[1] : "text_short.txt"};
    cout << "using file " << filename << endl;
    const auto cnt = count_distinct_words(filename);
    return 0;
}