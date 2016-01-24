//
// Generated file, do not edit! Created by nedtool 4.6 from transport/rtp/RTPSenderStatusMessage.msg.
//

#ifndef _RTPSENDERSTATUSMESSAGE_M_H_
#define _RTPSENDERSTATUSMESSAGE_M_H_

#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0406
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif

// dll export symbol
#ifndef INET_API
#  if defined(INET_EXPORT)
#    define INET_API  OPP_DLLEXPORT
#  elif defined(INET_IMPORT)
#    define INET_API  OPP_DLLIMPORT
#  else
#    define INET_API
#  endif
#endif



// cplusplus {{
#include "INETDefs.h"
// }}

/**
 * Enum generated from <tt>transport/rtp/RTPSenderStatusMessage.msg:33</tt> by nedtool.
 * <pre>
 * //
 * // Messages of type ~RTPSenderStatusMessage are used to send information
 * // from an rtp sender module to the application. Within this class a status
 * // string is defined in which the information is stored. This can be "PLAYING",
 * // "STOPPED" or "FINISHED".
 * // If a message must provide more information than just a string, a new class
 * // defining this parameter can derived.
 * //
 * enum RTPSenderStatus
 * {
 * 
 *     RTP_SENDER_STATUS_PLAYING = 1;
 *     RTP_SENDER_STATUS_FINISHED = 2;
 *     RTP_SENDER_STATUS_STOPPED = 3;
 *     RTP_SENDER_STATUS_PAUSED = 4;
 * }
 * </pre>
 */
enum RTPSenderStatus {
    RTP_SENDER_STATUS_PLAYING = 1,
    RTP_SENDER_STATUS_FINISHED = 2,
    RTP_SENDER_STATUS_STOPPED = 3,
    RTP_SENDER_STATUS_PAUSED = 4
};

/**
 * Class generated from <tt>transport/rtp/RTPSenderStatusMessage.msg:41</tt> by nedtool.
 * <pre>
 * packet RTPSenderStatusMessage
 * {
 *     short status @enum(RTPSenderStatus);
 *     uint32 timeStamp;
 * }
 * </pre>
 */
class INET_API RTPSenderStatusMessage : public ::cPacket
{
  protected:
    short status_var;
    uint32 timeStamp_var;

  private:
    void copy(const RTPSenderStatusMessage& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const RTPSenderStatusMessage&);

  public:
    RTPSenderStatusMessage(const char *name=NULL, int kind=0);
    RTPSenderStatusMessage(const RTPSenderStatusMessage& other);
    virtual ~RTPSenderStatusMessage();
    RTPSenderStatusMessage& operator=(const RTPSenderStatusMessage& other);
    virtual RTPSenderStatusMessage *dup() const {return new RTPSenderStatusMessage(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual short getStatus() const;
    virtual void setStatus(short status);
    virtual uint32 getTimeStamp() const;
    virtual void setTimeStamp(uint32 timeStamp);
};

inline void doPacking(cCommBuffer *b, RTPSenderStatusMessage& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, RTPSenderStatusMessage& obj) {obj.parsimUnpack(b);}


#endif // ifndef _RTPSENDERSTATUSMESSAGE_M_H_

