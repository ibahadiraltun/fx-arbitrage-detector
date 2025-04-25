import confluent_kafka.admin as KafkaAdmin
from confluent_kafka import Producer
import requests

currencies = [
    "usd", "eur", "jpy", "gbp", "cny", "aud", "cad", "chf",
    "hkd", "sgd", "sek", "krw", "nok", "nzd", "inr", "mxn",
    "twd", "zar", "brl", "dkk", "pln", "thb", "ils", "idr",
    "czk", "aed", "try", "huf", "clp", "sar", "php", "myr",
    "cop", "rub", "ron", "pen"
]

topic_name = "exchange-rates"
config = {
    "bootstrap.servers": "localhost:50761"
}


def callback(err, msg):
    if err:
        print('ERROR: Message failed delivery: {}'.format(err))
    else:
        print("Produced event to topic {topic}: key = {key} value = {value} partition = {partition}".format(
            topic=msg.topic(), key=msg.key().decode('utf-8'), value=msg.value().decode('utf-8'), partition=msg.partition()))


def fetch_rate(currency='usd'):
    endpoint = 'https://cdn.jsdelivr.net/npm/@fawazahmed0/currency-api@latest/v1/currencies/' + f'{currency}.min.json'
    return requests.get(endpoint).json()


producer = Producer({**config, "acks": "all"})
idx = 0
while True:
    for base_currency in currencies:
        res = fetch_rate(base_currency)
        timestamp = res['date']
        rates = res[base_currency]
        for target_currency, rate in rates.items():
            producer.produce(
                topic=topic_name,
                value=str(rate),
                key=f"{base_currency}:{target_currency}",
                partition=idx,
                callback=callback
            )
        idx = (idx + 1) % 36
        producer.poll(1000)

