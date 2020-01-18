#include <functional>

class Deferrer {
    std::function<void()> fn;
public:
    Deferrer(std::function<void()> fn): fn(fn)
    {
    }

    ~Deferrer()
    {
        this->fn();
    }
};

#define DEFER_CONCAT2(a, b) a##b
#define DEFER_CONCAT(a, b) DEFER_CONCAT2(a, b)
#define DEFER_LOC DEFER_CONCAT(__FUNCTION__, __LINE__)
#define DEFER_VARNAME DEFER_CONCAT(__deferred__, DEFER_LOC) 

#define DEFER(...) Deferrer DEFER_VARNAME ( [&]() __VA_ARGS__ )
