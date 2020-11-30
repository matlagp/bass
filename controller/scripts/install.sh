#!/bin/bash

BASS_BASE_PATH="/opt/bass"

SETUP_WIFI="n"
read -p "Do you want to configure your WiFi now? [y/N]: " SETUP_WIFI
if [ "$SETUP_WIFI" = "y" ] || [ "$SETUP_WIFI" = 'Y' ]; then
  bash "${BASS_BASE_PATH}/scripts/wifi_setup.sh"
fi

# Add Mopidy repository
wget -q -O - https://apt.mopidy.com/mopidy.gpg | sudo apt-key add -

# Install dependencies
apt update
apt install mosquitto mosquitto-clients python3-pip python-dev python3-dev libbluetooth-dev gstreamer1.0 gstreamer1.0-tools mopidy
python3 -m pip install flask paho-mqtt pybluez pycodestyle python-dotenv

# Load ALSA loopback device on startup
echo "snd-aloop" >> /etc/modules
modprobe snd-aloop

# Copy unit files
cp "${BASS_BASE_PATH}/bass-server.service" /lib/systemd/system
cp "${BASS_BASE_PATH}/bass-streamer.service" /lib/systemd/system

# Enable services
systemctl enable mosquitto
systemctl start mosquitto
systemctl enable mopidy
systemctl start mopidy
systemctl enable bass-server
systemctl start bass-server
systemctl enable bass-streamer
systemctl start bass-streamer
