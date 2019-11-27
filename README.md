# Policy Based SD card driver
## Motivation
There are a lot of SD card drivers available online but most are tighly coupled with the framework they are developed for. I wanted a SD card driver that I could use anywhere, unit test from my desktop development machine, and not sacrafice in speed or code space.

## Policy based design
Policy based design allows me to write the SD card driver without thinking too much about the underlying hardware implementation. Design descisons which are "orthogonal" can be based on policies provided at compile time. This is similar to how software should be modularized based on design decisions and not on process flow ([Parnas, 1971](https://www.win.tue.nl/~wstomv/edu/2ip30/references/criteria_for_modularization.pdf))

### design decisions:
I identified the following seperate design decisions that were then made into policy classes:
 - **CRC**: If the system is space limited then the CRC policy can calculate the CRC each time. If the system needs performance it can use a table based CRC. Some systems have hardware CRC peripherals, so that could also be used.
 - **Timing**: There are certain actions that require timout periods. Keeping track of time is delegated to a policy class. This enables very simple implementations. The default tick based policy simply tracks time by counting how many times the driver asks what time it is. This has been tested and works well on a system with no clock/counter that works well with timing.
 - **SPI communication**: This driver assumes SPI communication but the policy only needs a read/write/CS. SD cards expect weird CS behaviour so the CS pin could not also be abstracted.
 
### design tradeoff:
Since the policy classes are obtained through template parameters there is the possibiliy of code bloat *IF AND ONLY IF* you have multiple SD cards with different policies. This seems like an acceptable tradeoff since it is unlikely that a system with two SD cards would use a different policy for CRC/timing for each card. since the SPI communication is a policy that is the most likely pain point if the SD cards are on different SPI busses.

## Testing
The driver was tested with the [I2C Driver](https://spidriver.com/) and [Aardvark](https://www.totalphase.com/products/aardvark-i2cspi/) SPI devices from a desktop PC during development. It was also tested running on a [Atmel SAML21 custom board](https://www.microchip.com/wwwproducts/en/ATSAML21E18B) and [FeatherM0](https://www.adafruit.com/product/2772). It passes all tests from the [elem-chan FatFS](http://elm-chan.org/fsw/ff/00index_e.html) library on all the systems.

## Future Work
I need to clean up this repo now that I made it public. 
 - Add better testing and better default policies
 - Provide more isntructiuons on setting up the various test and dev environments
 - add a liscense and make sure it is compatable with dependencies
 - documentation!

## Notes
Most for myself, but these are some of the notes I used when developing this driver

[Good stack overflow with flow chart and explainations](https://electronics.stackexchange.com/questions/77417/what-is-the-correct-command-sequence-for-microsd-card-initialization-in-spi)

[blog post with good descriptions](https://luckyresistor.me/cat-protector/software/sdcard-2/)

[secondary blog post](http://patrickleyman.be/blog/sd-card-c-driver-init/)

SD card command may require 0xFF clocks (with no CS asserted) inbetween commands to change state

also make sure to actually change CS line. Most cards should be ok with a grounded CS line, but some are not.


[ARM Mbed implementation](https://github.com/ARMmbed/sd-driver/blob/master/SDBlockDevice.cpp)

[MODM block device interface](https://github.com/modm-io/modm/blob/develop/src/modm/driver/storage/block_device_spiflash.hpp)

[SD card intro by FatFS author](http://elm-chan.org/docs/mmc/i/sdinit.png)

https://www.mouser.com/datasheet/2/638/96FMMSDI-4G-ET-AT1_datasheet20170331165103-1385548.pdf

http://www.dejazzer.com/ee379/lecture_notes/lec12_sd_card.pdf
