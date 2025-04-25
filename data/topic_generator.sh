#!/bin/bash

currencies=(
	"USD" "EUR" "JPY" "GBP" "CNY" "AUD" "CAD" "CHF"
	"HKD" "SGD" "SEK" "KRW" "NOK" "NZD" "INR" "MXN"
	"TWD" "ZAR" "BRL" "DKK" "PLN" "THB" "ILS" "IDR"
	"CZK" "AED" "TRY" "HUF" "CLP" "SAR" "PHP" "MYR"
	"COP" "RUB" "RON" "PEN"
);

for c in ${currencies[@]}; do
	echo "$c"
done