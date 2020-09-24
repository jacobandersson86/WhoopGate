#ifdef __cplusplus
extern "C" {
#endif

#ifndef MQTT_H
#define MQTT_H

void mqtt_start_client(void);
void mqtt_subscribe_topics(void);
void mqtt_publish_id(void);
void mqtt_stop_client(void);

#endif //MQTT_H

#ifdef __cplusplus
}
#endif