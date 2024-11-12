#!/bin/bash

if [ ! -f "/etc/systemd/system/mqtt-monitor.service" ]; then
    echo "MQTT Monitor service is not installed."
    echo "Please run install_service.sh first."
    exit 1
fi

check_status() {
    if systemctl is-active --quiet mqtt-monitor.service; then
        echo "Service is running"
        return 0
    else
        echo "Service is not running"
        return 1
    fi
}

case "$1" in
    start)
        echo "Starting MQTT Monitor service..."
        sudo systemctl start mqtt-monitor.service
        check_status
        ;;
    stop)
        echo "Stopping MQTT Monitor service..."
        sudo systemctl stop mqtt-monitor.service
        check_status
        ;;
    restart)
        echo "Restarting MQTT Monitor service..."
        sudo systemctl restart mqtt-monitor.service
        check_status
        ;;
    status)
        echo "Checking MQTT Monitor service status..."
        sudo systemctl status mqtt-monitor.service
        ;;
    log)
        echo "Showing MQTT Monitor service logs..."
        sudo journalctl -u mqtt-monitor.service -f
        ;;
    check)
        check_status
        ;;
    *)
        echo "Usage: $0 {start|stop|restart|status|log|check}"
        exit 1
        ;;
esac

exit 0
