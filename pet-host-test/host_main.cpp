#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <fstream>
#include <cmath>
#include <sstream>
#include <syslog.h>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "mqtt_cnn.h"

const std::vector<std::string> PET_CLASSES = { 
    "Abyssinian", "American_Bulldog", "American_Pit_Bull_Terrier", "Basset_Hound", 
    "Beagle", "Bengal", "Birman", "Bombay", "Boxer", "British_Shorthair", 
    "Chihuahua", "Egyptian_Mau", "English_Cocker_Spaniel", "English_Setter", 
    "German_Shorthaired", "Great_Pyrenees", "Havanese", "Japanese_Chin", 
    "Keeshond", "Leonberger", "Maine_Coon", "Miniature_Pinscher", "Newfoundland", 
    "Pomeranian", "Pug", "Ragdoll", "Russian_Blue", "Samyed", "Scottish_Terrier", 
    "Shiba_Inu", "Sphynx", "Staffordshire_Bull_Terrier", "Wheaten_Terrier", 
    "Yorkshire_Terrier", "Persian", "Saint_Bernard", "Siamese" 
};

struct Prediction {
    int class_index;
    float probability;
};

// Helper: Save RGB buffer to a standard PPM image file
void save_ppm(const std::string& filename, uint8_t* data, int width, int height) {
    std::ofstream out(filename, std::ios::binary);
    out << "P6\n" << width << " " << height << "\n255\n";
    out.write(reinterpret_cast<char*>(data), width * height * 3);
    out.close();
    syslog(LOG_INFO, "Saved captured frame to %s", filename.c_str());
}

// Build JSON payload manually
std::string build_json_payload(double infer_ms, const std::vector<Prediction>& top_preds) {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"inference_time_ms\": " << std::fixed << std::setprecision(2) << infer_ms << ",\n";
    ss << "  \"inference_time_us\": " << (int)(infer_ms * 1000) << ",\n";
    ss << "  \"top_1_class\": \"" << PET_CLASSES[top_preds[0].class_index] << "\",\n";
    ss << "  \"top_1_confidence\": " << std::fixed << std::setprecision(2) << top_preds[0].probability << ",\n";
    ss << "  \"top_5\": [\n";
    for(int i = 0; i < 5 && i < top_preds.size(); ++i) {
        ss << "    {\"" << PET_CLASSES[top_preds[i].class_index] << "\": " 
           << std::fixed << std::setprecision(2) << top_preds[i].probability << "}";
        if (i < 4) ss << ",";
        ss << "\n";
    }
    ss << "  ]\n}";
    return ss.str();
}

void print_ascii_dashboard(int frame_count, double wall_ms, double infer_ms, const std::vector<Prediction>& top_preds) {
    std::cout << "\033[2J\033[H"; // Clear screen
    std::cout << "  [Frame " << frame_count << "]  wall=" << std::fixed << std::setprecision(1) << wall_ms << "ms  infer=" << infer_ms << "ms\n";
    std::cout << "┌─────────────────────────────────────────────┐\n";
    std::cout << "│  TOP-1 : " << std::left << std::setw(26) << PET_CLASSES[top_preds[0].class_index] 
              << std::right << std::setw(5) << std::fixed << std::setprecision(1) << top_preds[0].probability << "%  │\n";
    std::cout << "│  Time  : " << std::left << std::setw(6) << (int)(infer_ms * 1000) << " µs  (" 
              << std::fixed << std::setprecision(1) << infer_ms << " ms)             │\n";
    std::cout << "├─────────────────────────────────────────────┤\n";
    std::cout << "│  TOP-5:                                     │\n";
    for(int i = 0; i < 5 && i < top_preds.size(); ++i) {
        std::cout << "│  " << i+1 << ". " << std::left << std::setw(28) << PET_CLASSES[top_preds[i].class_index] 
                  << std::right << std::setw(6) << std::fixed << std::setprecision(1) << top_preds[i].probability << "% │\n";
    }
    std::cout << "└─────────────────────────────────────────────┘\n" << std::endl;
}

int main(int argc, char *argv[]) {
    openlog("PetClassifier", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Starting Pet Classifier Host App");
    gst_init(&argc, &argv);

    // Initialize MQTT (Using HiveMQ public broker on standard TCP port 1883)
    MqttPublisher mqtt("broker.hivemq.com", 1883, "raspi4/pet_classifier/results");
    mqtt.connect();

    // 1. Load Model
    std::unique_ptr<tflite::FlatBufferModel> model = tflite::FlatBufferModel::BuildFromFile("mobilenet_v2_pet_qat.tflite");
    if (!model) {
        syslog(LOG_ERR, "Failed to load tflite model");
        return -1;
    }

    tflite::ops::builtin::BuiltinOpResolver resolver;
    std::unique_ptr<tflite::Interpreter> interpreter;
    tflite::InterpreterBuilder(*model, resolver)(&interpreter);
    interpreter->AllocateTensors();
    
    TfLiteTensor* input_tensor = interpreter->tensor(interpreter->inputs()[0]);
    float input_scale = input_tensor->params.scale;
    int input_zp = input_tensor->params.zero_point;
    
    TfLiteTensor* output_tensor = interpreter->tensor(interpreter->outputs()[0]);
    float output_scale = output_tensor->params.scale;
    int output_zp = output_tensor->params.zero_point;

    // 2. Setup GStreamer
    std::string pipeline_str = "v4l2src device=/dev/video0 ! video/x-raw,width=640,height=480 ! videoconvert ! videoscale ! video/x-raw,width=224,height=224,format=RGB ! appsink name=mysink drop=true max-buffers=1";
    GError *error = nullptr;
    GstElement *pipeline = gst_parse_launch(pipeline_str.c_str(), &error);
    
    if (error) {
        syslog(LOG_ERR, "Pipeline error: %s", error->message);
        return -1;
    }

    GstElement *appsink = gst_bin_get_by_name(GST_BIN(pipeline), "mysink");
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    int frame_count = 0;
    auto wall_start = std::chrono::high_resolution_clock::now();

    // 3. Inference Loop
    while (true) {
        GstSample *sample = gst_app_sink_pull_sample(GST_APP_SINK(appsink));
        if (!sample) continue;

        GstBuffer *buffer = gst_sample_get_buffer(sample);
        GstMapInfo map;
        gst_buffer_map(buffer, &map, GST_MAP_READ);

        // MobileNetV2 preprocessing: [0, 255] -> [-1.0, 1.0] -> INT8
        for (int i = 0; i < 224 * 224 * 3; i++) {
            float normalized_pixel = (static_cast<float>(map.data[i]) / 127.5f) - 1.0f;
            int8_t quantized_val = static_cast<int8_t>(std::round(normalized_pixel / input_scale) + input_zp);
            quantized_val = std::max((int8_t)-128, std::min((int8_t)127, quantized_val));
            input_tensor->data.int8[i] = quantized_val;
        }

        auto infer_start = std::chrono::high_resolution_clock::now();
        interpreter->Invoke();
        auto infer_end = std::chrono::high_resolution_clock::now();

        std::vector<Prediction> predictions;
        for (int i = 0; i < 37; i++) {
            float real_prob = (output_tensor->data.int8[i] - output_zp) * output_scale;
            predictions.push_back({i, real_prob * 100.0f});
        }

        std::sort(predictions.begin(), predictions.end(), 
            [](const Prediction& a, const Prediction& b) { return a.probability > b.probability; });

        auto wall_end = std::chrono::high_resolution_clock::now();
        double infer_ms = std::chrono::duration<double, std::milli>(infer_end - infer_start).count();
        double wall_ms = std::chrono::duration<double, std::milli>(wall_end - wall_start).count();
        wall_start = wall_end;

        print_ascii_dashboard(frame_count++, wall_ms, infer_ms, predictions);

        // Success Trigger
        if (predictions[0].probability > 80.0f && frame_count > 15) {
            syslog(LOG_INFO, "High confidence detected: %s (%.1f%%). Triggering MQTT.", 
                   PET_CLASSES[predictions[0].class_index].c_str(), predictions[0].probability);
            
            save_ppm("success_capture.ppm", map.data, 224, 224);
            
            // Construct and send JSON payload
            std::string payload = build_json_payload(infer_ms, predictions);
            mqtt.publish(payload);

            gst_buffer_unmap(buffer, &map);
            gst_sample_unref(sample);
            break; // Exit loop after successful publish
        }

        gst_buffer_unmap(buffer, &map);
        gst_sample_unref(sample);
    }

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    closelog();
    return 0;
}
