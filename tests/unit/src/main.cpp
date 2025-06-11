#include "wired/tools/log.h"
#include "gtest/gtest.h"

void parse_args(int argc, char** argv);

int main(int argc, char** argv) {
    WIRED_LOG_LEVEL(wired::LOG_DEBUG);
    ::testing::InitGoogleTest(&argc, argv);
    parse_args(argc, argv);

    int status_code = RUN_ALL_TESTS();
    std::cerr << "Tests exited with code: " << status_code << std::endl;
    return status_code;
}

// leaves argc and argv unchanged
void parse_args(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-l" && i + 1 < argc) {
            std::string log_level = argv[++i];
            if (log_level == "disable") {
                WIRED_LOG_LEVEL(wired::LOG_DISABLE);
            } else if (log_level == "debug") {
                WIRED_LOG_LEVEL(wired::LOG_DEBUG);
            } else if (log_level == "info") {
                WIRED_LOG_LEVEL(wired::LOG_INFO);
            } else if (log_level == "warning") {
                WIRED_LOG_LEVEL(wired::LOG_WARNING);
            } else if (log_level == "error") {
                WIRED_LOG_LEVEL(wired::LOG_ERROR);
            } else if (log_level == "critical" || log_level == "all") {
                WIRED_LOG_LEVEL(wired::LOG_CRITICAL);
            } else {
                std::cerr << "Unknown log level: " << log_level << std::endl;
            }
        } else {
            std::cerr << "Additional options: [-l "
                         "log_level=[disable/debug/info/warning/error/critical/"
                         "all]]\n"
                      << std::endl;
            std::cerr << "Unknown option: " << arg << std::endl;
            exit(1);
        }
    }
}