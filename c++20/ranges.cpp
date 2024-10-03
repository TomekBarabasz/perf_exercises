#include <iostream>
#include <vector>
#include <ranges>
#include <numeric>
#include <string>
#include <cctype> //for toupper

using namespace std;

// compile with:
// clang-20 ranges.cpp -lstdc++ -std=c++23 -o ranges

template <typename T>
std::string join(T& container, const char sep[])
{
    using value_type = std::ranges::range_value_t<T>;
    if (std::ranges::begin(container) == std::ranges::end(container)) {
        return {};
    }
    return std::accumulate(begin(container), end(container), std::string{},
        [&](const std::string& acc, value_type b) {
            return !acc.empty() ? acc + sep + std::to_string(b) : std::to_string(b);
        }
    );
}

// pipelined join for ranges
struct join_impl
{
    std::string separator;
    join_impl(const char sep[]) : separator(sep) {}

    template <std::ranges::range Range>
    std::string operator()(Range&& r) const {
        return std::accumulate(
            std::ranges::begin(r), std::ranges::end(r), std::string{},
            [this](const std::string& acc, const auto& elem) {
                return acc.empty() ? std::to_string(elem) : acc + separator + std::to_string(elem);
            });
    }
};

// Overload `operator|` to make `join` work in a range pipeline
template <std::ranges::range Range>
std::string operator|(Range&& r, const join_impl& j) {
    return j(std::forward<Range>(r));
}

auto join(const char sep[]) { return join_impl(sep);}

void test1()
{
    vector<int> vec{1, 2, 3, 4, 5, 6};
    auto rvec = std::views::reverse(vec);

    cout << "hello" << endl;
    cout << "vec =" << join(vec,"..") << endl;
    cout << "rvec=" << join(rvec,"_") << endl;

#if 0
    // compile error:
    // no matching function for call to object of type 'const _Transform'
    auto zip = std::views::transform(vec,rvec,[](int a,int b){
        return std::pair(a,b);
    });
    for (auto [x, y] : zip) {
        std::cout << '(' << x << ',' << y << ')' << std::endl;
    }
#endif
    auto pow = [](int a) { return a*a;};
    auto vpow1 = std::views::transform(vec,pow);
    cout << "vpow1 = " << join(vpow1,",") << endl;
    auto vpow2 = vec | std::views::transform(pow);
    cout << "vpow2 = " << join(vpow2,",") << endl;

    auto even = [](int a) {return a%2==0;};
    auto vpow_even = vec | std::views::transform(pow) | std::views::filter(even);
    cout << "vpow even = " << join(vpow_even,",") << endl;
    auto s = vec 
             | std::views::transform(pow) 
             | std::views::filter(even)
             | join(",");
    cout << "vpow even = " << s << endl;
    //std::ranges::shuffle(dt, std::mt19937(std::random_device()()));
}

void test2()
{
    vector<string> words {"one","two","three"};
    auto str_toupper = [](const string& s) {
        string result;
        transform(s.begin(), s.end(), std::back_inserter(result), 
                           [](unsigned char c) { 
                               return toupper(c); 
                           });
        return result;
    };
    auto tw = words | std::views::transform( str_toupper );
    for(const auto & w : tw) {
        cout << w << endl;
    }
}

void test3()
{
    auto good = "1234567890";
    auto sep1 = std::ranges::find(std::string_view(good), '0');
    std::cout << *sep1 << "\n";auto bad = 1234567890;
    auto sep2 = std::ranges::find(std::to_string(bad), '0');
    // std::cout << *sep2 << "\n"; COMPILE ERROR: 
    // input collection is temporary i.e. return value of std::to_string
    // and after returning find sep2 in an invalid pointer
    // the return type is std::ranges::dangling!
    // COOL!
}

void test4()
{
    std::vector<int> dt = { 1, 2, 3, 4, 5, 6, 7, 8, 9};
#if 0
    // not compiling! fix me!
    std::vector<int> result;std::ranges::transform(dt, 
                        dt | std::views::reverse,
                        std::back_inserter(result),
                        std::minus<void>(),
                        [](int v) { return v*v; },
                        [](int v) { return v*v; }
                        );
    std::ranges::copy(result, std::ostream_iterator<int>(std::cout, ","));
#endif
}

int main(int argc, const char** argv)
{
    cout << "-------- test1 --------" << endl;
    test1();
    cout << "-------- test2 --------" << endl;    
    test2();
    cout << "-------- test3 --------" << endl;    
    test3();
    cout << "-------- test4 --------" << endl;    
    test4();
    return 0;
}