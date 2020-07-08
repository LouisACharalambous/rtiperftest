

/*
WARNING: THIS FILE IS AUTO-GENERATED. IF MODIFIED, MOVE BEFORE RE-GENERATING.

This file was generated from perftest.idl using "rtiddsgen FACE TSS version".
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/rti_tss_common.h"
#include "perftest.h"

#include "ParameterManager.h"

/* include RTI TSS header file for configuration data */
#include "config/ext_config.h"

/* include RTI TSS header file for logging configuration */
#include "log/ext_log.h"

extern "C" {

extern struct RTI_TSS_QoSConfigPlugin* RTI_TSS_get_qos_announcement(ParameterManager *);
extern struct RTI_TSS_QoSConfigPlugin* RTI_TSS_get_qos_throughput(ParameterManager *);
extern struct RTI_TSS_QoSConfigPlugin* RTI_TSS_get_qos_latency(ParameterManager *);

/* No context can be passed to RTI_TSS_ConfigData_get_config_data, therefore,
 * we need to pass the PM setting this global variable at RTITSSImpl::Initialize
 */
ParameterManager *RTI_TSS_gv_pm = NULL;

void RTI_TSS_ConfigData_get_config(const char* role, const char* topic,
                                   RTI_TSS_ExternalConfigDataElement* config)
{
    char conn_name[64];
    sprintf(conn_name, "%s %s", role, topic);

    config->domain = RTI_TSS_gv_pm->get<int>("domain");
    config->topic_name = topic;
    config->type_name = "FACE::DM::TestData_t";
    config->connection_name = DDS_String_dup(conn_name);

    if (strcmp(topic, THROUGHPUT_TOPIC_NAME) == 0)
    {
        config->qos_plugin = RTI_TSS_get_qos_throughput(RTI_TSS_gv_pm);
    }
    else if (strcmp(topic, LATENCY_TOPIC_NAME) == 0)
    {
       config->qos_plugin = RTI_TSS_get_qos_latency(RTI_TSS_gv_pm);
    }
    else if (strcmp(topic, ANNOUNCEMENT_TOPIC_NAME) == 0)
    {
       config->qos_plugin = RTI_TSS_get_qos_announcement(RTI_TSS_gv_pm);
    }
    else
    {
#if !FACE_COMPLIANCE_LEVEL_SAFETY_BASE_OR_STRICTER
        Logger_error(Logger_getInstance(), __func__,
                     "Unknown topic %s\n", topic);
#endif
    }

    if (strcmp(role, "writer") == 0)
    {
        config->direction_type = FACE_SOURCE;
    }
    else if (strcmp(role, "reader") == 0)
    {
        config->direction_type = FACE_DESTINATION;
    }
    else
    {
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
    RTI_TSS_ExternalConfigData* config_data_g = NULL;

    if (RTI_TSS_gv_pm == NULL) {
        fprintf(stderr, "RTI_TSS_gv_pm was not set\n");
        return NULL;
    }

    if (config_data_g == NULL)
    {
        config_data_g = (RTI_TSS_ExternalConfigData*) malloc(sizeof(*config_data_g));
        memset(config_data_g, 0, sizeof(*config_data_g));

        /* set the logging verbosity to warning level */
        config_data_g->verbosity = LOG_VERBOSITY_INFO;

        /* set the name of the QoS XML file that will be used by DDS */
        config_data_g->url_profile = "perftest_qos_profiles.xml";

        config_data_g->length = 2 * 3;
        config_data_g->config_elements_ptr = (RTI_TSS_ExternalConfigDataElement*)
                malloc(sizeof(*config_data_g->config_elements_ptr) * config_data_g->length);

        /* Throughput */
        RTI_TSS_ConfigData_get_config("writer", THROUGHPUT_TOPIC_NAME, &config_data_g->config_elements_ptr[0]);
        RTI_TSS_ConfigData_get_config("reader", THROUGHPUT_TOPIC_NAME, &config_data_g->config_elements_ptr[1]);

        /* Latency */
        RTI_TSS_ConfigData_get_config("writer", LATENCY_TOPIC_NAME, &config_data_g->config_elements_ptr[2]);
        RTI_TSS_ConfigData_get_config("reader", LATENCY_TOPIC_NAME, &config_data_g->config_elements_ptr[3]);

        /* Announcement */
        RTI_TSS_ConfigData_get_config("writer", ANNOUNCEMENT_TOPIC_NAME, &config_data_g->config_elements_ptr[4]);
        RTI_TSS_ConfigData_get_config("reader", ANNOUNCEMENT_TOPIC_NAME, &config_data_g->config_elements_ptr[5]);
    }

    return config_data_g;
}

} // Extern C