#ifndef TTIME_H
#define TTIME_H

#include <string>
#include <cstdint>

namespace tmms {

    namespace base {

        class TTime {
        public:
            static uint64_t now();
            static uint64_t nowMs();
            static void now(int& year, int& month, int& day, int& hour, int& minute, int& second);
            static std::string ISONow();
            static std::string IOSNowMs();

        };
    }

}

#endif
