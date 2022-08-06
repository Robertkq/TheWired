#ifndef kqTheWiredClient
#define kqTheWiredClient

#include <kqNet.h>
#include "..\TheWiredCommon\common.h"

struct client : kq::client_interface<msgids>
{
    client(uint64_t(*scrambleFunc)(uint64_t)) : kq::client_interface<msgids>(scrambleFunc) {}

    std::string name;
    std::string lastWhisper;

};

#endif