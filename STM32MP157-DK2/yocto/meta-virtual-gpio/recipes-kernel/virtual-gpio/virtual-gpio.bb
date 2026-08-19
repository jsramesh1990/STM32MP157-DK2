SUMMARY = "Virtual GPIO kernel driver for STM32MP157-DK2"
DESCRIPTION = "Linux kernel module providing a virtual GPIO interface for GPIO testing and simulation"
SECTION = "kernel/modules"
LICENSE = "MIT"

inherit module

SRC_URI = "file://virtual_gpio.c \
           file://virtual_gpio.h \
           file://Makefile \
          "

S = "${WORKDIR}"

# Kernel module package
RPROVIDES:${PN} += "kernel-module-virtual-gpio"

# Automatically load the module after installation
KERNEL_MODULE_AUTOLOAD += "virtual_gpio"

# Build the external kernel module
do_compile() {
    oe_runmake \
        -C ${STAGING_KERNEL_DIR} \
        M=${S} \
        ARCH=${ARCH} \
        CROSS_COMPILE="${TARGET_PREFIX}" \
        modules
}

do_install() {
    install -d ${D}${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra

    install -m 0644 \
        ${S}/virtual_gpio.ko \
        ${D}${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra/
}

FILES:${PN} += "${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra/virtual_gpio.ko"
