import machine
from bmp280 import *

bus = machine.I2C(scl=machine.Pin(22), sda=machine.Pin(21))
bmp = BMP280(bus)

bmp.use_case(BMP280_CASE_WEATHER)
bmp.oversample(BMP280_OS_HIGH)

bmp.temp_os = BMP280_TEMP_OS_8
bmp.press_os = BMP280_PRES_OS_4

bmp.standby = BMP280_STANDBY_250
bmp.iir = BMP280_IIR_FILTER_2

bmp.spi3w = BMP280_SPI3W_ON

bmp.force_measure()

bmp.normal_measure()

bmp.sleep()

print(bmp.temperature)
print(bmp.pressure)
