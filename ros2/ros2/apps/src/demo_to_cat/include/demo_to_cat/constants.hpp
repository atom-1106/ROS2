#ifndef DEMO_CONSTANTS_HPP_
#define DEMO_CONSTANTS_HPP_

#include <string>

namespace demo {

// Publisher/Subscriber/Topic:
// APP_PUBSUB: Publisher and Subscriber, both present in same Node
static const std::string APP_PUBSUB_PUB_NODE_NAME = "app_pubsub_pub_node";
static const std::string APP_PUBSUB_SUB_NODE_NAME = "app_pubsub_sub_node";
static const std::string APP_PUBSUB_TOPIC_NAME = "app_pubsub_topic";

// APP_PUB, APP_SUB, TOPIC: APP_PS
static const std::string APP_PUB_NODE_NAME = "app_pub_node";
static const std::string APP_SUB_NODE_NAME = "app_sub_node";
static const std::string APP_PS_TOPIC_NAME = "ps_topic";

// SERVICES: APP_SERVER, APP_CLIENT
static const std::string APP_SERVER_NODE_NAME = "app_server_node";
static const std::string APP_CLIENT_NODE_NAME = "app_client_node";
// SERVICE_NAME
static const std::string APP_SC_SERVICE_NAME = "app_sc_sensor_calibrator";

// ACTION SERVER, CLIENT APP
static const std::string APP_ACTION_SERVER_NODE_NAME = "app_action_server_node";
static const std::string APP_ACTION_CLIENT_NODE_NAME = "app_action_client_node";
static const std::string APP_SC_ACTION_NAME = "app_sc_update_firmware";

// DUAL THREADED APP
static const std::string APP_DUALSUB_PUB_NODE_NAME = "app_dualsub_pub_node";
static const std::string APP_DUALSUB_NODE_NAME = "app_dualsub_node";
static const std::string APP_DUALSUB_TOPIC_NAME = "app_dualsub_topic";

// Use case:
// Running below applications in three different domains
static const std::string APP_SUB_CLIENT_NODE_NAME = "app_sub_serviceclient_node";
static const std::string APP_SUB_ACSER_NODE_NAME = "app_sub_acser_node";
static const std::string APP_PUB_SER_ACCLI_NODE_NAME = "app_pub_serviceserver_accli_node";
static const std::string APP_PS_CS_ASC_TOPIC_NAME = "app_ps_cs_asc_vehicle_topic";
static const std::string APP_PS_CS_ASC_SERVICE_NAME = "app_ps_cs_asc_sensor_calibrator";
static const std::string APP_PS_CS_ASC_ACTION_NAME = "app_ps_cs_asc_fibonacci";


// sensors constants
static const std::string SENSOR_STEERING_ANGLE = "steering_angle";
static const std::string SENSOR_TEMPERATURE = "temperature";

}

#endif // DEMO_CONSTANTS_HPP_