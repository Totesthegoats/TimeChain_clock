# Timechain Clock

A collection of modular Bitcoin blockchain data displays that show real-time information from the Bitcoin network. Think of it as a physical dashboard for the Bitcoin blockchain.

## Overview

The Timechain Clock is a hardware project that brings Bitcoin blockchain data into the physical world through dedicated displays. Each module shows a different aspect of Bitcoin's operation, creating an ambient information display for Bitcoin enthusiasts.

### Modules

1. **Block Height Display** (Complete)
   - E-ink display showing current Bitcoin block height
   - Real-time fee rate information
   - Updates when new blocks are mined
   - CUrrently Fee estimation is included here

2. **Difficulty Epoch Progress** (🚧 In Development)
   - LED ring visualization of progress through the current difficulty adjustment period
   - Shows how close Bitcoin is to the next difficulty adjustment

3. **Halving Countdown** (Complete)
   - Countdown display to the next Bitcoin halving event
   - Shows blocks remaining until the next halving

4. **Mempool Fee Visualization** (🚧 In Development)
   - LED bar display showing current mempool fee levels
   - Visual representation of network congestion

## Hardware

### Components

**Core Electronics:**
- ESP32 DevKit boards (microcontroller for each module)
- Waveshare Pico-ePaper-4.2 displays (for block height)
- LED rings (for difficulty epoch progress)
- USB power supplies

**Sourcing:**
- Amazon.ie

### Block Height Display Setup

**Required Libraries:**
- GxEPD2 (for e-ink display control)
- WiFi (ESP32 built-in)
- HTTPClient (ESP32 built-in)
- ArduinoJson (for API parsing)

**Wiring:**
The Waveshare Pico-ePaper-4.2 connects to ESP32 as follows:
- BUSY → GPIO 4
- RST → GPIO 16
- DC → GPIO 17
- CS → GPIO 5
- CLK → GPIO 18
- DIN → GPIO 23
- GND → GND
- VCC → 3.3V

**Pin Configuration Notes:**
- ESP32 pin labels like "D23" correspond to GPIO 23 in code ect
- The GxEPD2_420_GDEY042T81 driver works best for this display

## Software

### ESP32 Firmware

The firmware polls the mempool.space API every 30 seconds to check for new blocks. When a change is detected, it performs a full screen refresh showing:
- Bitcoin symbol
- Current block height (large text using FreeMonoBold24pt7b font)
- Fee rates (high priority, medium priority, low priority, no priority)

**Key Implementation Details:**
- Uses full screen refreshes (not partial updates) for reliability
- Avoids hibernate() commands to prevent wake-up issues
- Only updates display when actual data changes
- Calculates derived data (halving countdown, epoch progress, supply statistics) from API responses

### Mobile App

**Tech Stack:**
- React Native
- Expo framework
- File-based routing

**Status:** In development
- Companion app for configuration and monitoring
- Will support multiple display modules
- Allows WiFi configuration and customization

**Development:**
```bash
# Start development server
npx expo start

# Clear cache if needed
npx expo start -c
```

## API Integration

The project uses the mempool.space API for Bitcoin blockchain data:

**Block Height Endpoint:**
```
GET https://mempool.space/api/blocks/tip/height
```

**Fee Recommendations:**
```
GET https://mempool.space/api/v1/fees/recommended
```

**Implementation Notes:**
- Polls every 30 seconds
- Only updates display on actual changes
- Handles API failures gracefully
- No authentication required

## Development Setup

### Arduino IDE Setup

1. Install ESP32 board support
   - Add to Board Manager URLs: `https://dl.espressif.com/dl/package_esp32_index.json`
   - Install "esp32 by Espressif Systems"

2. Install required libraries via Library Manager:
   - GxEPD2
   - ArduinoJson

3. Select board: "ESP32 Dev Module"

4. Configure upload settings:
   - Upload Speed: 115200
   - Flash Frequency: 80MHz

### Testing

Use Serial Monitor (115200 baud) to view debug output:
- Connection status
- API responses
- Display update events
- Error messages

## Project Philosophy

**Simplicity First:** Inspired by the BlockClock, this project aims to be straightforward to engineer while delivering a beautiful, functional product.

**Modular Design:** Each display module works independently and can be used alone or as part of a complete system.

**Reliable Updates:** Full screen refreshes over partial updates, polling-based over push notifications - choosing reliability over complexity.

**Physical Bitcoin:** Bringing blockchain data into the physical world creates a different kind of connection to Bitcoin than purely digital interfaces.

## Future Roadmap

**Hardware:**
- Complete remaining three display modules
- Design enclosures and mounting systems
- Explore manufacturing and scaling from prototype to production
- Investigate component cost reductions at volume

**Software:**
- Complete mobile app with full configuration capabilities
- Add support for multiple language displays
- Implement user preferences (units, update frequency)
- Transition from Expo Go to standalone app store deployment

**Distribution:**
- Research manufacturing approaches
- Explore small-batch vs. larger production runs
- Consider DIY kit options
- Investigate distribution channels

## Troubleshooting

**Display Issues:**
- If display shows inverted colors, ensure you're using `display.setTextColor(GxEPD_BLACK)`
- If display doesn't refresh, verify wiring and try full screen refresh instead of partial
- Avoid using `hibernate()` if you need regular updates

**WiFi Connection:**
- Ensure credentials are correct in code
- Check Serial Monitor for connection status
- ESP32 only supports 2.4GHz networks

**API Issues:**
- Test endpoints with Postman first
- Check Serial Monitor for HTTP response codes
- Verify JSON parsing with actual API responses

## Contributing

This is currently a personal project, but feel free to fork and create your own version. If you build one, I'd love to see it!


## Contact

Toto - Project Developer

If you've found value from this you can send me a couple of sats via totes@coinos.io

---

**Note:** This project is in active development. The block height display is working, with additional modules coming soon.
