#!/bin/bash

echo "Starting MQTT Monitor Service installation..."

if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (use sudo)"
    exit 1
fi

cd /home/useros/workshop/mqtt

if [ ! -f "mqtt_app" ]; then
    echo "Building application..."
    make clean
    make
    if [ ! -f "mqtt_app" ]; then
        echo "Failed to build mqtt_app"
        exit 1
    fi
fi

echo "Setting permissions..."
chown -R useros:useros .
chmod -R 755 .
chmod +x mqtt_app
chmod 777 .

if systemctl is-active --quiet mqtt-monitor.service; then
    echo "Stopping existing service..."
    systemctl stop mqtt-monitor.service
    systemctl disable mqtt-monitor.service
fi

echo "Installing service file..."
cp mqtt-monitor.service /etc/systemd/system/

echo "Reloading systemd..."
systemctl daemon-reload

echo "Starting service..."
systemctl enable mqtt-monitor.service
systemctl start mqtt-monitor.service

echo "Checking service status..."
systemctl status mqtt-monitor.service

echo "Installation complete. Use control_service.sh to manage the service."
