#ifndef MQTT_CNN_H
#define MQTT_CNN_H

#include <string>
#include <mosquitto.h>

class MqttPublisher {
private:
    struct mosquitto *mosq;
    std::string broker_address;
    int broker_port;
    std::string topic;

public:
    MqttPublisher(const std::string& address, int port, const std::string& topic);
    ~MqttPublisher();
    
    bool connect();
    bool publish(const std::string& payload);
};

#endif // MQTT_CNN_H
