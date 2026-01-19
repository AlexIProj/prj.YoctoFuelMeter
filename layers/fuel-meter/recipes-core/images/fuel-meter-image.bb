SUMMARY = "Imaginea Custom pentru Fuel Meter pe RPi 5"
LICENSE = "MIT"

require recipes-core/images/core-image-minimal.bb
IMAGE_INSTALL += "libgpiod libgpiod-tools"

IMAGE_BASENAME = "fuel-meter-image"