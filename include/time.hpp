#pragma once
#include <chrono>

namespace Time {
    using namespace std::chrono;
    namespace internal {
        // timestamp storage struct
        struct Timestamps {
            double delta;
            double total;
            // timestamp of init()
            high_resolution_clock::time_point start;
            // timestamp of previous frame
            high_resolution_clock::time_point previous;
        };
        // for internal use only
        auto static get() -> Timestamps& {
            static Timestamps timer;
            return timer;
        }
    };
    // call once at the very start of the application
    void static init() {
        internal::Timestamps& timestamps = internal::get();
        timestamps.start = high_resolution_clock::now();
        timestamps.previous = timestamps.start;
        timestamps.delta = 0.0;
        timestamps.total = 0.0;
    }
    // call once per frame
    void static update() {
        internal::Timestamps& timestamps = internal::get();
        // get time since last frame
        auto current = high_resolution_clock::now();
        auto delta = duration_cast<std::chrono::microseconds>(current - timestamps.previous);
        timestamps.delta = (double)delta.count() / 1000000.0; // convert to seconds
        // update timestamp to reflect new frame
        timestamps.previous = current;

        // update total time
        auto total = duration_cast<std::chrono::microseconds>(current - timestamps.start);
        timestamps.total = (double)total.count() / 1000000.0; // convert to seconds
    }
    // time elapsed (seconds) since the last frame
    auto static get_delta() -> double {
        return internal::get().delta;
    }
    // time elapsed (seconds) since the application start (init)
    auto static get_total() -> double {
        return internal::get().total;
    }
};