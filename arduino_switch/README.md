# Arduino switch
Simple arduino switch with nRF24 radio.

## Dependencies
- Makefile chain [Arduino-Makefile](https://github.com/sudar/Arduino-Makefile)
- RF24 and OneWire custom libraries. They are located in *lib* subfloder

## Board selection
See boards available with `make show_boards`
Update BOARD_TAG in Makefile file.

## Use
- Update ARDUINO_SKETCHBOOK in Makefile file with Arduino-Makefile download path.
- Update ARDUINO_LIBS in Makefile file with libraries path
- Build and upload with `make` and `make upload`

## Note
Tested with arduino 1.6.0
