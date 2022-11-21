// CV_Item_Tracker.h defines callbacks:

#include "CV_Item_Tracker.h"

// https://stackoverflow.com/questions/10789012/g-cdecl-calling-convention-with-steinberg-vst-sdk
// cdecl is the default calling method in G++.

#if defined(__GNUC__)
#define DNNTUNNELCV_API	VSTCALLBACK
#elif defined(WIN32) || defined(_WIN32) || defined(__FLAT__) || defined CBUILDER
#define DNNTUNNELCV_API __declspec(dllexport) VSTCALLBACK
#else
#error TODO
#endif

// === panedrone: witout extern "C" {
//
// Function name is like 'bool __cdecl is_running(void)'
//
extern "C" {

	bool DNNTUNNELCV_API _is_running();

	void DNNTUNNELCV_API _stop();

	void DNNTUNNELCV_API _process_cam_flow(int cam_index, bool res_default, int res_width, int res_height, int api, bool debug, int conf_0_100, int nms_0_100, bool grayscale);

	void DNNTUNNELCV_API _process_free_run(int cam_index, bool res_default, int res_width, int res_height, int api, bool grayscale);

	void DNNTUNNELCV_API _process_video_flow(const char *vieo_file_path, bool debug, int conf_0_100, int nms_0_100, bool grayscale);

	long DNNTUNNELCV_API _get_max_track_age();

	double DNNTUNNELCV_API _get_avg_track_length();

	double DNNTUNNELCV_API _get_avg_track_age();

	double DNNTUNNELCV_API _get_fps();

	// mandatory settings before start

	void DNNTUNNELCV_API _setup_yolo(const char * class_names, const char * yolo_cfg, const char * yolo_weights, bool rgb);

	void DNNTUNNELCV_API _setup_resolution(bool use_default_resolution, int width, int height);

	void DNNTUNNELCV_API _setup_inp_width_height(int inp_width_height);

	// settings

	void DNNTUNNELCV_API _set_dnn_thresholds(int conf_0_100, int nms_0_100);

	void DNNTUNNELCV_API _set_aoi(int x, int y, int w, int h);

	void DNNTUNNELCV_API _set_flip_x(bool value);

	void DNNTUNNELCV_API _set_mask(const char *path);

	void DNNTUNNELCV_API _set_min_bounding_box(int value);

	void DNNTUNNELCV_API _set_rect_distance_ratio(double value);

	void DNNTUNNELCV_API _set_frame_delay(int value);

	void DNNTUNNELCV_API _set_half_size_frame(bool value);

	// callbacks

	void DNNTUNNELCV_API _set_on_item_callback(on_item_callback callback);

	void DNNTUNNELCV_API _set_on_message_callback(on_message_callback callback);
}