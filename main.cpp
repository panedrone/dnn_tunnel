#include "pch.h"

#include <iostream>
#include <thread>

#include <opencv2/opencv.hpp>

#include "dnn_tunnel_cv.h"

static void VSTCALLBACK on_item(int bytes_count, unsigned char *bytes)
{
    std::vector<unsigned char> img;
    img.insert(img.end(), &bytes[0], &bytes[bytes_count]);
    cv::Mat mat = cv::imdecode(img, cv::ImreadModes::IMREAD_COLOR);
    cv::imshow("Item", mat);
}

static bool prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

static void VSTCALLBACK on_message(const char *msg)
{
    if (prefix("[ERROR]", msg))
    {
        std::cerr << msg << std::endl;
    }
    else
    {
        std::cout << msg << std::endl;
    }
}

struct AOI
{
    int X = 0;
    int Y = 0;
    int Width = 0;
    int Height = 0;
};

static void init_tracking(std::string yolo_class_names, std::string yolo_cfg, std::string yolo_weights,
                          int inp_width_height, AOI aoi, bool flip_x)
{
    _set_on_message_callback(on_message);

    _set_on_item_callback(on_item);

    /////////////////////////////////////

    _setup_yolo(yolo_class_names.c_str(), yolo_cfg.c_str(), yolo_weights.c_str());

    _setup_inp_width_height(inp_width_height);

    _set_aoi(aoi.X, aoi.Y, aoi.Width, aoi.Height);

    _set_flip_x(flip_x);
}

static void post_message(std::string msg)
{
    on_message(msg.c_str());
}

void task1(std::string msg)
{
    // _process_video_flow(vieo_file_path, debug, conf_0_100, nms_0_100, grayscale
    //                     : false);

    post_message("Ended");
}

static void run_video_tracking(std::string vieo_file_path, bool debug, int conf_0_100, int nms_0_100)
{
    if (_is_running())
    {
        post_message("[ERROR] Tracking is running");

        return;
    }

    // https://stackoverflow.com/questions/22332181/passing-lambdas-to-stdthread-and-calling-class-methods

    std::thread t1([&]() {
        _process_video_flow(vieo_file_path.c_str(), debug, conf_0_100, nms_0_100, false);
        post_message("Ended");
    });

    // https://stackoverflow.com/questions/266168/simple-example-of-threading-in-c
    // If later on you want to wait for the thread to be done executing the function, call:

    t1.join();
}

std::string _erase_path(std::string s)
{
    s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
    return s;
}

int main(int, char *args[])
{
//     std::cout << "Hello, OpenCV 4.4!\n";
//     std::cerr << "Test cerr" << std::endl;
//     // https://stackoverflow.com/questions/1528298/get-path-of-executable

//     std::string path;
//     std::string aux(args[0]);
// #if defined(_WIN32) || defined(WIN32)
//     int pos = aux.rfind('\\');
//     path = aux.substr(0, pos + 1);
// #else
//     int pos = aux.rfind('/');
//     path = aux.substr(0, pos + 1);
// #endif
//     // path = aux.substr(0, pos + 1);

//     std::string yolo_profile_path = path + "yolov3_tiny_160_10_gray.txt";
//     std::vector<std::string> lines;
//     {
//         // std::wifstream wifs;
//         std::ifstream ifs; 
//         ifs.open(yolo_profile_path);
//         if (!ifs.is_open())
//         {
//             CV_Error(cv::Error::Code::StsError, std::string("File ") + yolo_profile_path + " not found");
//         }
//         // === panedrone: std::ifstream does not work correctly with unicode files (starting with something like EF BB BF)
//         std::string line;
//         while (std::getline(ifs, line))
//         {
//             lines.push_back(_erase_path(line));
//         }
//         ifs.close();
//     } // ensure destruct

//     if (lines.size() < 3)
//     {
//         CV_Error(cv::Error::Code::StsError, std::string("Invalid YOLO profile"));
//     }

//     std::string yolo_class_names = path + lines[0]; // "obj_10.names";
//     std::string yolo_cfg = path + lines[1];         // "yolov3-tiny_obj_160_10_gray.cfg";
//     std::string yolo_weights = path + lines[2];     // "yolov3-tiny_obj_160_10_gray_final.weights";

//     std::ifstream ifs2;
//     const char *path2 = yolo_class_names.c_str();
//     auto can = cv::utils::fs::canonical(yolo_class_names);
//     if (!cv::utils::fs::exists(can))
//     {
//         std::cerr << yolo_class_names << std::endl;
//         return -1;
//     }
//     ifs2.open(path2); //, std::ifstream::in); // // === panedrone: #include <fstream>

//     if (!ifs2.is_open())
//     {
//         CV_Error(cv::Error::Code::StsError, yolo_class_names + "\nfile not found");
//     }
//     // std::string yolo_class_names = _erase_path(lines[0]); // "obj_10.names";
//     // std::string yolo_cfg = _erase_path(lines[1]); // "yolov3-tiny_obj_160_10_gray.cfg";
//     // std::string yolo_weights = _erase_path(lines[2]); // "yolov3-tiny_obj_160_10_gray_final.weights";

//     init_tracking(yolo_class_names, yolo_cfg, yolo_weights, /*inp_width_height*/ 160, AOI(), /*flip_x*/ false);

//     std::string vieo_file_path = path + "WIN_20200424_11_08_18_Pro.mp4";
//     bool debug = true;
//     int conf_0_100 = 50;
//     int nms_0_100 = 0;

//     run_video_tracking(vieo_file_path, debug, conf_0_100, nms_0_100);

    // cv::waitKey();

    return 0;
}
