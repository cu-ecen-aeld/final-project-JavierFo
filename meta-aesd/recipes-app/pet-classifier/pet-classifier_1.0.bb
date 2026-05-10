SUMMARY = "Pet Classifier App with MQTT"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://main.cpp \
           file://mqtt_cnn.cpp \
           file://mqtt_cnn.h \
           file://CMakeLists.txt \
           file://mobilenet_v2_pet_qat.tflite"
S = "${WORKDIR}"

# ADDED mosquitto to DEPENDS
DEPENDS = "gstreamer1.0 gstreamer1.0-plugins-base libtensorflow-lite mosquitto"

inherit cmake pkgconfig

do_install() {
    install -d ${D}${bindir}
    install -m 0755 pet-classifier ${D}${bindir}/

    install -d ${D}/usr/share/models
    install -m 0644 ${WORKDIR}/mobilenet_v2_pet_qat.tflite ${D}/usr/share/models/
}

FILES:${PN} += "/usr/share/models/mobilenet_v2_pet_qat.tflite"
