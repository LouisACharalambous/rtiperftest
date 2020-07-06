#ifndef __RTITSSIMPL_H__
#define __RTITSSIMPL_H__

#ifdef RTI_PERF_TSS

#include "Infrastructure_common.h"
#include "MessagingIF.h"

#include "perftest_cpp.h"
#include "perftest.hxx"
#include "perftestSupport.h"
#include "perftestPlugin.h"
#include "perftest_TypedTS_Impl.hpp"
#include "RTI/TSS/Base.hpp"

template <class Type, class TypedCB>
class TSSListener : public TypedCB
{
    IMessagingCB *_callback;
    TestMessage _message;

public:
    TSSListener(IMessagingCB *callback)
        : _callback(callback), _message() {}

    void Callback_Handler(
            FACE::TSS::CONNECTION_ID_TYPE connection_id,
            FACE::TSS::TRANSACTION_ID_TYPE transaction_id,
            const Type& sample,
            const FACE::TSS::HEADER_TYPE& header,
            const FACE::TSS::QoS_EVENT_TYPE& qos_parameters,
            FACE::RETURN_CODE_TYPE::Value& return_code);
};

template <class Type, class TypedTS, class TypedCB>
class TSSConnection
{
    FACE::TSS::CONNECTION_ID_TYPE _connection_id;
    TypedTS *_typedTS;
    TSSListener<Type, TypedCB> *_typedCB;

    TestMessage _message;
    Type _sample;

    void _registerCallBack(IMessagingCB *callback);

protected:
    DDS_DataWriter *_writer;
    DDS_DataReader *_reader;

    inline bool _send(const TestMessage &message);
    inline TestMessage *_receive();

public:
    TSSConnection(FACE::TSS::CONNECTION_ID_TYPE connection_id,
                  IMessagingCB *callback = NULL);
    ~TSSConnection();
};

template <typename T>
class RTITSSImpl : public IMessaging
{
    std::vector<FACE::TSS::CONNECTION_ID_TYPE> _connections;
    RTI::TSS::Base *_tss;
    PerftestSemaphore *_pong_semaphore;

    FACE::TSS::CONNECTION_ID_TYPE _createConnection(
            std::string name, FACE::RETURN_CODE_TYPE::Value &retcode);

    /* NOTE: The following methods are type-specific,
     * so they have to be defined for each type.
     */
    int _serializeTyped(T *data, unsigned int &size);
    auto _createTypedPublisher(FACE::TSS::CONNECTION_ID_TYPE conn_id);
    auto _createTypedSubscriber(FACE::TSS::CONNECTION_ID_TYPE conn_id,
                                IMessagingCB *callback);

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

template <class Type, class TypedTS, class TypedCB>
class RTITSSPublisher : public IMessagingWriter,
                        public TSSConnection<Type, TypedTS, TypedCB>
{
    PerftestSemaphore *_pong_semaphore;
    bool _is_reliable;

public:
    RTITSSPublisher(FACE::TSS::CONNECTION_ID_TYPE connection_id,
                    PerftestSemaphore *pongSemaphore);

    bool Send(const TestMessage &message, bool isCftWildCardKey = false);
    void Flush() { /* dummy */ }

    void waitForAck(int sec, unsigned int nsec);
    void WaitForReaders(int numSubscribers);
    bool waitForPingResponse(int timeout = PERFTEST_SEMAPHORE_TIMEOUT_INFINITE);
    bool notifyPingResponse();

    unsigned int getPulledSampleCount();
    unsigned int getSampleCount();
    unsigned int getSampleCountPeak();
};

template <class Type, class TypedTS, class TypedCB>
class RTITSSSubscriber : public IMessagingReader,
                         public TSSConnection<Type, TypedTS, TypedCB>
{
    PerftestSemaphore *_pong_semaphore;

public:
    RTITSSSubscriber(FACE::TSS::CONNECTION_ID_TYPE connection_id,
                    PerftestSemaphore *pongSemaphore,
                    IMessagingCB *callback)
        : TSSConnection<Type, TypedTS, TypedCB>(connection_id, callback),
          _pong_semaphore(pongSemaphore) {}

    TestMessage *ReceiveMessage();

    void WaitForWriters(int numPublishers);

    unsigned int getSampleCount();
    unsigned int getSampleCountPeak();
};

#endif /* RTI_PERF_TSS */
#endif /* __RTITSSIMPL_H__ */