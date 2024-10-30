#!/bin/bash

# Default values
NUM_LEDS=60
UNIVERSE=0
START_ADDRESS=1
DEVICE_NAME="ArtNet_LED_Strip"
WIFI_SSID="SomeSSID"
WIFI_PASSWORD="mypassword123"
WIFI_PRIORITY=0

# Function to print usage
print_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  -n, --num-leds NUM       Number of LEDs (default: 60)"
    echo "  -u, --universe NUM       DMX universe (default: 0)"
    echo "  -a, --address NUM        Start address (default: 1)"
    echo "  -d, --device-name NAME   Device name (default: ArtNet_LED_Strip)"
    echo "  -s, --ssid SSID          WiFi SSID (default: SomeSSID)" 
    echo "  -p, --password PASS     WiFi password (default: mypassword123)"
    echo "  -r, --priority NUM      WiFi priority (default: 0)"
    echo "  -h, --help              Show this help message"
}

# Function to cleanup data directory
cleanup() {
    echo "Cleaning up data directory..."
    if [ -d "$DATA_DIR" ]; then
        rm -rf "$DATA_DIR"
        echo "Data directory cleaned up"
    fi
}

# Trap cleanup function for script interruption or exit
trap cleanup EXIT

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        -n|--num-leds)
            NUM_LEDS="$2"
            shift 2
            ;;
        -u|--universe)
            UNIVERSE="$2"
            shift 2
            ;;
        -a|--address)
            START_ADDRESS="$2"
            shift 2
            ;;
        -d|--device-name)
            DEVICE_NAME="$2"
            shift 2
            ;;
        -s|--ssid)
            WIFI_SSID="$2"
            shift 2
            ;;
        -p|--password)
            WIFI_PASSWORD="$2"
            shift 2
            ;;
        -r|--priority)
            WIFI_PRIORITY="$2"
            shift 2
            ;;
        -h|--help)
            print_usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            print_usage
            exit 1
            ;;
    esac
done

# Create data directory if it doesn't exist
DATA_DIR="data"
mkdir -p "$DATA_DIR"

# Generate config.json
echo "Generating config.json..."
cat > "$DATA_DIR/config.json" << EOF
{
    "numLeds": $NUM_LEDS,
    "universe": $UNIVERSE,
    "startAddress": $START_ADDRESS,
    "deviceName": "$DEVICE_NAME"
}
EOF

# Escape \ in SSID
WIFI_SSID=$(echo "$WIFI_SSID" | sed 's/\\/\\\\/g')

# Generate wifi_config.json
echo "Generating wifi_config.json..."
cat > "$DATA_DIR/wifi_config.json" << EOF
{
    "wifiConfigs": [
        {
            "ssid": "$WIFI_SSID",
            "password": "$WIFI_PASSWORD",
            "priority": $WIFI_PRIORITY
        }
    ]
}
EOF

# Verify files were created
if [[ ! -f "$DATA_DIR/config.json" ]] || [[ ! -f "$DATA_DIR/wifi_config.json" ]]; then
    echo "Error: Failed to create configuration files"
    exit 1
fi

# Upload to SPIFFS
echo "Uploading files to SPIFFS..."
if command -v pio &> /dev/null; then
    pio run --target uploadfs
    if [ $? -eq 0 ]; then
        echo "Successfully uploaded files to SPIFFS"
    else
        echo "Error: Failed to upload files to SPIFFS"
        exit 1
    fi
else
    echo "Error: PlatformIO (pio) not found. Please install PlatformIO first."
    exit 1
fi
