/*
 * (c) 2005-2020  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "MessagingIF.h"
#include "perftest_cpp.h"
#include "RTITSSImpl.h"
#include "rti_tss_impl.h"

#ifndef RTI_PERF_MICRO
  #include "ndds/ndds_cpp.h"
#endif

/* Pointer to PM used by RTI_TSS_ConfigData_get_config_data */
extern ParameterManager *RTI_TSS_gv_pm;

/* RTITSSImpl implementation */
template <class Type, class TypedTS, class TypedCB>
RTITSSImpl<Type, TypedTS, TypedCB>::RTITSSImpl()
{
    _tss = new RTI::TSS::Base;
    _pong_semaphore = NULL;
    _pm = NULL;
}

template <class Type, class TypedTS, class TypedCB>
RTITSSImpl<Type, TypedTS, TypedCB>::~RTITSSImpl()
{
    delete _tss;
}

template <class Type, class TypedTS, class TypedCB>
bool RTITSSImpl<Type, TypedTS, TypedCB>::Initialize(ParameterManager &PM, perftest_cpp *parent)
{
    FACE::RETURN_CODE_TYPE::Value retcode;

    /* Set reference for PM to be used at RTI_TSS_ConfigData_get_config_data */
    RTI_TSS_gv_pm = &PM;
    _pm = &PM;

    _tss->Initialize("RTI_TSS_STATIC_INITIALIZATION##perftest", retcode);
    if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR) {
		fprintf(stderr, "Failed Initialize(rc=%d)\n", retcode);
		return false;
	}

    if (_pm->get<bool>("latencyTest")) {
        _pong_semaphore = PerftestSemaphore_new();
        if (_pong_semaphore == NULL) {
            fprintf(stderr, "Failed create pong semaphore\n");
		    return false;
        }
    }

    return true;
}

template <class Type, class TypedTS, class TypedCB>
void RTITSSImpl<Type, TypedTS, TypedCB>::Shutdown()
{
    FACE::RETURN_CODE_TYPE::Value retcode;

    for (int i = 0; i < _connections.size(); ++i) {
        _tss->Destroy_Connection(_connections[i], retcode);
        if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR)
	    {
		    fprintf(stderr, "Failed destroy connection (rc=%d)\n", retcode);
	    }
    }

    if(_pong_semaphore != NULL) {
        PerftestSemaphore_delete(_pong_semaphore);
        _pong_semaphore = NULL;
    }
}

template <class Type, class TypedTS, class TypedCB>
std::string RTITSSImpl<Type, TypedTS, TypedCB>::PrintConfiguration()
{
    std::ostringstream stringStream;

    stringStream << "\tDomain: " << _pm->get<int>("domain") << std::endl;

    stringStream << "\tFACE Profile: ";
#if ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_NONE
    stringStream << "None";
#elif ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_SECURITY
    stringStream << "Security";
#elif ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_SAFETY_BASE
    stringStream << "Safety Base";
#elif ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_SAFETY_EXTENDED
    stringStream << "Safety Extended";
#elif ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_GENERAL
    stringStream << "General Purpose";
#else
    #warning "Unknown FACE profile";
    stringStream << "Unknown - " << ENABLE_FACE_COMPLIANCE;
#endif

    stringStream << std::endl;

    return stringStream.str();
}

template <class Type, class TypedTS, class TypedCB>
FACE::TSS::CONNECTION_ID_TYPE RTITSSImpl<Type, TypedTS, TypedCB>::_createConnection(
        std::string name, FACE::RETURN_CODE_TYPE::Value &retcode)
{
    FACE::TSS::CONNECTION_NAME_TYPE connection_name(name.c_str());
    FACE::TSS::MESSAGE_SIZE_TYPE max_message_size;
    FACE::TSS::CONNECTION_ID_TYPE connection_id;
    FACE::TIMEOUT_TYPE timeout(0);

    _tss->Create_Connection(connection_name,
                            timeout,
                            connection_id,
                            max_message_size,
                            retcode);

    return connection_id;
}

template <class Type, class TypedTS, class TypedCB>
IMessagingWriter *RTITSSImpl<Type, TypedTS, TypedCB>::CreateWriter(const char *topic_name)
{
    std::string name = "writer " + std::string(topic_name);
    FACE::TSS::CONNECTION_ID_TYPE connection_id;
    FACE::RETURN_CODE_TYPE::Value retcode;

    connection_id = _createConnection(name, retcode);
    if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR) {
		fprintf(stderr, "Failed Create_Connection %s (writer) (rc=%d)\n",
                name.c_str(), retcode);
        return NULL;
	}

    _connections.push_back(connection_id);

    return new RTITSSPublisher<Type, TypedTS, TypedCB>(
                connection_id, _pong_semaphore);
}

template <class Type, class TypedTS, class TypedCB>
IMessagingReader *RTITSSImpl<Type, TypedTS, TypedCB>::CreateReader(const char *topic_name,
                                              IMessagingCB *callback)
{
    std::string name = "reader " + std::string(topic_name);
    FACE::TSS::CONNECTION_ID_TYPE connection_id;
    FACE::RETURN_CODE_TYPE::Value retcode;

    connection_id = _createConnection(name, retcode);
    if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR) {
		fprintf(stderr, "Failed Create_Connection %s (reader) (rc=%d)\n",
                name.c_str(), retcode);
        return NULL;
	}

    _connections.push_back(connection_id);

    return new RTITSSSubscriber<Type, TypedTS, TypedCB>(
                connection_id, _pong_semaphore, callback);
}

template <class Type, class TypedTS, class TypedCB>
unsigned long RTITSSImpl<Type, TypedTS, TypedCB>::GetInitializationSampleCount()
{
    return _pm->get<unsigned long>("sendQueueSize");
}

template <>
int RTITSSImpl<FACE::DM::TestData_t,
               TestData_t::TypedTS,
               TestData_t::Read_Callback>::_serializeTyped(
        FACE::DM::TestData_t *data, unsigned int &size)
{
    return FACE_DM_TestData_tPlugin_serialize_to_cdr_buffer(NULL, &size, data);
}

template <class Type, class TypedTS, class TypedCB>
bool RTITSSImpl<Type, TypedTS, TypedCB>::get_serialized_overhead_size(unsigned int &overhead_size)
{
    Type data;
    data.entity_id = 0;
    data.seq_num = 0;
    data.timestamp_sec = 0;
    data.timestamp_usec = 0;
    data.latency_ping = 0;

    if (!DDS_OctetSeq_ensure_length(&data.bin_data, 0, 0)) {
        fprintf(stderr, "Failed DDS_OctetSeq_ensure_length\n");
        return false;
    }

    if (_serializeTyped(&data, overhead_size) != DDS_RETCODE_OK) {
        fprintf(stderr, "Failed to serialize to CDR buffer\n");
        return false;
    }

    overhead_size -= RTI_CDR_ENCAPSULATION_HEADER_SIZE;

    return true;
}


/* RTITSSPublisher implementation */
template <class Type, class TypedTS, class TypedCB>
RTITSSPublisher<Type, TypedTS, TypedCB>::RTITSSPublisher(
        FACE::TSS::CONNECTION_ID_TYPE connection_id, PerftestSemaphore *pongSemaphore)
        : TSSConnection<Type, TypedTS, TypedCB>(connection_id),
          _pong_semaphore(pongSemaphore)
{
    DDS_DataWriterQos qos;
    DDS_DataWriterQos_initialize(&qos);
    DDS_DataWriter_get_qos(this->_writer, &qos);
    _is_reliable = (qos.reliability.kind == DDS_RELIABLE_RELIABILITY_QOS);
}

template <class Type, class TypedTS, class TypedCB>
bool RTITSSPublisher<Type, TypedTS, TypedCB>::Send(const TestMessage &message,
                                          bool isCftWildCardKey)
{
    return this->_send(message);
}

template <class Type, class TypedTS, class TypedCB>
void RTITSSPublisher<Type, TypedTS, TypedCB>::waitForAck(int sec, unsigned int nsec)
{
#ifndef RTI_PERF_MICRO
    if (_is_reliable) {
        DDS_Duration_t timeout = {sec, nsec};
        DDS_DataWriter_wait_for_acknowledgments(this->_writer, &timeout);
    } else {
        PerftestClock::milliSleep(nsec / 1000000);
    }
#endif
}

template <class Type, class TypedTS, class TypedCB>
void RTITSSPublisher<Type, TypedTS, TypedCB>::WaitForReaders(int numSubscribers)
{
    DDS_PublicationMatchedStatus status;
    DDS_ReturnCode_t retcode;

    while (true) {
        retcode = DDS_DataWriter_get_publication_matched_status(this->_writer, &status);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "WaitForReaders failed: %d\n", retcode);
        }

        if (status.current_count >= numSubscribers) {
            break;
        }

        PerftestClock::milliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
    }
}

template <class Type, class TypedTS, class TypedCB>
bool RTITSSPublisher<Type, TypedTS, TypedCB>::waitForPingResponse(int timeout)
{
    if(_pong_semaphore != NULL) {
        if (!PerftestSemaphore_take(_pong_semaphore, timeout)) {
            fprintf(stderr, "Error taking semaphore\n");
            return false;
        }
    }

    return true;
}

template <class Type, class TypedTS, class TypedCB>
bool RTITSSPublisher<Type, TypedTS, TypedCB>::notifyPingResponse()
{
    if(_pong_semaphore != NULL) {
        if (!PerftestSemaphore_give(_pong_semaphore)) {
            fprintf(stderr, "Error giving semaphore\n");
            return false;
        }
    }

    return true;
}

template <class Type, class TypedTS, class TypedCB>
unsigned int RTITSSPublisher<Type, TypedTS, TypedCB>::getPulledSampleCount()
{
#ifndef RTI_PERF_MICRO
    DDS_DataWriterProtocolStatus status;
    DDS_ReturnCode_t retcode;

    retcode = DDS_DataWriter_get_datawriter_protocol_status(this->_writer, &status);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "Error getting protocol status\n");
        return -1;
    }

    return (unsigned int) status.pulled_sample_count;
#else
    // Not supported in Micro
    return 0;
#endif
}

template <class Type, class TypedTS, class TypedCB>
unsigned int RTITSSPublisher<Type, TypedTS, TypedCB>::getSampleCount()
{
#ifndef RTI_PERF_MICRO
    DDS_DataWriterCacheStatus status;
    DDS_ReturnCode_t retcode;

    retcode = DDS_DataWriter_get_datawriter_cache_status(this->_writer, &status);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "get_datawriter_cache_status failed: %d.\n", retcode);
        return -1;
    }

    return (unsigned int) status.sample_count;
#else
    // Not supported in Micro
    return 0;
#endif
}

template <class Type, class TypedTS, class TypedCB>
unsigned int RTITSSPublisher<Type, TypedTS, TypedCB>::getSampleCountPeak()
{
#ifndef RTI_PERF_MICRO
    DDS_DataWriterCacheStatus status;
    DDS_ReturnCode_t retcode;

    retcode = DDS_DataWriter_get_datawriter_cache_status(this->_writer, &status);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "get_datawriter_cache_status failed: %d.\n", retcode);
        return -1;
    }

    return (unsigned int)status.sample_count_peak;
#else
    // Not supported in Micro
    return 0;
#endif
}


/* RTITSSSubscriber Implementation */
template <class Type, class TypedTS, class TypedCB>
TestMessage *RTITSSSubscriber<Type, TypedTS, TypedCB>::ReceiveMessage()
{
    return this->_receive();
}

template <class Type, class TypedTS, class TypedCB>
void RTITSSSubscriber<Type, TypedTS, TypedCB>::WaitForWriters(int numPublishers)
{
    DDS_SubscriptionMatchedStatus status;
    DDS_ReturnCode_t retcode;

    while (true) {
        retcode = DDS_DataReader_get_subscription_matched_status(this->_reader, &status);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "WaitForWriters failed: %d\n", retcode);
        }

        if (status.current_count >= numPublishers) {
            break;
        }

        PerftestClock::milliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
    }
}

template <class Type, class TypedTS, class TypedCB>
unsigned int RTITSSSubscriber<Type, TypedTS, TypedCB>::getSampleCount()
{
#ifndef RTI_PERF_MICRO
    DDS_DataReaderCacheStatus status;
    DDS_ReturnCode_t retcode;

    retcode = DDS_DataReader_get_datareader_cache_status(this->_reader, &status);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "get_datareader_cache_status failed: %d.\n", retcode);
        return -1;
    }

    return (unsigned int)status.sample_count;
#else
    // Not supported in Micro
    return 0;
#endif
}

template <class Type, class TypedTS, class TypedCB>
unsigned int RTITSSSubscriber<Type, TypedTS, TypedCB>::getSampleCountPeak()
{
#ifndef RTI_PERF_MICRO
    DDS_DataReaderCacheStatus status;
    DDS_ReturnCode_t retcode;

    retcode = DDS_DataReader_get_datareader_cache_status(this->_reader, &status);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "get_datareader_cache_status failed: %d.\n", retcode);
        return -1;
    }

    return (unsigned int)status.sample_count_peak;
#else
    // Not supported in Micro
    return 0;
#endif
}

template <class Type, class TypedTS, class TypedCB>
TSSConnection<Type, TypedTS, TypedCB>::TSSConnection(
        FACE::TSS::CONNECTION_ID_TYPE connection_id, IMessagingCB *callback)
        : _connection_id(connection_id), _typedTS(new TypedTS), _typedCB(NULL)
{
    RTI_TSS_Impl *rti_tss = NULL;
    DDS_DomainParticipant *participant = NULL;
    FACE::RETURN_CODE_TYPE::Value retcode;
    FACE_CONNECTION_DIRECTION_TYPE direction;
    struct RTI_TSS_ConnectionEntry *connection_entry = NULL;
    bool set_writer = false;
    bool set_reader = false;

    rti_tss = RTI_TSS_Impl_get_instance();
    if (rti_tss == NULL) {
        fprintf(stderr, "Failed to get TSS impl instance.\n");
        return;
    }

    connection_entry = RTI_TSS_ConnectionManager_lookup_connection_id(
            rti_tss->connection_mgr, _connection_id);
    if (connection_entry == NULL)
    {
        fprintf(stderr, "Failed to get connection entry.\n");
        return;
    }

    participant = RTI_TSS_ConnectionEntry_get_participant(connection_entry);
    if (participant == NULL)
    {
        fprintf(stderr, "Failed to get participant from connection.\n");
        return;
    }

    direction = RTI_TSS_ConnectionEntry_get_connection_direction(connection_entry);
    if (direction == FACE_SOURCE) {
        set_writer = true;
    } else if (direction == FACE_DESTINATION) {
        set_reader = true;
    } else {
        set_writer = true;
        set_reader = true;
    }

    if (set_writer) {
        _writer = RTI_TSS_ConnectionEntry_get_writer(connection_entry);
        if (_writer == NULL) {
            fprintf(stderr, "Failed to get writer from connection.\n");
            return;
        }
    }

    if (set_reader) {
        _reader = RTI_TSS_ConnectionEntry_get_reader(connection_entry);
        if (_reader == NULL) {
            fprintf(stderr, "Failed to get reader from connection.\n");
            return;
        }
    }

    if (callback != NULL) {
        _typedCB = new TSSListener<Type, TypedCB>(callback);
        _typedTS->Register_Callback(_connection_id, *_typedCB, retcode);
        if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR)
        {
            printf("Failed Register_Callback (rc=%d)\n", retcode);
            return;
        }
    }
}

template <class Type, class TypedTS, class TypedCB>
TSSConnection<Type, TypedTS, TypedCB>::~TSSConnection()
{
    if (_typedTS != NULL) delete _typedTS;
    //if (_typedCB != NULL) delete _typedCB;
}

template <class Type, class TypedTS, class TypedCB>
inline bool TSSConnection<Type, TypedTS, TypedCB>::_send(const TestMessage &message)
{
    FACE::RETURN_CODE_TYPE::Value retcode;
    FACE::TSS::TRANSACTION_ID_TYPE transaction_id(0);
    FACE::TIMEOUT_TYPE timeout(0);

    _sample.entity_id = message.entity_id;
    _sample.seq_num = message.seq_num;
    _sample.timestamp_sec = message.timestamp_sec;
    _sample.timestamp_usec = message.timestamp_usec;
    _sample.latency_ping = message.latency_ping;

#ifdef RTI_PERF_PRO
    if (!DDS_OctetSeq_loan_contiguous(&_sample.bin_data,
                                      (DDS_Octet*) message.data,
                                      message.size, message.size)) {
        fprintf(stderr, "bin_data.loan_contiguous() failed.\n");
        return false;
    }
#else
    // TODO: To be implemented
    if (!FACE_sequence_loan_contiguous(&_sample.bin_data,
                                       (DDS_Octet*) _message.data,
                                       _message.size, _message.size)) {
        fprintf(stderr, "bin_data.loan_contiguous() failed.\n");
        return false;
    }
#endif // RTI_PERF_PRO

    _typedTS->Send_Message(_connection_id,
                           timeout,
                           transaction_id,
                           _sample,
                           retcode);
    if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR)
	{
		fprintf(stderr, "Failed Send_Message (rc=%d)\n", retcode);
        return false;
	}

#ifdef RTI_PERF_PRO
    if (!DDS_OctetSeq_unloan(&_sample.bin_data)) {
        fprintf(stderr, "bin_data.unloan() failed.\n");
        return false;
    }
#else
    if (!FACE_sequence_unloan(&_sample.bin_data)) {
        fprintf(stderr, "bin_data.unloan() failed.\n");
        return false;
    }
#endif // RTI_PERF_PRO

    return true;
}

template <class Type, class TypedTS, class TypedCB>
inline TestMessage *TSSConnection<Type, TypedTS, TypedCB>::_receive()
{
    FACE::RETURN_CODE_TYPE::Value retcode;
    FACE::TSS::TRANSACTION_ID_TYPE transaction_id(0);
    FACE::TSS::QoS_EVENT_TYPE qos_parameters;
    FACE::TSS::HEADER_TYPE header;
    FACE::TIMEOUT_TYPE timeout(0);

    _typedTS->Receive_Message(_connection_id,
                              timeout,
                              transaction_id, /* not used */
                              _sample,
                              header, /* not used */
                              qos_parameters, /* not used */
                              retcode);
    if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR)
	{
        if (retcode != FACE::RETURN_CODE_TYPE::NOT_AVAILABLE)
            fprintf(stderr, "!Failed Receive_Message (rc=%d)\n", retcode);

        return NULL;
	}

    _message.entity_id = _sample.entity_id;
    _message.seq_num = _sample.seq_num;
    _message.timestamp_sec = _sample.timestamp_sec;
    _message.timestamp_usec = _sample.timestamp_usec;
    _message.latency_ping = _sample.latency_ping;
    _message.size = DDS_OctetSeq_get_length(&_sample.bin_data);
    _message.data = (char *) DDS_OctetSeq_get_contiguous_buffer(&_sample.bin_data);

    return &_message;
}


template <class Type, class TypedCB>
void TSSListener<Type, TypedCB>::Callback_Handler(
        FACE::TSS::CONNECTION_ID_TYPE connection_id,
        FACE::TSS::TRANSACTION_ID_TYPE transaction_id,
        const Type& sample,
        const FACE::TSS::HEADER_TYPE& header,
        const FACE::TSS::QoS_EVENT_TYPE& qos_parameters,
        FACE::RETURN_CODE_TYPE::Value& return_code)
{
    _message.entity_id = sample.entity_id;
    _message.seq_num = sample.seq_num;
    _message.timestamp_sec = sample.timestamp_sec;
    _message.timestamp_usec = sample.timestamp_usec;
    _message.latency_ping = sample.latency_ping;
    _message.size = DDS_OctetSeq_get_length(&sample.bin_data);
    _message.data = (char *) DDS_OctetSeq_get_contiguous_buffer(&sample.bin_data);

    _callback->ProcessMessage(_message);
}

template class RTITSSImpl<FACE::DM::TestData_t, TestData_t::TypedTS, TestData_t::Read_Callback>;