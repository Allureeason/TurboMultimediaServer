#ifndef TIMING_WHEEL_H
#define TIMING_WHEEL_H

#include <memory>
#include <deque>
#include <unordered_set>
#include <vector>
#include <functional>

namespace tmms {
    namespace network {

        using EntryPtr = std::shared_ptr<void>;
        using WheelEntry = std::unordered_set<EntryPtr>;
        using Wheel = std::deque<WheelEntry>;
        using Wheels = std::vector<Wheel>;

        using Func = std::function<void()>;

        class CallbackEntry {
        public:
            CallbackEntry(const Func& cb) : cb_(cb) {}
            CallbackEntry(Func&& cb) : cb_(std::move(cb)) {}
            ~CallbackEntry() {
                cb_();
            }
        private:
            Func cb_;
        };

        using CallbackEntryPtr = std::shared_ptr<CallbackEntry>;

        const int kTimingMinute = 60;
        const int kTimingHour = 60 * 60;
        const int kTimingDay = 60 * 60 * 24;

        enum TimingType {
            kTimingTypeSecond = 0,
            kTimingTypeHour = 1,
            kTimingTypeMinute = 2,
            kTimingTypeDay = 3,

            kTimingTypeMax
        };

        class TimingWheel {
        public:
            TimingWheel();
            ~TimingWheel();

            void insertEntry(int delay, EntryPtr entry);
            void onTimer(int64_t now);
            void runAfter(int delay, const Func& f);
            void runAfter(int delay, Func&& f);
            void runEvery(int interval, const Func& f);
            void runEvery(int interval, Func&& f);

        private:
            void insertSecondEntry(int delay, EntryPtr entry);
            void insertMinuteEntry(int delay, EntryPtr entry);
            void insertHourEntry(int delay, EntryPtr entry);
            void insertDayEntry(int delay, EntryPtr entry);
            void popUp(Wheel& wheel);

        private:
            Wheels wheels_;
            int64_t last_ts_ { 0 };
            int64_t tick_ { 0 };
        };

    }

}

#endif // TIMING_WHELL_H