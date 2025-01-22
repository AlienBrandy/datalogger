/**
 * mqtt.c
 * 
 * SPDX-FileCopyrightText: Copyright © 2024 Honulanding Software <dev@honulanding.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mqtt.h"
#include "mqtt_client.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "datastream.h"
#include "jsmn.h"
#include "config.h"

/**
 * MQTT connection configuration
 * 
 * sample mosquitto command:
 * -q 1: QoS level 1
 * -h  : hostname
 * -d  : enable debug messages
 * -p  : port to connect with
 * -u  : username for authentication
 * -t  : topic on which to publish
 * -m  : message to send
 * mosquitto_pub -d -q 1 -h "mqtt.thingsboard.cloud" -p "1883" -t "v1/devices/me/telemetry" -u "$ACCESS_TOKEN" -m {"temperature":25}
 */
static const char* MQTT_PORT_TCP = "1883";
static const char* MQTT_PORT_TLS = "8883";

static esp_mqtt_client_handle_t client = NULL;
static bool connected = false;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(PROJECT_NAME, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(PROJECT_NAME, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(PROJECT_NAME, "MQTT_EVENT_CONNECTED");
        connected = true;
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(PROJECT_NAME, "MQTT_EVENT_DISCONNECTED");
        connected = false;
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(PROJECT_NAME, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(PROJECT_NAME, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(PROJECT_NAME, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
    {
        ESP_LOGI(PROJECT_NAME, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);

        static jsmn_parser parser;
        static jsmntok_t token[4];
        jsmn_init(&parser);
        int nTokens = jsmn_parse(&parser, event->data, event->data_len, token, 3);
        if (nTokens == 3)
        {
            char name[20] = {0};
            char val[20] = {0};
            strncpy(name, event->data + token[1].start, token[1].end - token[1].start);
            strncpy(val, event->data + token[2].start, token[2].end - token[2].start);
            ESP_LOGI(PROJECT_NAME, "name: %s value: %s", name, val);

            DATASTREAM_ERR_T retc = datastream_update_by_name(name, atof(val));
            ESP_LOGI(PROJECT_NAME, "update by name: %s", datastream_get_error_string(retc));
        }
        break;
    }
    case MQTT_EVENT_ERROR:
        ESP_LOGI(PROJECT_NAME, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(PROJECT_NAME, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(PROJECT_NAME, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_start_client(const char* broker, const char* access_token)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = broker,
        .credentials.username = access_token,
        .credentials.set_null_client_id = true,
    };

    client = esp_mqtt_client_init(&mqtt_cfg);

    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

bool mqtt_start(void)
{
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("transport_base", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("transport", ESP_LOG_VERBOSE);
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);

    const char* broker = NULL;
    const char* access_token = NULL;
    config_get(CONFIG_KEY_MQTT_BROKER, &broker);
    config_get(CONFIG_KEY_MQTT_ACCESS_TOKEN, &access_token);

    if (strlen(broker) == 0)
    {
        ESP_LOGW(PROJECT_NAME, "mqtt_start(): broker not defined.");
        return false;
    }
    if (strlen(access_token) == 0)
    {
        ESP_LOGW(PROJECT_NAME, "mqtt_start(): access token not defined.");
        return false;
    }
    mqtt_start_client(broker, access_token);
    return true;
}

void mqtt_publish(const char* topic, const char* key, const char* val)
{
    if (client == NULL)
    {
        return;
    }
    if (!connected)
    {
        return;
    }
    char data[128];
    snprintf(data, 50, "{\"%s\":\"%s\"}", key, val);
    ESP_LOGI(PROJECT_NAME, "publishing %s to %s", data, topic);
    int msg_id = esp_mqtt_client_publish(client, topic, data, 0, 1, 0);
    ESP_LOGI(PROJECT_NAME, "sent publish successful, msg_id=%d", msg_id);
}

void mqtt_subscribe(char* topic)
{
    if (client == NULL)
    {
        return;
    }
    int msg_id = esp_mqtt_client_subscribe(client, topic, 0);
    ESP_LOGI(PROJECT_NAME, "sent subscribe successful, msg_id=%d", msg_id);
}