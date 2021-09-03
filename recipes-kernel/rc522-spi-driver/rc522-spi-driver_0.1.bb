SUMMARY = "SPI Driver for RC522 RFID Module"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${S}/COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = "git://github.com/ugurturkel/rc522-spi-driver.git;branch=main"
SRCREV = "93f4a42ade94be2dae394b79ef348e8b8737f64c"
S = "${WORKDIR}/git"

# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.

RPROVIDES_${PN} += "kernel-module-rc522-spi-driver"
