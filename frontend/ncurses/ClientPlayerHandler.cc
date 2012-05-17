#include "ClientPlayerHandler.h"

#include "Protocol.h"
using namespace Protocol;

#define SEND_PACKET(stream)  \
{\
    int payloadSize = (BufObj(NULL) stream).Offset();   \
    char* buf = fnGetPayloadBuffer(                     \
            Protocol::Op::Group::Player, payloadSize);  \
    BufObj(buf) stream;                                 \
}\
    fnSendOut()

ClientPlayerHandler::ClientPlayerHandler()
{
}

ClientPlayerHandler::~ClientPlayerHandler()
{
}

void ClientPlayerHandler::Handle(char* buf, int len)
{
    using namespace Protocol;

    if (len < (int)sizeof(char))
        return;

    char op = Op::Player::None;

    BufObj bufObj(buf);
    bufObj >> op;
    switch (op) {
        case Op::Player::ItemProgress:
            break;

        default:
            break;
    }
}

void ClientPlayerHandler::Play()
{
}

void ClientPlayerHandler::Pause()
{
}

void ClientPlayerHandler::Stop()
{
}

void ClientPlayerHandler::Next()
{
}

void ClientPlayerHandler::Previous()
{
}

void ClientPlayerHandler::VolumeUp()
{
}

void ClientPlayerHandler::VolumeDown()
{
}

#undef SEND_PACKET
