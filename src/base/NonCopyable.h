#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

namespace tmms {
    namespace base {
        class NonCopyable {
        public:
            NonCopyable() = default;
            ~NonCopyable() = default;
            NonCopyable(const NonCopyable&) = delete;
            NonCopyable& operator=(const NonCopyable&) = delete;
            NonCopyable(NonCopyable&&) = delete;
        };
    } // namespace base
} // namespace tmms

#endif // NONCOPYABLE_H