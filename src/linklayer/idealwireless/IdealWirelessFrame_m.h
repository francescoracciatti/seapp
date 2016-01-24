//
// Generated file, do not edit! Created by nedtool 4.6 from linklayer/idealwireless/IdealWirelessFrame.msg.
//

#ifndef _IDEALWIRELESSFRAME_M_H_
#define _IDEALWIRELESSFRAME_M_H_

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

#include "MACAddress.h"
// }}

/**
 * Class generated from <tt>linklayer/idealwireless/IdealWirelessFrame.msg:33</tt> by nedtool.
 * <pre>
 * //
 * // Packet for ~IdealWirelessMac. Packet size is configurable
 * // in the MAC layer. 
 * //
 * packet IdealWirelessFrame
 * {
 *     MACAddress src;     // source address
 *     MACAddress dest;    // destination address
 *     int srcModuleId;    // technical data, uses instead of sending back an ACK packet
 * }
 * </pre>
 */
class INET_API IdealWirelessFrame : public ::cPacket
{
  protected:
    MACAddress src_var;
    MACAddress dest_var;
    int srcModuleId_var;

  private:
    void copy(const IdealWirelessFrame& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const IdealWirelessFrame&);

  public:
    IdealWirelessFrame(const char *name=NULL, int kind=0);
    IdealWirelessFrame(const IdealWirelessFrame& other);
    virtual ~IdealWirelessFrame();
    IdealWirelessFrame& operator=(const IdealWirelessFrame& other);
    virtual IdealWirelessFrame *dup() const {return new IdealWirelessFrame(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual MACAddress& getSrc();
    virtual const MACAddress& getSrc() const {return const_cast<IdealWirelessFrame*>(this)->getSrc();}
    virtual void setSrc(const MACAddress& src);
    virtual MACAddress& getDest();
    virtual const MACAddress& getDest() const {return const_cast<IdealWirelessFrame*>(this)->getDest();}
    virtual void setDest(const MACAddress& dest);
    virtual int getSrcModuleId() const;
    virtual void setSrcModuleId(int srcModuleId);
};

inline void doPacking(cCommBuffer *b, IdealWirelessFrame& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, IdealWirelessFrame& obj) {obj.parsimUnpack(b);}


#endif // ifndef _IDEALWIRELESSFRAME_M_H_

