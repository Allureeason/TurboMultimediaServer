#include "Packet.h"
#include <cstring>
#include <cstdlib>

using namespace tmms::mm;

PacketPtr Packet::newPacket(int32_t size) {
    auto block_size = size + sizeof(Packet);
    Packet* packet = (Packet*)malloc(block_size);
    memset((void*)packet, 0x00, block_size);
    packet->index_ = -1;
    packet->type_ = kPacketTypeUnknown;
    packet->capacity_ = size;
    packet->ext_.reset();

    return PacketPtr(packet, [](Packet* p) {
        delete[] (char*)p;
    });
}