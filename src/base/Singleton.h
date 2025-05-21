#ifndef SINGLETON_H
#define SINGLETON_H

#include <pthread.h>

#include "NonCopyable.h"

namespace tmms {
    namespace base {
        template <typename T>
        class Singleton : public NonCopyable {
        public:
            static T*& getInstance() {
                pthread_once(&once_, &init);
                return instance_;
            }
        private:
            Singleton() = delete;
            ~Singleton() = delete;

            static void init() {
                if (!instance_) {
                    instance_ = new T();
                }
            }
        private:
            static pthread_once_t once_;
            static T* instance_;
        };

        template <typename T>
        T* Singleton<T>::instance_ = nullptr;

        template <typename T>
        pthread_once_t Singleton<T>::once_ = PTHREAD_ONCE_INIT;
    }
}
#endif