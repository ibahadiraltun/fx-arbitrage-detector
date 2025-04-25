import confluent_kafka.admin as KafkaAdmin


admin_client = KafkaAdmin.AdminClient({
    "bootstrap.servers": "localhost:50761"
})

topic_list = []
topic_list.append(KafkaAdmin.NewTopic("example_topic2", 1, 1))
admin_client.create_topics(topic_list)
