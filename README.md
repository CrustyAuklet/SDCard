#### Init the card
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
