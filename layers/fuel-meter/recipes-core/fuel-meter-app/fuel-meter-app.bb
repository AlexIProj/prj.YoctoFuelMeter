SUMMARY = "Fuel Meter App"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "\
    file://main.c \
    file://config.h \
    file://calcs.c file://calcs.h \
    file://serial_com.c file://serial_com.h \
    file://sensors.c file://sensors.h \
    file://gpio_driver.c file://gpio_driver.h \
    file://fuel-meter.service"

S = "${WORKDIR}"

DEPENDS = "libgpiod"
RDEPENDS:${PN} = "libgpiod"

inherit pkgconfig systemd
SYSTEMD_SERVICE:${PN} = "fuel-meter.service"
SYSTEMD_AUTO_ENABLE = "disable"

do_compile() {
    ${CC} ${CFLAGS}  $(pkg-config --cflags libgpiod) \
    -o fuel-meter-app *.c  \
    ${LDFLAGS} $(pkg-config --libs libgpiod) -lpthread
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 fuel-meter-app ${D}${bindir}

    install -d ${D}${systemd_unitdir}/system
    install -m 0644 fuel-meter.service ${D}${systemd_unitdir}/system
}
