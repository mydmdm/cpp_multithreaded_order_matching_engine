#ifndef FIX44_XMLNONFIX_H
#define FIX44_XMLNONFIX_H

#include "Message.h"

namespace FIX44
{

  class XMLnonFIX : public Message
  {
  public:
    XMLnonFIX() : Message(MsgType()) {}
    XMLnonFIX(const FIX::Message& m) : Message(m) {}
    XMLnonFIX(const Message& m) : Message(m) {}
    XMLnonFIX(const XMLnonFIX& m) : Message(m) {}
    static FIX::MsgType MsgType() { return FIX::MsgType("n"); }

  };

}

#endif
