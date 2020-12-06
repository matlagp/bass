#!/bin/bash

EXTERNAL_WIFI_CONFIG_FILE="/etc/wpa_supplicant/external_wifi.conf"
ACTIVE_WIFI_CONFIG_FILE="/etc/wpa_supplicant/wpa_supplicant.conf"
IF_NAME="wlan0"

INTERACTIVE=true
NUMBER_OF_RETRIES=0

help() {
cat << EOF
Usage: $0 [-h] [-c country] [-s ssid] [-p password]

Options:
  -h            Print this help message and exit
  -c country    Your country code, like US, GB, PL...
  -s ssid       Wireless network SSID
  -p password   Wireless network password
EOF
}

save_wifi_config() {
cat << EOF > "$EXTERNAL_WIFI_CONFIG_FILE"
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
country=${COUNTRY_CODE}

network={
	ssid="$WIFI_SSID"
	psk="$WIFI_PASSWORD"
}
EOF

cp "$EXTERNAL_WIFI_CONFIG_FILE" "$ACTIVE_WIFI_CONFIG_FILE"
}

while getopts "hc:s:p:" opt; do
  case $opt in
    c)
      COUNTRY_CODE=${OPTARG}
      ;;
    s)
      WIFI_SSID=${OPTARG}
      ;;
    p)
      WIFI_PASSWORD=${OPTARG}
      ;;
    h)
      help
      exit
      ;;
    *)
      help
      exit 1
      ;;
  esac
done

if [ -n "$COUNTRY_CODE" ] && [ -n "$WIFI_SSID" ] && [ -n "$WIFI_PASSWORD" ]; then
  INTERACTIVE=false
fi

while true ; do
  if [ -z "$COUNTRY_CODE" ]; then
    read -p "Enter your two-letter country code (eg. US, GB, PL): " COUNTRY_CODE
  fi

  if [ -z "$WIFI_SSID" ]; then
    read -p "Enter WiFi SSID: " WIFI_SSID
  fi

  if [ -z "$WIFI_PASSWORD" ]; then
    read -sp "Enter WiFi password: " WIFI_PASSWORD
  fi

  save_wifi_config
  wpa_cli -i "$IF_NAME" reconfigure 1>/dev/null

  while [ "$NUMBER_OF_RETRIES" -lt 3 ]; do
    echo
    echo "Trying to connect..."
    sleep 10s

    WIFI_STATUS=$(iwgetid)
    if [ -n "$WIFI_STATUS" ]; then
      echo "Connected!"
      iwconfig
      exit
    else
      echo "Unable to connect just yet"
      NUMBER_OF_RETRIES=$((NUMBER_OF_RETRIES+1))
    fi
  done

  echo "Could not connect to WiFi, max number of retries exceeded"
  echo "Ensure WiFi is reachable and try again"
  if [ "$INTERACTIVE" = false ]; then
    exit 1
  fi

  COUNTRY_CODE=""
  WIFI_SSID=""
  WIFI_PASSWORD=""
  NUMBER_OF_RETRIES=0
done
