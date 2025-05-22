#include "TimingWheel.h"
#include "network/base/Network.h"

using namespace tmms::network;

TimingWheel::TimingWheel()
    :wheels_(kTimingTypeMax) {
    wheels_[kTimingTypeSecond].resize(60);
    wheels_[kTimingTypeMinute].resize(60);
    wheels_[kTimingTypeHour].resize(24);
    wheels_[kTimingTypeDay].resize(30);
}

TimingWheel::~TimingWheel() {

}

void TimingWheel::insertSecondEntry(int delay, EntryPtr entry) {
    wheels_[kTimingTypeSecond][delay - 1].insert(entry);
}

void TimingWheel::insertMinuteEntry(int delay, EntryPtr entry) {
    int minute = delay / kTimingMinute;
    int second = delay % kTimingMinute;
    
    CallbackEntryPtr newEntry = std::make_shared<CallbackEntry>([this, second, entry] {
        insertEntry(second, entry);
    });

    wheels_[kTimingTypeMinute][minute - 1].insert(entry);
}

void TimingWheel::insertHourEntry(int delay, EntryPtr entry) {
    int hour = delay / kTimingHour;
    int second = delay % kTimingHour;
    
    CallbackEntryPtr newEntry = std::make_shared<CallbackEntry>([this, second, entry] {
        insertEntry(second, entry);
    });
    wheels_[kTimingTypeHour][hour - 1].insert(entry);
}

void TimingWheel::insertDayEntry(int delay, EntryPtr entry) {
    int day = delay / kTimingDay;
    int second = delay % kTimingDay;
    
    CallbackEntryPtr newEntry = std::make_shared<CallbackEntry>([this, second, entry] {
        insertEntry(second, entry);
    });
    wheels_[kTimingTypeDay][day - 1].insert(entry);
}

void TimingWheel::insertEntry(int delay, EntryPtr entry) {
    if (delay <= 0) {
        entry.reset();
    } else if (delay < kTimingMinute) {
        insertSecondEntry(delay, entry);
    } else if (delay < kTimingHour) {
        insertMinuteEntry(delay, entry);
    } else if (delay < kTimingDay) {
        insertHourEntry(delay, entry);
    } else {
        int day = delay / kTimingDay;
        if (day > 30) {
            NETLOG_ERROR << "not support timing > 30 days";
            return;
        }

        insertDayEntry(delay, entry);
    }
}

void TimingWheel::onTimer(int64_t now) {
    if (last_ts_ == 0) {
        last_ts_ = now;
    }

    if (now - last_ts_ < 1000) {
        return;
    }

    last_ts_ = now;
    ++tick_;

    // 触发秒
    popUp(wheels_[kTimingTypeSecond]);

    if (tick_ % kTimingMinute) {
        popUp(wheels_[kTimingTypeMinute]);
    } else if (tick_ % kTimingHour) {
        popUp(wheels_[kTimingTypeHour]);
    } else if (tick_ % kTimingDay) {
        popUp(wheels_[kTimingTypeDay]);
    }
}

void TimingWheel::popUp(Wheel& wheel) {
    WheelEntry wheelEntry;
    wheel.front().swap(wheelEntry);
    wheel.pop_front();
    wheel.push_back(WheelEntry());
}

void TimingWheel::runAfter(int delay, const Func& f) {
    CallbackEntryPtr entry = std::make_shared<CallbackEntry>(f);
    insertEntry(delay, entry);
}

void TimingWheel::runAfter(int delay, Func&& f) {
    CallbackEntryPtr entry = std::make_shared<CallbackEntry>(std::move(f));
    insertEntry(delay, entry);
}

void TimingWheel::runEvery(int interval, const Func& f) {
    CallbackEntryPtr entry = std::make_shared<CallbackEntry>([this, interval, f] {
        f();
        runEvery(interval, f);
    });
    insertEntry(interval, entry);
}

void TimingWheel::runEvery(int interval, Func&& f) {
    CallbackEntryPtr entry = std::make_shared<CallbackEntry>([this, interval, f] {
        f();
        runEvery(interval, f);
    });
    insertEntry(interval, entry);
}