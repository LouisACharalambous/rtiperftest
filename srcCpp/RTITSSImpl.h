#ifndef __RTITSSIMPL_H__
#define __RTITSSIMPL_H__

#ifdef RTI_PERF_TSS

#include "Infrastructure_common.h"
#include "MessagingIF.h"

#include "perftest.hxx"
#include "perftestSupport.h"
#include "perftest_TypedTS_Impl.hpp"
#include "RTI/TSS/Base.hpp"

#if defined(RTI_DARWIN) && !defined(RTI_PERF_MICRO)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

template <class Type, class TypedTS>
class TSSConnection
{
    FACE::TSS::CONNECTION_ID_TYPE _connection_id;
    TypedTS *_typedTS;

    TestMessage _message;
    Type _sample;

protected:
    inline bool _send(const TestMessage &message);
    inline TestMessage *_receive();

public:
    TSSConnection(FACE::TSS::CONNECTION_ID_TYPE connection_id)
        : _connection_id(connection_id),
          _typedTS(new TypedTS())
    {
        if (!DDS_OctetSeq_initialize(&_sample.bin_data)) {
            fprintf(stderr, "Failed to initialize bin_data\n");
        }
    }

    ~TSSConnection() { delete _typedTS; }
};

template <typename T>
class RTITSSImpl : public IMessaging
{
    RTI::TSS::Base *_tss;
    std::vector<FACE::TSS::CONNECTION_ID_TYPE> _connections;

    enum ConnType {
        PUBLISHER,
        SUBSCRIBER
    };

    FACE::TSS::CONNECTION_ID_TYPE _createConnection(
            const char *name, FACE::RETURN_CODE_TYPE::Value &retcode);

    auto _createTypedPublisher(FACE::TSS::CONNECTION_ID_TYPE conn_id);
    auto _createTypedSubscriber(FACE::TSS::CONNECTION_ID_TYPE conn_id);

public:
    RTITSSImpl();
    ~RTITSSImpl();

    bool Initialize(ParameterManager &PM, perftest_cpp *parent);
    void Shutdown();

    std::string PrintConfiguration();

    IMessagingWriter *CreateWriter(const char *topic_name);
    IMessagingReader *CreateReader(const char *topic_name, IMessagingCB *callback);

    bool supports_listener() { return true; }
    bool supports_discovery() { return true; }

    unsigned long GetInitializationSampleCount();
    bool get_serialized_overhead_size(unsigned int &overhead_size);
};

template <class Type, class TypedTS>
class RTITSSPublisher : public IMessagingWriter,
                        public TSSConnection<Type, TypedTS>
{
public:
    RTITSSPublisher(FACE::TSS::CONNECTION_ID_TYPE connection_id)
        : TSSConnection<Type, TypedTS>(connection_id) {}

    bool Send(const TestMessage &message, bool isCftWildCardKey = false);
    void Flush() { /* dummy */ }

    void waitForAck(int sec, unsigned int nsec);
    void WaitForReaders(int numSubscribers);
    bool waitForPingResponse();
    bool waitForPingResponse(int timeout);
    bool notifyPingResponse();

    unsigned int getPulledSampleCount();
    unsigned int getSampleCount();
    unsigned int getSampleCountPeak();
};

template <class Type, class TypedTS>
class RTITSSSubscriber : public IMessagingReader,
                         public TSSConnection<Type, TypedTS>
{
public:
    RTITSSSubscriber(FACE::TSS::CONNECTION_ID_TYPE connection_id)
        : TSSConnection<Type, TypedTS>(connection_id) {}

    TestMessage *ReceiveMessage();
    bool unblock();

    void WaitForWriters(int numPublishers);

    unsigned int getSampleCount();
    unsigned int getSampleCountPeak();
};

#endif /* RTI_PERF_TSS */
#endif /* __RTITSSIMPL_H__ */