#include "wired/tools/log.h"
#include "gtest/gtest.h"

void parse_args(int argc, char** argv);

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    parse_args(argc, argv);

    int sc = RUN_ALL_TESTS();
    return sc;
}

// leaves argc and argv unchanged
void parse_args(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-l" && i + 1 < argc) {
            std::string log_level = argv[++i];
            if (log_level == "disable") {
                LOG_LEVEL(LOG_DISABLE);
            } else if (log_level == "debug") {
                LOG_LEVEL(LOG_DEBUG);
            } else if (log_level == "info") {
                LOG_LEVEL(LOG_WARNING);
            } else if (log_level == "warning") {
                LOG_LEVEL(LOG_ERROR);
            } else if (log_level == "error") {
                LOG_LEVEL(LOG_ERROR);
            } else if (log_level == "critical") {
                LOG_LEVEL(LOG_CRITICAL);
            } else if (log_level == "all") {
                LOG_LEVEL(LOG_CRITICAL);
            } else {
                std::cerr << "Unknown log level: " << log_level << std::endl;
            }
        } else {
            std::cerr << "Additional options: [-l "
                         "log_level=[disable/debug/info/warning/error/critical/"
                         "all]]\n"
                      << std::endl;
            std::cerr << "Unknown option: " << arg << std::endl;
        }
    }
}