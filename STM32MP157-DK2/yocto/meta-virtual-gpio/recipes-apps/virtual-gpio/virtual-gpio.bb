SUMMARY = "STM32MP157 Virtual GPIO Application"
DESCRIPTION = "GPIO control application supporting sysfs and libgpiod"
LICENSE = "MIT"

SRC_URI = " \
    file://gpio-common.c \
    file://gpio-sysfs.c \
    file://gpio-libgpiod.c \
    file://gpio_common.h \
    file://gpio_sysfs.h \
    file://gpio_libgpiod.h \
    file://gpio_toggle.c \
    file://led_blink.c \
    file://button.c \
    file://button_irq.c \
    file://pwm_fade.c \
    file://pwm_led.c \
    file://gpio-test-config.json \
"

S = "${WORKDIR}"

DEPENDS += "libgpiod"

do_compile() {
    ${CC} ${CFLAGS} \
        -I${S} \
        gpio-common.c \
        gpio-sysfs.c \
        gpio-libgpiod.c \
        gpio_toggle.c \
        -lgpiod \
        ${LDFLAGS} \
        -o gpio-toggle

    ${CC} ${CFLAGS} \
        -I${S} \
        gpio-common.c \
        gpio-sysfs.c \
        gpio-libgpiod.c \
        led_blink.c \
        -lgpiod \
        ${LDFLAGS} \
        -o led-blink

    ${CC} ${CFLAGS} \
        -I${S} \
        gpio-common.c \
        gpio-sysfs.c \
        gpio-libgpiod.c \
        button.c \
        -lgpiod \
        ${LDFLAGS} \
        -o gpio-button

    ${CC} ${CFLAGS} \
        -I${S} \
        gpio-common.c \
        gpio-sysfs.c \
        gpio-libgpiod.c \
        button_irq.c \
        -lgpiod \
        ${LDFLAGS} \
        -o button-irq

    ${CC} ${CFLAGS} \
        -I${S} \
        gpio-common.c \
        gpio-sysfs.c \
        gpio-libgpiod.c \
        pwm_fade.c \
        -lgpiod \
        ${LDFLAGS} \
        -o pwm-fade

    ${CC} ${CFLAGS} \
        -I${S} \
        gpio-common.c \
        gpio-sysfs.c \
        gpio-libgpiod.c \
        pwm_led.c \
        -lgpiod \
        ${LDFLAGS} \
        -o pwm-led
}

do_install() {
    install -d ${D}${bindir}
    install -d ${D}${sysconfdir}/virtual-gpio

    install -m 0755 gpio-toggle ${D}${bindir}/gpio-toggle
    install -m 0755 led-blink ${D}${bindir}/led-blink
    install -m 0755 gpio-button ${D}${bindir}/gpio-button
    install -m 0755 button-irq ${D}${bindir}/button-irq
    install -m 0755 pwm-fade ${D}${bindir}/pwm-fade
    install -m 0755 pwm-led ${D}${bindir}/pwm-led

    install -m 0644 gpio-test-config.json \
        ${D}${sysconfdir}/virtual-gpio/gpio-test-config.json
}

RDEPENDS:${PN} += "libgpiod"
