#pragma once

#include "CV_Rect.h"

enum Cap_Backend_API
{
	ANY = 0xff,
	DSHOW = 0,		// DirectShow (via videoInput)
	MSMF = 1		// Microsoft Media Foundation (via videoInput)
};

// https://stackoverflow.com/questions/10789012/g-cdecl-calling-convention-with-steinberg-vst-sdk
// cdecl is the default calling method in G++.

// #if defined(__GNUC__)
//     #define VSTCALLBACK __cdecl

// x64 not add _ to name
// https://stackoverflow.com/questions/58490678/how-many-underscores-in-the-exported-function-in-c

#if defined(__GNUC__)
    #define VSTCALLBACK
#elif defined(WIN32) || defined(_WIN32) || defined(__FLAT__) || defined CBUILDER
    #define VSTCALLBACK __cdecl
	// #define VSTCALLBACK
#else
    #define VSTCALLBACK
#endif

typedef void(VSTCALLBACK * on_item_callback)(int, unsigned char *);
typedef void(VSTCALLBACK * on_message_callback)(const char *);

/**

=== panedrone:

IMPLEMENTATION OF THIS CLASS IS BASED ON
https://github.com/opencv/opencv/blob/master/samples/dnn/object_detection.cpp

*/
class CV_Item_Tracker
{
private:

	CV_Item_Tracker();

	std::string vieo_file_path_name;

	void open_camera(int cam_index);

	bool _is_running = false;

	void _preprocess_frame(cv::Mat &frame, bool flip_x, bool grayscale, bool half_size_frame);

	void _pre_process(const cv::Mat& frame, cv::Mat &blob, cv::dnn::Net& net, cv::Size inpSize, float scale,
		const cv::Scalar& mean, bool swapRB);

	double _post_process(cv::Mat &mat_painted, const cv::Mat &mat_original, const std::vector<cv::Mat>& outs, cv::dnn::Net& net, cv::Point& gc);

	void _draw_pred(int classId, float conf, int left, int top, int right, int bottom, cv::Mat& frame);

	on_item_callback _on_item_cb;

	on_message_callback _on_message_cb;

	std::vector<std::string> classes;

	cv::Rect NO_ROI;

	cv::Rect roi; // zero width == no crop

	int resolution_width;
	int resolution_height;

	cv::Mat _mat_mask;

	void run_dnn_object_detection(int cam_index);

public:

	static CV_Item_Tracker inst;

#define tracker CV_Item_Tracker::inst

	static void post_message(const char *msg)
	{
		if (inst._on_message_cb)
		{
			inst._on_message_cb(msg);
		}
	}

	static void post_image(int bytes_count, unsigned char *bytes)
	{
		if (inst._on_item_cb)
		{
			inst._on_item_cb(bytes_count, bytes);
		}
	}

	int inp_width = 160;
	int inp_height = 160;

	void bind(on_message_callback on_message_cb)
	{
		_on_message_cb = on_message_cb;
	}

	void bind(on_item_callback on_item_cb)
	{
		_on_item_cb = on_item_cb;
	}

	float conf = 0.5f;	// "{ thr         | .5 | Confidence threshold. }"
	float nms = 0.0f;	// "{ nms         | .4 | Non-maximum suppression threshold. }"

	int min_bounding_box;
	double rect_distance_ratio;

	// ask them on timer
	long max_track_age = std::numeric_limits<long>::min();
	double avg_track_age;
	double avg_track_length;
	double fps;

	bool flip_x = false;
	bool grayscale_yolo = false;
	bool half_size_frame = true;
	bool free_run_grayscale = false;

	bool is_debug = false;
	int frame_delay = 10;

	Cap_Backend_API cap_backend_api = Cap_Backend_API::MSMF;

	void set_mask(const char *mask);

	std::string yolo_names;
	std::string yolo_cfg;
	std::string yolo_weights;

	bool is_debug_build()
	{
#if !defined(NDEBUG)
		return true;
#else
		return false;
#endif
	}

	bool resolution_default = true;

	void set_resolution(int width, int height);

	void set_aoi(const CV_Rect &aoi); // Rectangle.Empty for no AOI

	CV_Rect get_aoi(); // it is called to qury after selectROI

	void run_tracking(int cam_index)
	{
		vieo_file_path_name.clear();

		run_dnn_object_detection(cam_index);
	}

	void run_tracking(const char *video_fn)
	{
		vieo_file_path_name = video_fn;

		run_dnn_object_detection(-1);
	}

	void run_free(int cam_index);

	bool is_running()
	{
		return _is_running;
	}

	void stop()
	{
		_is_running = false;
	}
};