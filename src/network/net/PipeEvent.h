#ifndef PIPE_EVENT_H
#define PIPE_EVENT_H

#include "Event.h"

namespace tmms {
    namespace network {

        class PipeEvent : public Event {
            public:
                PipeEvent(EventLoop* loop);
                ~PipeEvent();

                virtual void onRead() override;

                void write(const char* data, size_t len);

            private:
                int write_fd_ { -1 };
        };
    }
}
#endif
