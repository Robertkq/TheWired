#ifndef kqTheWiredCommon
#define kqTheWiredCommon

#include <cstdint>
#include <windows.h>

static std::string host_ip = "your.ip.here";
static uint16_t host_port = 60000;

enum msgids : uint8_t
{
    ServerAcceptName,
    ServerRejectName,
    ClientProvideName,
    Whisper,
    MessageAll,
    Confetti
};

    // The scramble function should not be public
    // Make sure you keep it hidden and design a different one, perhaps harder to crack, than this one.
uint64_t scramble(uint64_t input)
{
    auto out = input ^ 0x5A9B6C2F0F011;
    out = (out & 0xF0F0F0F0F0F0F0) >> 4;
    return out;
}

void color(int k)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), k);
}

#endif