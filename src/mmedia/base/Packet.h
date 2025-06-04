#ifndef MEDIA_PACKET_H
#define MEDIA_PACKET_H

#include <cstdint>
#include <memory>

namespace tmms {
    namespace mm {
        enum {
            kPacketTypeVideo = 1,
            kPacketTypeAudio = 2,
            kPacketTypeMeta = 4,
            kPacketTypeMeta3 = 8,
            kPacketTypeKeyFrame = 16,
            kPacketTypeIDR = 32,

            kPacketTypeUnknown = 255,
        };

        class Packet;
        using PacketPtr = std::shared_ptr<Packet>;

#pragma pack(push, 1)
        class Packet {
        public:
            Packet(int32_t size) : size_(size) {}
            ~Packet() {}

            static PacketPtr newPacket(int32_t size);

            bool isVideo() const {
                return (type_ & kPacketTypeVideo);
            }
            bool isAudio() const {
                return type_ == kPacketTypeAudio;
            }
            bool isMeta() const {
                return type_ == kPacketTypeMeta;
            }
            bool isMeta3() const {
                return type_ == kPacketTypeMeta3;
            }
            bool isKeyFrame() const {
                return ((type_ & kPacketTypeKeyFrame) && (type_ & kPacketTypeVideo));
            }
            bool isIDR() const {
                return ((type_ & kPacketTypeIDR) && (type_ & kPacketTypeKeyFrame));
            }

            inline int32_t packetSize() const {
                return size_ + sizeof(Packet);
            }

            inline int space() const {
                return capacity_ - size_;
            }

            inline void setPacketSize(int32_t len) {
                size_ = len;
            }

            inline void updatePacketSize(int32_t len) {
                size_ += len;
            }

            inline int32_t type() const {
                return type_;
            }

            inline void setType(int32_t type) {
                type_ = type;
            }

            inline uint32_t size() const {
                return size_;
            }

            inline void setSize(uint32_t size) {
                size_ = size;
            }

            inline int32_t index() const {
                return index_;
            }

            inline void setIndex(int32_t index) {
                index_ = index;
            }

            inline uint64_t timestamp() const {
                return timestamp_;
            }

            inline void setTimestamp(uint64_t timestamp) {
                timestamp_ = timestamp;
            }

            inline uint32_t capacity() const {
                return capacity_;
            }

            inline void setCapacity(uint32_t capacity) {
                capacity_ = capacity;
            }

            template<typename T>
            inline std::shared_ptr<T> ext() const {
                return std::static_pointer_cast<T>(ext_);
            }

            inline void setExt(std::shared_ptr<void> ext) {
                ext_ = ext;
            }

            inline char* data() {
                return (char*)this + sizeof(Packet);
            }

        private:
            int32_t type_ { kPacketTypeUnknown }; // 类型
            uint32_t size_ { 0 }; // 大小
            int32_t index_ { -1 }; // 索引
            uint64_t timestamp_ { 0 }; // 时间戳
            uint32_t capacity_ { 0 }; // 容量
            std::shared_ptr<void> ext_ { nullptr }; // 扩展数据
        };
#pragma pack(pop)

    }
}

#endif