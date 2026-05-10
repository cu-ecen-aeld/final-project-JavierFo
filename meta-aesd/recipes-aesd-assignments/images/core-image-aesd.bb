inherit core-image
CORE_IMAGE_EXTRA_INSTALL += "aesd-assignments"
CORE_IMAGE_EXTRA_INSTALL += "openssh"
inherit extrausers
# See https://docs.yoctoproject.org/singleindex.html#extrausers-bbclass
# We set a default password of root to match our busybox instance setup
# Don't do this in a production image
# PASSWD below is set to the output of
# printf "%q" $(mkpasswd -m sha256crypt root) to hash the "root" password
# string
PASSWD = "\$5\$2WoxjAdaC2\$l4aj6Is.EWkD72Vt.byhM5qRtF9HcCM/5YpbxpmvNB5"
EXTRA_USERS_PARAMS = "usermod -p '${PASSWD}' root;"

# Add GStreamer and plugins (Updated for V4L2)
CORE_IMAGE_EXTRA_INSTALL += "gstreamer1.0 gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-good-video4linux2"

# Add libcamera (requires meta-raspberrypi)
CORE_IMAGE_EXTRA_INSTALL += "libcamera"

# Add MQTT support (Mosquitto broker/clients and Python paho-mqtt)
CORE_IMAGE_EXTRA_INSTALL += "mosquitto mosquitto-clients python3-paho-mqtt"

# Add TensorFlow Lite C++ library
CORE_IMAGE_EXTRA_INSTALL += "libtensorflow-lite python3-tensorflow-lite"

# Add v4l-utils for camera debugging
CORE_IMAGE_EXTRA_INSTALL += "v4l-utils"

CORE_IMAGE_EXTRA_INSTALL += "pet-classifier"

CORE_IMAGE_EXTRA_INSTALL += "wpa-supplicant iw"
