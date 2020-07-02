

/*
WARNING: THIS FILE IS AUTO-GENERATED. IF MODIFIED, MOVE BEFORE RE-GENERATING.

This file was generated from perftest.idl using "rtiddsgen FACE TSS version".
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/rti_tss_common.h"

/* include RTI TSS header file for configuration data */
#include "config/ext_config.h"

/* include RTI TSS header file for logging configuration */
#include "log/ext_log.h"

extern struct RTI_TSS_QoSConfigPlugin* FACE_DM_TestData_t_get_qos_announcement(void);
extern struct RTI_TSS_QoSConfigPlugin* FACE_DM_TestData_t_get_qos_throughput(void);
extern struct RTI_TSS_QoSConfigPlugin* FACE_DM_TestData_t_get_qos_latency(void);

/*
* This code implements the function that provides
* 	configuration data to the RTI TSS library.  The function
* 	will be called by the RTI TSS library during the call
* 	to FACE_TS_Initialize()
*/
RTI_TSS_ExternalConfigData*
RTI_TSS_ConfigData_get_config_data(const char* config_name)
{
    static RTI_TSS_ExternalConfigData* config_data_g = 0;

    #if !FACE_COMPLIANCE_LEVEL_SAFETY_BASE_OR_STRICTER
    struct Logger* logger = Logger_getInstance();

    Logger_info(logger, __func__,
                "perftest providing configuration data to RTI TSS\n");
    #endif

    if (config_data_g == 0)
    {
        config_data_g = (RTI_TSS_ExternalConfigData*) malloc(sizeof(*config_data_g));
        memset(config_data_g, 0, sizeof(*config_data_g));

        /* set the logging verbosity to warning level */
        config_data_g->verbosity = LOG_VERBOSITY_WARNING;

        /* set the name of the QoS XML file that will be used by DDS */
        config_data_g->url_profile = "perftest_qos_profiles.xml";

        config_data_g->length = 3;
        config_data_g->config_elements_ptr = (RTI_TSS_ExternalConfigDataElement*)
        malloc(sizeof(*config_data_g->config_elements_ptr) * config_data_g->length);


        config_data_g->config_elements_ptr[0].connection_name = "Throughput";
        config_data_g->config_elements_ptr[0].direction_type = FACE_BI_DIRECTIONAL;
        config_data_g->config_elements_ptr[0].domain = 0;
        config_data_g->config_elements_ptr[0].topic_name = "Throughput";
        config_data_g->config_elements_ptr[0].type_name = "FACE::DM::TestData_t";
        config_data_g->config_elements_ptr[0].qos_plugin = FACE_DM_TestData_t_get_qos_throughput();
        #ifndef RTI_CONNEXT_MICRO
            config_data_g->config_elements_ptr[0].bi_dir_ignore_self = 1;
        #endif

        config_data_g->config_elements_ptr[1].connection_name = "latency";
        config_data_g->config_elements_ptr[1].direction_type = FACE_BI_DIRECTIONAL;
        config_data_g->config_elements_ptr[1].domain = 0;
        config_data_g->config_elements_ptr[1].topic_name = "latency";
        config_data_g->config_elements_ptr[1].type_name = "FACE::DM::TestData_t";
        config_data_g->config_elements_ptr[1].qos_plugin = FACE_DM_TestData_t_get_qos_latency();
        #ifndef RTI_CONNEXT_MICRO
            config_data_g->config_elements_ptr[1].bi_dir_ignore_self = 1;
        #endif

        config_data_g->config_elements_ptr[2].connection_name = "announcement";
        config_data_g->config_elements_ptr[2].direction_type = FACE_BI_DIRECTIONAL;
        config_data_g->config_elements_ptr[2].domain = 0;
        config_data_g->config_elements_ptr[2].topic_name = "announcement";
        config_data_g->config_elements_ptr[2].type_name = "FACE::DM::TestData_t";
        config_data_g->config_elements_ptr[2].qos_plugin = FACE_DM_TestData_t_get_qos_announcement();
        #ifndef RTI_CONNEXT_MICRO
            config_data_g->config_elements_ptr[2].bi_dir_ignore_self = 1;
        #endif
    }

    return config_data_g;
}
