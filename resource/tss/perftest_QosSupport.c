

/*
WARNING: THIS FILE IS AUTO-GENERATED. IF MODIFIED, MOVE BEFORE RE-GENERATING.

This file was generated from perftest.idl using "rtiddsgen FACE TSS version".
*/

/* include RTI TSS header file for QoS configuration */
#include "qos_support/qos_plugin.h"

#include "util/rti_tss_common.h"

#ifdef RTI_CONNEXT_MICRO
#include "rti_me_c.h"
#include "disc_dpde/disc_dpde_discovery_plugin.h"
#include "wh_sm/wh_sm_history.h"
#include "rh_sm/rh_sm_history.h"
#include "netio/netio_udp.h"
#endif

#include "log/ext_log.h"

/* function for customizing domain participant qos */
DDS_Boolean
FACE_DM_TestData_t_participant_qos(
    struct DDS_DomainParticipantQos* dp_qos,
    void* plugin_data)
{
    /* the argument qos structure will already contain the QoS values loaded from the
    * profile "HelloWorld_Library::HelloWorld_Profile"
    */

    /* Discovery announcements to loopback and Connext default multicast */
    DDS_StringSeq_set_maximum(&(dp_qos->discovery.initial_peers),2);
    DDS_StringSeq_set_length(&(dp_qos->discovery.initial_peers),2);

    *DDS_StringSeq_get_reference(&(dp_qos->discovery.initial_peers),0) =
    DDS_String_dup("127.0.0.1");
    *DDS_StringSeq_get_reference(&(dp_qos->discovery.initial_peers),1) =
    DDS_String_dup("239.255.0.1");

    #ifndef RTI_CONNEXT_MICRO
    /* disable shared memory for Connext Pro */
    dp_qos->transport_builtin.mask = DDS_TRANSPORTBUILTIN_UDPv4;
    #endif

    #ifdef RTI_CONNEXT_MICRO
    RT_Registry_T *registry = NULL;
    struct UDP_InterfaceFactoryProperty *udp_property = NULL;
    struct DPDE_DiscoveryPluginProperty discovery_plugin_properties =
    DPDE_DiscoveryPluginProperty_INITIALIZER;

    #if !FACE_COMPLIANCE_LEVEL_SAFETY_BASE_OR_STRICTER
    struct Logger* logger = Logger_getInstance();
    #endif

    registry = DDS_DomainParticipantFactory_get_registry(
        DDS_DomainParticipantFactory_get_instance());

    if ((RT_Registry_lookup(registry,
    DDSHST_WRITER_DEFAULT_HISTORY_NAME) == NULL) &&
    !RT_Registry_register(registry, DDSHST_WRITER_DEFAULT_HISTORY_NAME,
    WHSM_HistoryFactory_get_interface(), NULL, NULL))
    {
        Logger_error(logger, __func__, "failed to register writer history\n");
        return DDS_BOOLEAN_FALSE;
    }

    if ((RT_Registry_lookup(registry,
    DDSHST_READER_DEFAULT_HISTORY_NAME) == NULL) &&
    !RT_Registry_register(registry, DDSHST_READER_DEFAULT_HISTORY_NAME,
    RHSM_HistoryFactory_get_interface(), NULL, NULL))
    {
        Logger_error(logger, __func__, "failed to register reader history\n");
        return DDS_BOOLEAN_FALSE;
    }

    /* Configure UDP transports allowed interfaces */
    if ((RT_Registry_lookup(registry, NETIO_DEFAULT_UDP_NAME) != NULL) &&
    !RT_Registry_unregister(registry, NETIO_DEFAULT_UDP_NAME, NULL, NULL))
    {
        Logger_error(logger, __func__, "failed to unregister udp\n");
        return DDS_BOOLEAN_FALSE;
    }

    udp_property = (struct UDP_InterfaceFactoryProperty *)
    malloc(sizeof(struct UDP_InterfaceFactoryProperty));
    *udp_property = UDP_INTERFACE_FACTORY_PROPERTY_DEFAULT;

    /*  */
    #if FACE_COMPLIANCE_LEVEL_SAFETY_BASE_OR_STRICTER

    /* Safety Base or stricter, with Micro, must manually configure available
    * interfaces.
    * First we disable reading out the interface list. Note that on some
    * platforms reading out the interface list has been compiled out, so
    * this property could have no effect.
    */
    udp_property->disable_auto_interface_config = RTI_TRUE;

    REDA_StringSeq_set_maximum(&udp_property->allow_interface,1);
    REDA_StringSeq_set_length(&udp_property->allow_interface,1);

    /* The name of the interface can be the anything, up to
    * UDP_INTERFACE_MAX_IFNAME characters including the '\0' character
    */
    *DDS_StringSeq_get_reference(&udp_property->allow_interface,0) =
    DDS_String_dup("loopback");

    /* This function takes the following arguments:
    * Param 1 is the iftable in the UDP property
    * Param 2 is the IP address of the interface in host order
    * Param 3 is the Netmask of the interface
    * Param 4 is the name of the interface
    * Param 5 are flags. The following flags are supported for Security and
    *    Safety Base (use OR for multiple):
        *        UDP_INTERFACE_INTERFACE_UP_FLAG - Interface is up
        *
        *    The following flags are supported for non-Security and non-Safety
        *    Base:
        *        UDP_INTERFACE_INTERFACE_MULTICAST_FLAG - Interface supports multicast
        */
        RTI_BOOL result;
        result = UDP_InterfaceTable_add_entry(&udp_property->if_table,
        0x7f000001,0xff000000,"loopback",
        UDP_INTERFACE_INTERFACE_UP_FLAG);

        if (!result)
        {
            Logger_error(logger, __func__, "failed to add UDP table entry\n");
            printf("failed UDP table add entry!\n");
            return DDS_BOOLEAN_FALSE;
    }

    #else

    /* For additional allowed interface(s), increase maximum and length, and
    * set interface below.
    */
    REDA_StringSeq_set_maximum(&udp_property->allow_interface,2);
    REDA_StringSeq_set_length(&udp_property->allow_interface,2);

    /* loopback interface */
    #if defined (RTI_LINUX)
    *REDA_StringSeq_get_reference(&udp_property->allow_interface,0) = "lo";
    #elif defined (RTI_VXWORKS)
    *REDA_StringSeq_get_reference(&udp_property->allow_interface,0) = "lo0";
    #elif defined(RTI_WIN32)
    *REDA_StringSeq_get_reference(&udp_property->allow_interface,0) =
    "Loopback Pseudo-Interface 1";
    #else
    *REDA_StringSeq_get_reference(&udp_property->allow_interface,0) = "lo";
    #endif
    #if defined (RTI_LINUX)
    *REDA_StringSeq_get_reference(&udp_property->allow_interface,1) = "enp0s3";
    #elif defined (RTI_VXWORKS)
    *REDA_StringSeq_get_reference(&udp_property->allow_interface,1) = "geisc0";
    #elif defined(RTI_WIN32)
    *REDA_StringSeq_get_reference(&udp_property->allow_interface,1) =
    "Local Area Connection";
    #else
    *REDA_StringSeq_get_reference(&udp_property->allow_interface,1) = "ce0";
    #endif

    #endif /* FACE_COMPLIANCE_LEVEL_SAFETY_BASE_OR_STRICTER */

    if (!RT_Registry_register(registry, NETIO_DEFAULT_UDP_NAME,
    UDP_InterfaceFactory_get_interface(),
    (struct RT_ComponentFactoryProperty*)udp_property, NULL))
    {
        Logger_error(logger, __func__, "failed to register udp\n");
        return DDS_BOOLEAN_FALSE;
    }

    if ((RT_Registry_lookup(registry, "dpde") == NULL) &&
    !RT_Registry_register(registry,
    "dpde",
    DPDE_DiscoveryFactory_get_interface(),
    &discovery_plugin_properties._parent,
    NULL))
    {
        Logger_error(logger, __func__, "failed to register dpde\n");
        return DDS_BOOLEAN_FALSE;
    }

    if (!RT_ComponentFactoryId_set_name(&(dp_qos->discovery.discovery.name),"dpde"))
    {
        Logger_error(logger, __func__, "failed to set discovery plugin name\n");
        return DDS_BOOLEAN_FALSE;
    }

    sprintf(dp_qos->participant_name.name,"RTI FACE Example");

    /* if there are more remote or local endpoints, you need to increase these limits */
    dp_qos->resource_limits.local_writer_allocation = 10;
    dp_qos->resource_limits.local_reader_allocation = 10;
    dp_qos->resource_limits.local_publisher_allocation = 10;
    dp_qos->resource_limits.local_subscriber_allocation = 10;
    dp_qos->resource_limits.local_topic_allocation = 10;
    dp_qos->resource_limits.local_type_allocation = 10;
    dp_qos->resource_limits.remote_participant_allocation = 80;
    dp_qos->resource_limits.remote_writer_allocation = 80;
    dp_qos->resource_limits.remote_reader_allocation = 80;
    dp_qos->resource_limits.matching_writer_reader_pair_allocation = 30;
    dp_qos->resource_limits.matching_reader_writer_pair_allocation = 30;
    dp_qos->resource_limits.max_destination_ports = 100;
    dp_qos->resource_limits.max_receive_ports = 100;
    #endif

    #if FACE_COMPLIANCE_LEVEL_SAFETY_BASE_OR_STRICTER
    /* Safety Base or stricter must use non-auto DDS participant ID
    * ATTENTION: the participant_id below _must_ be modified to be unique
    * between all applications. Otherwise, new participants with the same ID
    * with an existing participant will fail to be created.
    */
    dp_qos->protocol.participant_id = 1;
    #endif /* FACE_COMPLIANCE_LEVEL_SAFETY_BASE_OR_STRICTER */

    return DDS_BOOLEAN_TRUE;
}

/* function for customizing publisher qos */
DDS_Boolean
FACE_DM_TestData_t_publisher_qos(
    struct DDS_PublisherQos* pub_qos,
    void* plugin_data)
{
    #ifndef RTI_CONNEXT_MICRO
    /* For Connext Pro
    * the argument qos structure will already contain the QoS values loaded from the
    * profile "HelloWorld_Library::HelloWorld_Profile"
    */

    /* example to increase the length of the partition qos */
    /*
    DDS_Boolean result = DDS_BOOLEAN_TRUE;
    int seq_length = DDS_StringSeq_get_length(&pub_qos->partition.name);
    char** partition = NULL;

    #if !FACE_COMPLIANCE_LEVEL_SAFETY_BASE_OR_STRICTER
    struct Logger* logger = Logger_getInstance();
    #endif

    result = DDS_StringSeq_ensure_length(&pub_qos->partition.name,
    seq_length + 1, seq_length + 1);
    if (!result)
    {
        Logger_error(
            logger,
            __func__,
            "!Unable to set the sequence length for publisher qos (%d=>%d)\n",
            seq_length,
            seq_length+1);
        return DDS_BOOLEAN_FALSE;
    }

    partition = DDS_StringSeq_get_reference(&pub_qos->partition.name, seq_length);
    *partition = DDS_String_dup("my partition");
    */
    #else
    DDS_PublisherQos_initialize(pub_qos);
    #endif

    return DDS_BOOLEAN_TRUE;
}

/* function for customizing subscriber qos */
DDS_Boolean
FACE_DM_TestData_t_subscriber_qos(
    struct DDS_SubscriberQos* sub_qos,
    void* plugin_data)
{
    #ifndef RTI_CONNEXT_MICRO
    /* For Connext Pro
    * the argument qos structure will already contain the QoS values loaded from the
    * profile "HelloWorld_Library::HelloWorld_Profile"
    */

    /*example to  increase the length of the partition qos */
    /*
    DDS_Boolean result = DDS_BOOLEAN_TRUE;
    int seq_length = DDS_StringSeq_get_length(&sub_qos->partition.name);
    char** partition = NULL;
    struct Logger* logger = Logger_getInstance();

    result = DDS_StringSeq_ensure_length(&sub_qos->partition.name,
    seq_length + 1, seq_length + 1);
    if (!result)
    {
        Logger_error(
            logger,
            __func__,
            "!Unable to set the sequence length for subscriber qos (%d=>%d)\n",
            seq_length,
            seq_length + 1);
        return DDS_BOOLEAN_FALSE;
    }

    partition = DDS_StringSeq_get_reference(&sub_qos->partition.name, seq_length);
    *partition = DDS_String_dup("my partition");
    */
    #else
    DDS_SubscriberQos_initialize(sub_qos);
    #endif

    return DDS_BOOLEAN_TRUE;
}

/* function for customizing topic qos */
DDS_Boolean
FACE_DM_TestData_t_topic_qos(
    struct DDS_TopicQos* topic_qos,
    void* plugin_data)
{
    /* For Connext Pro
    * the argument qos structure will already contain the QoS values loaded from the
    * profile "HelloWorld_Library::HelloWorld_Profile"
    */
    #ifdef RTI_CONNEXT_MICRO
    DDS_TopicQos_initialize(topic_qos);
    #endif
    return DDS_BOOLEAN_TRUE;
}

/* function for customizing data writer qos */
DDS_Boolean
FACE_DM_TestData_t_datawriter_qos(
    struct DDS_DataWriterQos* writer_qos,
    void* plugin_data)
{

    /* For Connext Pro
    * the argument qos structure will already contain the QoS values loaded from the
    * profile "HelloWorld_Library::HelloWorld_Profile"
    */

    #ifdef RTI_CONNEXT_MICRO
    /* set qos for best effort communication without keys
    writer_qos->reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
    writer_qos->resource_limits.max_samples = 1;
    writer_qos->resource_limits.max_samples_per_instance = 1;
    writer_qos->resource_limits.max_instances = 1;
    writer_qos->history.depth = 1; */
    /* use these instead if reliable (also without keys) */
    writer_qos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    writer_qos->resource_limits.max_samples_per_instance = 200;
    writer_qos->resource_limits.max_instances = 20;
    writer_qos->resource_limits.max_samples =
    writer_qos->resource_limits.max_instances *
    writer_qos->resource_limits.max_samples_per_instance;
    writer_qos->history.depth = 32;
    writer_qos->protocol.rtps_reliable_writer.heartbeat_period.sec = 0;
    writer_qos->protocol.rtps_reliable_writer.heartbeat_period.nanosec = 250000000;
    #endif

    return DDS_BOOLEAN_TRUE;
}

/* function for customizing data reader qos */
DDS_Boolean
FACE_DM_TestData_t_datareader_qos(
    struct DDS_DataReaderQos* reader_qos,
    void* plugin_data)
{
    /* For Connext Pro
    * the argument qos structure will already contain the QoS values loaded from the
    * profile "HelloWorld_Library::HelloWorld_Profile"
    */
    #ifdef RTI_CONNEXT_MICRO
    /* set qos for best effort communication without keys */
    reader_qos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    reader_qos->resource_limits.max_instances = 20;
    reader_qos->resource_limits.max_samples_per_instance = 200;
    reader_qos->resource_limits.max_samples =
    reader_qos->resource_limits.max_instances *
    reader_qos->resource_limits.max_samples_per_instance;
    reader_qos->history.depth = 32;

    /* if there are more remote writers, increase these limits */
    reader_qos->reader_resource_limits.max_remote_writers = 30;
    reader_qos->reader_resource_limits.max_remote_writers_per_instance = 30;
    #endif

    return DDS_BOOLEAN_TRUE;
}

/* function to return the plugin with references to the above functions */
struct RTI_TSS_QoSConfigPlugin*
FACE_DM_TestData_t_get_qos_throughput()
{
    static RTI_TSS_QoSConfigPlugin* QoS_plugin_g = NULL;

    if (QoS_plugin_g == NULL)
    {
        QoS_plugin_g = (RTI_TSS_QoSConfigPlugin*)malloc(sizeof(*QoS_plugin_g));

        memset(QoS_plugin_g, 0, sizeof(*QoS_plugin_g));

        QoS_plugin_g->configure_domain_participant_qos_fnc =
                FACE_DM_TestData_t_participant_qos;
        QoS_plugin_g->configure_publisher_qos_fnc =
                FACE_DM_TestData_t_publisher_qos;
        QoS_plugin_g->configure_subscriber_qos_fnc =
                FACE_DM_TestData_t_subscriber_qos;
        QoS_plugin_g->configure_topic_qos_fnc =
                FACE_DM_TestData_t_topic_qos;
        QoS_plugin_g->configure_datawriter_qos_fnc =
                FACE_DM_TestData_t_datawriter_qos;
        QoS_plugin_g->configure_datareader_qos_fnc =
                FACE_DM_TestData_t_datareader_qos;

        QoS_plugin_g->qos_library = "PerftestQosLibrary";
        QoS_plugin_g->qos_profile = "ThroughputQos";
        QoS_plugin_g->plugin_data = NULL;
    }

    return QoS_plugin_g;
}

struct RTI_TSS_QoSConfigPlugin*
FACE_DM_TestData_t_get_qos_latency()
{
    static RTI_TSS_QoSConfigPlugin* QoS_plugin_g = NULL;

    if (QoS_plugin_g == NULL)
    {
        QoS_plugin_g = (RTI_TSS_QoSConfigPlugin*)malloc(sizeof(*QoS_plugin_g));

        memset(QoS_plugin_g, 0, sizeof(*QoS_plugin_g));

        QoS_plugin_g->configure_domain_participant_qos_fnc =
                FACE_DM_TestData_t_participant_qos;
        QoS_plugin_g->configure_publisher_qos_fnc =
                FACE_DM_TestData_t_publisher_qos;
        QoS_plugin_g->configure_subscriber_qos_fnc =
                FACE_DM_TestData_t_subscriber_qos;
        QoS_plugin_g->configure_topic_qos_fnc =
                FACE_DM_TestData_t_topic_qos;
        QoS_plugin_g->configure_datawriter_qos_fnc =
                FACE_DM_TestData_t_datawriter_qos;
        QoS_plugin_g->configure_datareader_qos_fnc =
                FACE_DM_TestData_t_datareader_qos;

        QoS_plugin_g->qos_library = "PerftestQosLibrary";
        QoS_plugin_g->qos_profile = "LatencyQos";
        QoS_plugin_g->plugin_data = NULL;
    }

    return QoS_plugin_g;
}

struct RTI_TSS_QoSConfigPlugin*
FACE_DM_TestData_t_get_qos_announcement()
{
    static RTI_TSS_QoSConfigPlugin* QoS_plugin_g = NULL;

    if (QoS_plugin_g == NULL)
    {
        QoS_plugin_g = (RTI_TSS_QoSConfigPlugin*)malloc(sizeof(*QoS_plugin_g));

        memset(QoS_plugin_g, 0, sizeof(*QoS_plugin_g));

        QoS_plugin_g->configure_domain_participant_qos_fnc =
                FACE_DM_TestData_t_participant_qos;
        QoS_plugin_g->configure_publisher_qos_fnc =
                FACE_DM_TestData_t_publisher_qos;
        QoS_plugin_g->configure_subscriber_qos_fnc =
                FACE_DM_TestData_t_subscriber_qos;
        QoS_plugin_g->configure_topic_qos_fnc =
                FACE_DM_TestData_t_topic_qos;
        QoS_plugin_g->configure_datawriter_qos_fnc =
                FACE_DM_TestData_t_datawriter_qos;
        QoS_plugin_g->configure_datareader_qos_fnc =
                FACE_DM_TestData_t_datareader_qos;

        QoS_plugin_g->qos_library = "PerftestQosLibrary";
        QoS_plugin_g->qos_profile = "AnnouncementQos";
        QoS_plugin_g->plugin_data = NULL;
    }

    return QoS_plugin_g;
}
