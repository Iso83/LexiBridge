#include "TestAssert.h"

#include <translator/service.h>

class ServiceTests {
public:
    static int repeatedLifetime() {
        // Every iteration must register and remove the same logger names.
        for (int iteration = 0; iteration < 8; ++iteration) {
            marian::bergamot::BlockingService service(marian::bergamot::BlockingService::Config{});
        }
        return 0;
    }

    static int exceptionUnwinding() {
        try {
            marian::bergamot::BlockingService service(marian::bergamot::BlockingService::Config{});
            throw std::runtime_error("lifetime test");
        } catch (const std::runtime_error& error) {
            CPPTEST_ASSERT(std::string(error.what()) == "lifetime test");
        }
        return repeatedLifetime();
    }
};

int main() {
    try {
        CPPTEST_RUN(ServiceTests::repeatedLifetime);
        CPPTEST_RUN(ServiceTests::exceptionUnwinding);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
