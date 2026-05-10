#include "mqtt_cnn.h"
#include <iostream>
#include <syslog.h>
#include <cstring>

MqttPublisher::MqttPublisher(const std::string& address, int port, const std::string& topic)
    : broker_address(address), broker_port(port), topic(topic) {
    
    mosquitto_lib_init();
    // Create a new Mosquitto client instance with a clean session
    mosq = mosquitto_new("Pi4_Pet_Classifier", true, nullptr);
    if (!mosq) {
        std::cerr << "[MQTT ERROR] Failed to create Mosquitto instance." << std::endl;
        syslog(LOG_ERR, "Failed to create Mosquitto instance.");
    }
}

MqttPublisher::~MqttPublisher() {
    if (mosq) {
        mosquitto_destroy(mosq);
    }
    mosquitto_lib_cleanup();
}

bool MqttPublisher::connect() {
    if (!mosq) return false;

    std::cout << "\n[MQTT DEBUG] Attempting to connect to broker: " << broker_address << ":" << broker_port << std::endl;
    syslog(LOG_INFO, "Attempting MQTT connection to %s:%d", broker_address.c_str(), broker_port);

    int ret = mosquitto_connect(mosq, broker_address.c_str(), broker_port, 60);
    if (ret != MOSQ_ERR_SUCCESS) {
        std::cerr << "[MQTT ERROR] Connection failed: " << mosquitto_strerror(ret) << std::endl;
        syslog(LOG_ERR, "MQTT Connection failed: %s", mosquitto_strerror(ret));
        return false;
    }

    std::cout << "[MQTT DEBUG] Successfully connected to broker!" << std::endl;
    syslog(LOG_INFO, "Successfully connected to MQTT broker.");
    return true;
}

bool MqttPublisher::publish(const std::string& payload) {
    if (!mosq) return false;

    std::cout << "[MQTT DEBUG] Preparing to transmit data to topic: " << topic << std::endl;
    std::cout << "[MQTT DEBUG] Payload size: " << payload.length() << " bytes." << std::endl;
    syslog(LOG_INFO, "Publishing to topic %s", topic.c_str());

    int ret = mosquitto_publish(mosq, nullptr, topic.c_str(), payload.length(), payload.c_str(), 0, false);
    
    if (ret != MOSQ_ERR_SUCCESS) {
        std::cerr << "[MQTT ERROR] Publish failed: " << mosquitto_strerror(ret) << std::endl;
        syslog(LOG_ERR, "MQTT Publish failed: %s", mosquitto_strerror(ret));
        return false;
    }

    std::cout << "[MQTT DEBUG] Transmission successful!" << std::endl;
    syslog(LOG_INFO, "MQTT Transmission successful.");
    return true;
}
