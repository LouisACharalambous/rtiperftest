/*
 * (c) 2005-2020  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "MessagingIF.h"
#include "perftest_cpp.h"
#include "RTITSSImpl.h"

/* RTITSSImpl implementation */
template <typename T>
RTITSSImpl<T>::RTITSSImpl()
{
    _tss = new RTI::TSS::Base;
}

template <typename T>
RTITSSImpl<T>::~RTITSSImpl()
{
    delete _tss;
}

template <typename T>
bool RTITSSImpl<T>::Initialize(ParameterManager &PM, perftest_cpp *parent)
{
    FACE::RETURN_CODE_TYPE::Value retcode;

    _tss->Initialize("RTI_TSS_STATIC_INITIALIZATION##perftest", retcode);
    if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR) {
		fprintf(stderr, "Failed Initialize(rc=%d)\n", retcode);
		return false;
	}

    return true;
}

template <typename T>
void RTITSSImpl<T>::Shutdown()
{
    FACE::RETURN_CODE_TYPE::Value retcode;

    for (FACE::TSS::CONNECTION_ID_TYPE conn : _connections) {
        _tss->Destroy_Connection(conn, retcode);
        if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR)
	    {
		    fprintf(stderr, "Failed destroy connection (rc=%d)\n", retcode);
	    }
    }
}

template <typename T>
std::string RTITSSImpl<T>::PrintConfiguration()
{
    return "Some TSS Config";
}

template <>
auto RTITSSImpl<FACE::DM::TestData_t>::_createTypedPublisher(FACE::TSS::CONNECTION_ID_TYPE conn_id)
{
    return new RTITSSPublisher<FACE::DM::TestData_t, TestData_t::TypedTS>(conn_id);
}

template <>
auto RTITSSImpl<FACE::DM::TestData_t>::_createTypedSubscriber(FACE::TSS::CONNECTION_ID_TYPE conn_id)
{
    return new RTITSSSubscriber<FACE::DM::TestData_t, TestData_t::TypedTS>(conn_id);
}


template <typename T>
FACE::TSS::CONNECTION_ID_TYPE RTITSSImpl<T>::_createConnection(
        const char *name, FACE::RETURN_CODE_TYPE::Value &retcode)
{
    FACE::TSS::CONNECTION_NAME_TYPE connection_name(name);
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

template <typename T>
IMessagingWriter *RTITSSImpl<T>::CreateWriter(const char *topic_name)
{
    FACE::TSS::CONNECTION_ID_TYPE connection_id;
    FACE::RETURN_CODE_TYPE::Value retcode;

    connection_id = _createConnection(topic_name, retcode);
    if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR)
	{
		fprintf(stderr, "Failed Create_Connection for %s (rc=%d)\n",
                topic_name, retcode);
        return NULL;
	}

    _connections.push_back(connection_id);
    return _createTypedPublisher(connection_id);
}

template <typename T>
IMessagingReader *RTITSSImpl<T>::CreateReader(const char *topic_name,
                                              IMessagingCB *callback)
{
    FACE::TSS::CONNECTION_ID_TYPE connection_id;
    FACE::RETURN_CODE_TYPE::Value retcode;

    connection_id = _createConnection(topic_name, retcode);
    if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR)
	{
		fprintf(stderr, "Failed Create_Connection (rc=%d)\n", retcode);
        return NULL;
	}

    _connections.push_back(connection_id);
    return _createTypedSubscriber(connection_id);
}

template <typename T>
unsigned long RTITSSImpl<T>::GetInitializationSampleCount()
{
    return 0;
}

template <typename T>
bool RTITSSImpl<T>::get_serialized_overhead_size(unsigned int &overhead_size)
{
    return true;
}


/* RTITSSPublisher implementation */
template <class Type, class TypedTS>
bool RTITSSPublisher<Type, TypedTS>::Send(const TestMessage &message,
                                          bool isCftWildCardKey)
{
    return this->_send(message);
}

template <class Type, class TypedTS>
void RTITSSPublisher<Type, TypedTS>::waitForAck(int sec, unsigned int nsec)
{
}

template <class Type, class TypedTS>
void RTITSSPublisher<Type, TypedTS>::WaitForReaders(int numSubscribers)
{
}

template <class Type, class TypedTS>
bool RTITSSPublisher<Type, TypedTS>::waitForPingResponse()
{
    return true;
}

template <class Type, class TypedTS>
bool RTITSSPublisher<Type, TypedTS>::waitForPingResponse(int timeout)
{
    return true;
}

template <class Type, class TypedTS>
bool RTITSSPublisher<Type, TypedTS>::notifyPingResponse()
{
    return true;
}

template <class Type, class TypedTS>
unsigned int RTITSSPublisher<Type, TypedTS>::getPulledSampleCount()
{
    return 0;
}

template <class Type, class TypedTS>
unsigned int RTITSSPublisher<Type, TypedTS>::getSampleCount()
{
    return 0;
}

template <class Type, class TypedTS>
unsigned int RTITSSPublisher<Type, TypedTS>::getSampleCountPeak()
{
    return 0;
}


/* RTITSSSubscriber Implementation */
template <class Type, class TypedTS>
TestMessage *RTITSSSubscriber<Type, TypedTS>::ReceiveMessage()
{
    return NULL;
}

template <class Type, class TypedTS>
bool RTITSSSubscriber<Type, TypedTS>::unblock()
{
    return false;
}

template <class Type, class TypedTS>
void RTITSSSubscriber<Type, TypedTS>::WaitForWriters(int numPublishers)
{
}

template <class Type, class TypedTS>
unsigned int RTITSSSubscriber<Type, TypedTS>::getSampleCount()
{
    return 0;
}

template <class Type, class TypedTS>
unsigned int RTITSSSubscriber<Type, TypedTS>::getSampleCountPeak()
{
    return 0;
}

template <class Type, class TypedTS>
inline bool TSSConnection<Type, TypedTS>::_send(const TestMessage &message)
{
    FACE::RETURN_CODE_TYPE::Value retcode;
    FACE::TSS::TRANSACTION_ID_TYPE transaction_id(0);
    FACE::TIMEOUT_TYPE timeout(0);

    _sample.entity_id = _message.entity_id;
    _sample.seq_num = _message.seq_num;
    _sample.timestamp_sec = _message.timestamp_sec;
    _sample.timestamp_usec = _message.timestamp_usec;
    _sample.latency_ping = _message.latency_ping;

    if (!DDS_OctetSeq_loan_contiguous(&_sample.bin_data,
                                     (DDS_Octet*) _message.data,
                                     _message.size, _message.size)) {
            fprintf(stderr, "bin_data.loan_contiguous() failed.\n");
            return false;
    }

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

    if (!DDS_OctetSeq_unloan(&_sample.bin_data)) {
        fprintf(stderr, "bin_data.unloan() failed.\n");
        return false;
    }

    return true;
}

template <class Type, class TypedTS>
inline TestMessage *TSSConnection<Type, TypedTS>::_receive()
{
    FACE::RETURN_CODE_TYPE::Value retcode;
    FACE::TSS::TRANSACTION_ID_TYPE transaction_id(0);
    FACE::TSS::QoS_EVENT_TYPE qos_parameters;
    FACE::TSS::HEADER_TYPE header;
    FACE::TIMEOUT_TYPE timeout(0);

    _typedTS->Receive_Message(_connection_id,
                              timeout,
                              transaction_id,
                              _sample,
                              header, /* not used */
                              qos_parameters, /* not used */
                              retcode);
    if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR)
	{
		fprintf(stderr, "!Failed Receive_Message (rc=%d)\n", retcode);
        return NULL;
	}

    _message.entity_id = _sample.entity_id;
    _message.seq_num = _sample.seq_num;
    _message.timestamp_sec = _sample.timestamp_sec;
    _message.timestamp_usec = _sample.timestamp_usec;
    _message.latency_ping = _sample.latency_ping;
    _message.size = _sample.bin_data.length();
    _message.data = (char *) _sample.bin_data.get_contiguous_buffer();

    return &_message;
}

template class RTITSSImpl<FACE::DM::TestData_t>;