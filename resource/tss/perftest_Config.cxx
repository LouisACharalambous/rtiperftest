

/*
WARNING: THIS FILE IS AUTO-GENERATED. IF MODIFIED, MOVE BEFORE RE-GENERATING.

This file was generated from perftest.idl using "rtiddsgen FACE TSS version".
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/rti_tss_common.h"
#include "perftest.h"

#include "Infrastructure_common.h"
#include "ParameterManager.h"

/* include RTI TSS header file for configuration data */
#include "config/ext_config.h"

/* include RTI TSS header file for logging configuration */
#include "log/ext_log.h"

extern "C" {

extern struct RTI_TSS_QoSConfigPlugin* RTI_TSS_get_qos_announcement();
extern struct RTI_TSS_QoSConfigPlugin* RTI_TSS_get_qos_throughput();
extern struct RTI_TSS_QoSConfigPlugin* RTI_TSS_get_qos_latency();
extern ParameterManager *RTI_TSS_gv_pm;

void RTI_TSS_ConfigData_get_config(std::string role, std::string topic,
                                   RTI_TSS_ExternalConfigDataElement* config)
{
    config->domain = RTI_TSS_gv_pm->get<int>("domain");
    config->topic_name = DDS_String_dup(topic.c_str());
    config->type_name = "FACE::DM::TestData_t";
    config->connection_name = DDS_String_dup((role + " " + topic).c_str());

    if (topic == THROUGHPUT_TOPIC_NAME) {
        config->qos_plugin = RTI_TSS_get_qos_throughput();
    } else if (topic == LATENCY_TOPIC_NAME) {
       config->qos_plugin = RTI_TSS_get_qos_latency();
    } else if (topic == ANNOUNCEMENT_TOPIC_NAME) {
       config->qos_plugin = RTI_TSS_get_qos_announcement();
    } else {
#if !FACE_COMPLIANCE_LEVEL_SAFETY_BASE_OR_STRICTER
        Logger_error(Logger_getInstance(), __func__,
                     "Unknown topic %s\n", topic.c_str());
#endif
    }

    if (role == "writer") {
        config->direction_type = FACE_SOURCE;
    } else if (role == "reader") {
        config->direction_type = FACE_DESTINATION;
    } else {
        config->direction_type = FACE_BI_DIRECTIONAL;
#ifndef RTI_CONNEXT_MICRO
        config->bi_dir_ignore_self = 1;
#endif
    }
}

/*
* This code implements the function that provides
* 	configuration data to the RTI TSS library.  The function
* 	will be called by the RTI TSS library during the call
* 	to FACE_TS_Initialize()
*/
RTI_TSS_ExternalConfigData*
RTI_TSS_ConfigData_get_config_data(const char *config_name)
{
    RTI_TSS_ExternalConfigData* config_g = NULL;

    if (RTI_TSS_gv_pm == NULL) {
        fprintf(stderr, "RTI_TSS_gv_pm was not set\n");
        return NULL;
    }

    if (config_g == NULL)
    {
        config_g = new RTI_TSS_ExternalConfigData;
        memset(config_g, 0, sizeof(*config_g));

        /* set the logging verbosity to warning level */
        config_g->verbosity = LOG_VERBOSITY_INFO;

        /* set the name of the QoS XML file that will be used by DDS */
        config_g->url_profile = DDS_String_dup(RTI_TSS_gv_pm->get<std::string>("qosFile").c_str());

        config_g->length = 2 * 3;
        config_g->config_elements_ptr = new RTI_TSS_ExternalConfigDataElement[config_g->length];

        /* Throughput */
        RTI_TSS_ConfigData_get_config("writer", THROUGHPUT_TOPIC_NAME, &config_g->config_elements_ptr[0]);
        RTI_TSS_ConfigData_get_config("reader", THROUGHPUT_TOPIC_NAME, &config_g->config_elements_ptr[1]);

        /* Latency */
        RTI_TSS_ConfigData_get_config("writer", LATENCY_TOPIC_NAME, &config_g->config_elements_ptr[2]);
        RTI_TSS_ConfigData_get_config("reader", LATENCY_TOPIC_NAME, &config_g->config_elements_ptr[3]);

        /* Announcement */
        RTI_TSS_ConfigData_get_config("writer", ANNOUNCEMENT_TOPIC_NAME, &config_g->config_elements_ptr[4]);
        RTI_TSS_ConfigData_get_config("reader", ANNOUNCEMENT_TOPIC_NAME, &config_g->config_elements_ptr[5]);
    }

    return config_g;
}

} // Extern C