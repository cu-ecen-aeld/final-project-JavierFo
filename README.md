# final-project-JavierFo
final-project-JavierFo created by GitHub Classroom

(Also in Wiki)

## Edge-AI Pet Classifier
This README provides a comprehensive technical overview of the Edge-AI Pet Classifier, a high-performance embedded vision system designed for the Raspberry Pi 4. It integrates deep learning inference at the edge with cloud-based reporting via MQTT.

### A. Overview:
The goal of this project is to build a robust, real-time embedded system capable of identifying 37 different breeds of cats and dogs using a camera feed. Unlike standard cloud-based vision systems, this project emphasizes Edge AI, performing all neural network calculations locally to minimize latency and privacy risks.

### B. Motivation: 
Demonstrating the full lifecycle of an embedded Linux product—from training a Quantization-Aware (QAT) model and building a custom Yocto distribution to implementing a high-concurrency C++ application that bridges hardware-accelerated video pipelines with cloud telemetry.

## 1. Hardware Block Diagram
<img width="1536" height="1024" alt="ChatGPT Image 8 may 2026, 06_27_16 p m" src="https://github.com/user-attachments/assets/5c326d4f-e43b-46f5-b9ba-28871d991a00" />

## 2. Target Build System
This project utilizes the Yocto Project (Poky distribution) to generate a minimal, production-ready Linux image. 

By using Yocto instead of a general-purpose OS like Raspberry Pi OS, we achieve:
* Reduced Footprint: Only essential libraries (GStreamer, TFLite, Mosquitto) are included.
* Reproducibility: The entire OS configuration is defined in metadata layers.
* Performance: Optimized compiler flags for the Broadcom BCM2711.

## 3. Hardware Platform
The primary target is the Raspberry Pi 4 Model B (64-bit).

Build Platform Support: We leverage the meta-raspberrypi BSP layer to support hardware-specific features:

* Video4Linux2 (V4L2): For low-latency camera access.
* Broadcom GPU: Handled via GStreamer for potential zero-copy buffer sharing.
* Documentation: Raspberry Pi 4 Hardware Documentation.
* Secondary hw: webcam Steren COM-122

## 4. Open Source Projects Used
* TensorFlow Lite: Used for the INT8 inference engine.
* GStreamer: Handles the multi-threaded video capture and pixel format conversion (RGB).
* Eclipse Mosquitto: Provides the MQTT C library for cloud communication.
* Oxford-IIIT Pet Dataset: Used for training the MobileNetV2 model.

## 5. Previously Discussed Content Integration
The project applies core Embedded Linux concepts learned throughout the curriculum:

- Linux OS & System Calls: Implementation of file I/O for logging and .ppm image saving.
- Kernel vs. User Space: Interfacing with the v4l2 kernel driver from the user-space C++ application.
- Processes & pThreads: GStreamer operates on multiple internal threads to ensure the camera capture doesn't block the inference loop.
- IPC & Communication: Utilizing MQTT (network sockets) to transmit inference metadata to remote brokers.
- Yocto: Developed a custom meta-aesd layer, including recipes for the TFLite application and model deployment.
- Software Engineering: Adopted a modular C++ design separating the MqttPublisher logic from the main inference engine for testability on host Ubuntu machines.

## 6. New Content
This project extends beyond standard Embedded Linux into Deep Learning Engineering:

- MobileNetV2 Architecture: Utilizing inverted residuals and linear bottlenecks for efficient mobile CPU execution.
- Quantization-Aware Training (QAT): Simulating 8-bit precision during training to minimize "accuracy drop" when converting from Float32 to INT8.
- JSON Serialization: Manual construction of JSON payloads for standard cloud interoperability.

## 7. Source Code Organization
The application code and Yocto recipes are organized as follows:
```
.
├── meta-aesd (Custom Yocto Layer)
│   ├── recipes-app
│   │   └── pet-classifier
│   │       ├── files
│   │       │   ├── main.cpp              # Core inference logic
│   │       │   ├── mqtt_cnn.cpp          # MQTT wrapper implementation
│   │       │   ├── mqtt_cnn.h            # MQTT definitions
│   │       │   ├── CMakeLists.txt        # Cross-compilation config
│   │       │   └── mobilenet_v2_...tflite # The INT8 trained model
│   │       └── pet-classifier_1.0.bb     # Bitbake recipe
│   └── recipes-core
│       └── images
│           └── core-image-aesd.bb        # Custom image definition
└── README.md
```

### Submodule Links: 
- Poky: https://git.yoctoproject.org/poky
- Meta-OpenEmbedded: http://git.openembedded.org/meta-openembedded
- TensorFlow Lite: https://github.com/tensorflow/tensorflow (v2.12.0)
- Mosquitto: https://github.com/eclipse/mosquitto

### Quick Start (Deployment)
- WiFi Setup: Inject wpa_supplicant.conf into /etc/ on the rootfs.

Run Application:
- pet-classifier
- View Cloud Data: Visit the HiveMQ Web Console: https://www.hivemq.com/demos/websocket-client/ and subscribe to raspi4/pet_classifier/results.

## 8. Shared Material
N/A

## 9. Members: 
Javier Fo - Main developer

## Schedule Page:
[JavierFo's final-project-cu-ecen-aeld](https://github.com/users/JavierFo/projects/1/views/1?groupedBy%5BcolumnId%5D=345535217&visibleFields=%5B%22Title%22%2C%22Assignees%22%2C%22Status%22%2C345535217%5D
)

