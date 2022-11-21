#include "pch.h"
#include "dnn_tunnel_cv.h"

bool _is_running()
{
	return tracker.is_running();
}

void _stop()
{
	tracker.stop();
}

void _process_cam_flow(int cam_index, bool cam_resolution_use_default, int cam_resolution_width, int cam_resolution_height, int api, 
	bool debug, int conf_0_100, int nms_0_100, bool grayscale)
{
	tracker.is_debug = debug;

	tracker.conf = 0.01f * conf_0_100;
	tracker.nms = 0.01f * nms_0_100;

	tracker.resolution_default = cam_resolution_use_default;
	tracker.set_resolution(cam_resolution_width, cam_resolution_height);

	tracker.cap_backend_api = (Cap_Backend_API)api;

	tracker.run_tracking(cam_index);
}

void _process_free_run(int cam_index, bool cam_resolution_use_default, int cam_resolution_width, int cam_resolution_height, int api, bool grayscale)
{
	tracker.resolution_default = cam_resolution_use_default;
	tracker.set_resolution(cam_resolution_width, cam_resolution_height);

	tracker.cap_backend_api = (Cap_Backend_API)api;

	tracker.run_free(cam_index);
}

void _process_video_flow(const char *vieo_file_path, bool debug, int conf_0_100, int nms_0_100, bool grayscale)
{
	tracker.is_debug = debug;

	tracker.conf = 0.01f * conf_0_100;
	tracker.nms = 0.01f * nms_0_100;

	tracker.run_tracking(vieo_file_path);
}

long _get_max_track_age()
{
	return tracker.max_track_age;
}

double _get_avg_track_length()
{
	return tracker.avg_track_length;
}

double _get_avg_track_age()
{
	return tracker.avg_track_age;
}

double _get_fps()
{
	return tracker.fps;
}

// mandatory settings before start

void _setup_yolo(const char *yolo_class_names, const char *yolo_cfg, const char *yolo_weights, bool rgb)
{
	tracker.yolo_names = yolo_class_names;
	tracker.yolo_cfg = yolo_cfg;
	tracker.yolo_weights = yolo_weights;

	tracker.grayscale_yolo = !rgb;
}

void _setup_resolution(bool use_default_resolution, int width, int height)
{
	tracker.resolution_default = use_default_resolution;
	tracker.set_resolution(width, height);
}

void _setup_inp_width_height(int inp_width_height)
{
	tracker.inp_width = inp_width_height;
	tracker.inp_height = inp_width_height;
}

// settings

void _set_dnn_thresholds(int conf_0_100, int nms_0_100)
{
	tracker.conf = 0.01f * conf_0_100;
	tracker.nms = 0.01f * nms_0_100;
}

void _set_aoi(int x, int y, int w, int h)
{
	tracker.set_aoi(CV_Rect(x, y, w, h));
}

void _set_flip_x(bool value)
{
	tracker.flip_x = value;
}

void _set_mask(const char *path)
{
	tracker.set_mask(path);
}

void _set_min_bounding_box(int value)
{
	tracker.min_bounding_box = value;
}

void _set_rect_distance_ratio(double value)
{
	tracker.rect_distance_ratio = value;
}

void _set_frame_delay(int value)
{
	tracker.frame_delay = value;
}

void _set_half_size_frame(bool value)
{
	tracker.half_size_frame = value;
}

void _set_on_item_callback(on_item_callback callback)
{
	tracker.bind(callback);
}

void _set_on_message_callback(on_message_callback callback)
{
	tracker.bind(callback);
}
