#include "pch.h"

#include "CV_Item_Tracker.h"

#include "CV_Utils.h"

#include "CV_Grouped_Boxes.h"
// #include "CV_Cam_Settings.h"

static cv::VideoCapture cap;

CV_Item_Tracker CV_Item_Tracker::inst;

class VideoCaptureCleaner // === panedrone: just ensures release on destruct
{
public:
	~VideoCaptureCleaner()
	{
		cap.release();
	}
};

CV_Item_Tracker::CV_Item_Tracker()
{
	// CV_Cam_Settings::set_cap(&cap);
}

void CV_Item_Tracker::set_mask(const char *mask)
{
	if (mask == nullptr)
	{
		_mat_mask = cv::Mat();
	}
	else
	{
		_mat_mask = cv::imread(mask, cv::IMREAD_GRAYSCALE);

		// _mat_mask = CV_Utils::Bitmap_24bppRgb_2_Mat(bmp_mask);

		// cv::cvtColor(_mat_mask, _mat_mask, cv::COLOR_BGR2GRAY);

		cv::threshold(_mat_mask, _mat_mask, 20, 255, cv::ThresholdTypes::THRESH_BINARY);
	}
}

void CV_Item_Tracker::set_aoi(const CV_Rect &aoi)
{
	if (aoi.Width == 0)
	{
		roi = NO_ROI;
	}
	else
	{
		roi.x = aoi.X;
		roi.y = aoi.Y;
		roi.width = aoi.Width;
		roi.height = aoi.Height;
	}

	post_message("ROI changed");
}

CV_Rect CV_Item_Tracker::get_aoi()
{
	// roi is cv::Rect

	if (roi.width == 0)
	{
		return CV_Rect(0, 0, 0, 0);
	}
	else
	{
		return CV_Rect(roi.x, roi.y, roi.width, roi.height);
	}
}

void CV_Item_Tracker::set_resolution(int width, int height)
{
	resolution_width = width;
	resolution_height = height;
}

void CV_Item_Tracker::open_camera(int cam_index)
{
	// https://github.com/opencv/opencv/issues/15989
	//
	// CAP_DSHOW = 700,          //!< DirectShow (via videoInput)
	//
	// Alternatively, you can switch to the MSMF-framework and zoom with CAP_PROP_ZOOM.
	// The surface-book camera is perfectly able to zoom.
	// It just doesn't work with the DSHOW-framework and CAP_PROP_ZOOM.

	// CAP_MSMF = 1400,         //!< Microsoft Media Foundation (via videoInput)

	post_message("================= my version is 1.0 ====================");

#define CAM_FMT_STR1(cap) "Trying to open the camera using "#cap
#define CAM_FMT_STR2(cap) "Camera is open using "#cap

	if (cap_backend_api == Cap_Backend_API::DSHOW)
	{
		post_message(CAM_FMT_STR1(cv::VideoCaptureAPIs::CAP_DSHOW));
		cap.open(cam_index + cv::VideoCaptureAPIs::CAP_DSHOW); // === panedrone: it is required to show settings dialog!!!
		post_message(CAM_FMT_STR2(cv::VideoCaptureAPIs::CAP_DSHOW));
	}
	else if (cap_backend_api == Cap_Backend_API::MSMF)
	{
		post_message(CAM_FMT_STR1(cv::VideoCaptureAPIs::CAP_MSMF));
		cap.open(cam_index + cv::VideoCaptureAPIs::CAP_MSMF);
		post_message(CAM_FMT_STR2(cv::VideoCaptureAPIs::CAP_MSMF));
	}
	else
	{
		post_message(CAM_FMT_STR1(cv::VideoCaptureAPIs::CAP_ANY));
		cap.open(cam_index);
		post_message(CAM_FMT_STR2(cv::VideoCaptureAPIs::CAP_ANY));
	}

	if (cap.isOpened() == false)
	{
		post_message("[ERROR] Cannot open the Camera. Try to use other Mode.");
		return;
	}
}

void CV_Item_Tracker::_preprocess_frame(cv::Mat &frame, bool flip_x, bool grayscale, bool half_size_frame)
{

	if (half_size_frame)
	{
		cv::resize(frame, frame, cv::Size(), 0.5, 0.5);
	}

	if (roi.width != 0)
	{
		try
		{
			frame(roi).copyTo(frame); // === panedrone: cut then show
		}
		catch (...)
		{
			throw cv::Exception(0, "Invalid AOI", "_preprocess_frame", "", 0);
		}
	}

	if (grayscale)
	{
		cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
	}

	if (flip_x) // === panedrone: flip of cropped is faster
	{
		cv::flip(frame, frame, 1);
	}

	//int erosion_type = cv::MORPH_RECT; //  cv::MORPH_ELLIPSE;
	//int erosion_elem = 3;

	//cv::Mat element = cv::getStructuringElement(erosion_type,
	//cv::Size(2 * erosion_elem + 1, 2 * erosion_elem + 1),
	//cv::Point(erosion_elem, erosion_elem));

	//cv::erode(frame, frame, element);
	//int erosion_type = cv::MORPH_RECT; //  cv::MORPH_ELLIPSE;
	//int erosion_elem = 3;

	//cv::Mat element = cv::getStructuringElement(erosion_type,
	//cv::Size(2 * erosion_elem + 1, 2 * erosion_elem + 1),
	//cv::Point(erosion_elem, erosion_elem));

	//cv::erode(frame, frame, element);
	//cv::blur(frame, frame, cv::Size(3, 3));
}

void CV_Item_Tracker::run_free(int cam_index)
{
	fps = 0;

	open_camera(cam_index);

	VideoCaptureCleaner vc; // release video on destruct

	if (resolution_default)
	{
		post_message("Default resolution");
	}
	else
	{
		cap.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH, resolution_width); //     =============== panedrone: after open!!!!!!!!!!
		cap.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT, resolution_height);

		char msg[1024];
		snprintf(msg, sizeof(msg), "Requested resolution: %d x %d", resolution_width, resolution_height);
		post_message(msg);
	}

	cv::Mat frame;

	cap >> frame;

	if (frame.empty())
	{
		post_message("[ERROR] No frame!");

		return;
	}

	char msg[1024];
	snprintf(msg, sizeof(msg), "Started at %d x %d", frame.cols, frame.rows);
	post_message(msg);

	static const std::string kWinName = "FreeRun";

	{
		cv::namedWindow(kWinName, cv::WindowFlags::WINDOW_AUTOSIZE);

		//cv::namedWindow(kWinName, cv::WindowFlags::WINDOW_NORMAL);

		//cv::resizeWindow(kWinName, frame.cols, frame.rows);

		cv::imshow(kWinName, frame); // show it before loop to adjust sizes
	}

	try
	{
		_is_running = true;

		while (_is_running)
		{
			double timer = (double)cv::getTickCount();

			cap >> frame;

			if (frame.empty())
			{
				cv::waitKey();

				post_message("No frame!");

				break;
			}

			_preprocess_frame(frame, flip_x, free_run_grayscale, half_size_frame);

			int key = cv::waitKey(1);

			if (key == 27) // ESC
			{
				break;
			}

			fps = cv::getTickFrequency() / ((double)cv::getTickCount() - timer);

			cv::imshow(kWinName, frame);
		}
	}
	catch (...)
	{
		post_message("Unknown C++ Error in Free Run Mode");
	}

	_is_running = false;

	cv::destroyWindow(kWinName); // on exit from while
}

void CV_Item_Tracker::run_dnn_object_detection(int cam_index)
{
	fps = 0;

	// === panedrone: expecting abs. path in here:

	std::string class_names_path(yolo_names);
	std::string model_path(yolo_cfg);
	std::string config_path(yolo_weights);

	std::string framework("");
	// std::string framework("darknet"); // === panedrone: seems that darknet is default

	// https://www.reddit.com/r/cpp_questions/comments/1sdw8l/fstream_only_accepts_absolute_path/
	// All filesystem APIs calls are relative to the current working directory,
	// NOT the directory in which the code resides or the directory
	// in which the executable resides, although those may coincide.
	// https://social.msdn.microsoft.com/Forums/vstudio/en-US/0708314f-7837-4618-8e32-7bf371862bd6/ofstream-opening-using-relative-paths?forum=vcgeneral
	// Inside the IDE it is the directory where your .vcproj file is.
	{
		std::ifstream ifs;
		const char *path = class_names_path.c_str();
		ifs.open(path); //, std::ifstream::in); // === panedrone: #include <fstream>
		if (!ifs.is_open())
		{
			char msg[1024];
			snprintf(msg, sizeof(msg), "[ERROR] File not found: %s", class_names_path.c_str());
			post_message(msg);
			return;
		}
		std::string line;
		while (std::getline(ifs, line))
		{
			classes.push_back(line);
			char msg[1024];
			snprintf(msg, sizeof(msg), "Class loaded: %s", line.c_str());
			post_message(msg);
		}
		ifs.close();
	}  // ensure destruct

	//parser.add_argument('--backend', choices = backends, default = cv.dnn.DNN_BACKEND_DEFAULT, type = int,
	//	help = "Choose one of computation backends: "
	//	"%d: automatically (by default), "
	//	"%d: Halide language (http://halide-lang.org/), "
	//	"%d: Intel's Deep Learning Inference Engine (https://software.intel.com/openvino-toolkit), "
	//	"%d: OpenCV implementation" % backends)

	// #define ASSIGN_VAR(var, value) var = value; post_message(#var ## " = " ## #value);

	cv::dnn::Backend backend;

	backend = cv::dnn::Backend::DNN_BACKEND_DEFAULT;
	// ASSIGN_VAR(backend, cv::dnn::Backend::DNN_BACKEND_DEFAULT);
	// ASSIGN_VAR(backend, cv::dnn::Backend::DNN_BACKEND_OPENCV);  // === panedrone: nothinng is changing in Task Maneger
	// ASSIGN_VAR(backend, cv::dnn::Backend::DNN_BACKEND_CUDA); // === panedrone: nothinng is changing in Task Maneger

	// "{ thr         | .5 | Confidence threshold. }"

	// conf_threshold = 0.5f;

	// "{ nms         | .4 | Non-maximum suppression threshold. }"

	// nms_threshold = 0.4f;

	//	add_argument(zoo, parser, 'mean', nargs = '+', type = float, default = [0, 0, 0],
	//		help = 'Preprocess input image by subtracting mean values. '
	//		'Mean values should be in BGR order.')

	// size_t asyncNumReq = parser.get<int>("async");

	//	parser.add_argument('--target', choices = targets, default = cv2.dnn.DNN_TARGET_CPU, type = int,
	//		help = 'Choose one of target computation devices: '
	//		'%d: CPU target (by default), '
	//		'%d: OpenCL, '
	//		'%d: OpenCL fp16 (half-float precision), '
	//		'%d: VPU' % targets)

	cv::dnn::Target target;

	target = cv::dnn::Target::DNN_TARGET_CPU;
	// ASSIGN_VAR(target, cv::dnn::Target::DNN_TARGET_CPU);
	// ASSIGN_VAR(target, cv::dnn::Target::DNN_TARGET_OPENCL);			// === panedrone: nothinng is changing in Task Maneger
	// ASSIGN_VAR(target, cv::dnn::Target::DNN_TARGET_CUDA);            // === panedrone: nothinng is changing in Task Maneger

	///////////////////////////////////////////////////////////

	cv::Scalar mean = cv::Scalar(0, 0, 0);

	//	add_argument(zoo, parser, 'scale', type = float, default = 1.0,
	//		help = 'Preprocess input image by multiplying on a scale factor.')

	float scale; // https://docs.opencv.org/master/da/d9d/tutorial_dnn_yolo.html

	scale = 1.0f / 255; // 0.00392f;

	// ASSIGN_VAR(scale, 0.00392f);

	//	add_argument(zoo, parser, 'rgb', action = 'store_true',
	//		help = 'Indicate that model works with RGB input images instead BGR ones.')

	bool swapRB = false; // https://docs.opencv.org/master/da/d9d/tutorial_dnn_yolo.html

	//	add_argument(zoo, parser, 'width', type = int,
	//		help = 'Preprocess input image by resizing to a specific width.')
	//	add_argument(zoo, parser, 'height', type = int,
	//		help = 'Preprocess input image by resizing to a specific height.')

	//int inpWidth = -1;  // 7000 ms in debug
	//int inpHeight = -1;

	// https://pjreddie.com/darknet/yolo/
	//
	// YOLOv3-320 	COCO trainval 	test-dev 	51.5 	38.97 Bn 	45
	//
	//const int INP_W = 320;
	//const int INP_H = INP_W;

	// _preprocess_frame:
	//
	// if (inpSize.width <= 0) inpSize.width = frame.cols;
	// if (inpSize.height <= 0) inpSize.height = frame.rows;

	char msg[1024];
	snprintf(msg, sizeof(msg), "BLOB: %d x %d, scale: %f, swapRB: %d", inp_width, inp_height, scale, swapRB);
	post_message(msg);

	/////////////////////////////////////////////////////////////
	// Load a model.

	if (!std::ifstream(model_path).good())
	{
		snprintf(msg, sizeof(msg), "[ERROR] File not found: %s", model_path.c_str());
		post_message(msg);
		return;
	}

	if (!std::ifstream(config_path).good())
	{
		snprintf(msg, sizeof(msg), "[ERROR] File not found: %s", config_path.c_str());
		post_message(msg);
		return;
	}

	if (grayscale_yolo)
	{
		post_message("YOLO Grayscale Mode");
	}
	else 
	{
		post_message("YOLO RGB Mode");
	}

	cv::dnn::Net net;

	try
	{
		net = cv::dnn::readNet(model_path, config_path, framework);
	}
	catch (...)
	{
		post_message("[ERROR] cv::dnn::readNet FAILED");
		return;
	}

	post_message("cv::dnn::readNet OK");
	post_message(model_path.c_str());
	post_message(config_path.c_str());

	/**
	 * @brief Ask network to use specific computation backend where it supported.
	 * @param[in] backendId backend identifier.
	 * @see Backend
	 *
	 * If OpenCV is compiled with Intel's Inference Engine library, DNN_BACKEND_DEFAULT
	 * means DNN_BACKEND_INFERENCE_ENGINE. Otherwise it equals to DNN_BACKEND_OPENCV.
	 */
	net.setPreferableBackend(backend);

	/**
	  * @brief Ask network to make computations on specific target device.
	  * @param[in] targetId target identifier.
	  * @see Target
	  *
	  * List of supported combinations backend / target:
	  * |                        | DNN_BACKEND_OPENCV | DNN_BACKEND_INFERENCE_ENGINE | DNN_BACKEND_HALIDE |  DNN_BACKEND_CUDA |
	  * |------------------------|--------------------|------------------------------|--------------------|-------------------|
	  * | DNN_TARGET_CPU         |                  + |                            + |                  + |                   |
	  * | DNN_TARGET_OPENCL      |                  + |                            + |                  + |                   |
	  * | DNN_TARGET_OPENCL_FP16 |                  + |                            + |                    |                   |
	  * | DNN_TARGET_MYRIAD      |                    |                            + |                    |                   |
	  * | DNN_TARGET_FPGA        |                    |                            + |                    |                   |
	  * | DNN_TARGET_CUDA        |                    |                              |                    |                 + |
	  * | DNN_TARGET_CUDA_FP16   |                    |                              |                    |                 + |
	  */

	net.setPreferableTarget(target);

	std::vector<cv::String> outNames = net.getUnconnectedOutLayersNames();

	// Open a video file or an image file or a camera stream.

	if (cap.isOpened())
	{
		post_message("[ERROR] Cannot open the Camera");

		return;
	}

	/////////////////////////////////////////////////////

	if (vieo_file_path_name.empty())
	{
		open_camera(cam_index);

		if (resolution_default)
		{
			post_message("Default resolution");
		}
		else
		{
			//const int FRAME_W = 1920;
			//const int FRAME_H = 1080;

			int FRAME_W = resolution_width; // 1280
			// const int FRAME_H = 960; // === panedrone: it resets to 720
			int FRAME_H = resolution_height; // 720;

			//const int FRAME_W = 640;
			//const int FRAME_H = 480;

			//const int FRAME_W = 800; // it is only what I want, webcam can change it
			//const int FRAME_H = 600; // 800 x 600 is reset on 640 x 480 at home

			//const int FRAME_W = 320; // 320 x 240 is reset on 640 x 480 at home
			//const int FRAME_H = 240;

			cap.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH, FRAME_W); //     =============== panedrone: after open!!!!!!!!!!
			cap.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT, FRAME_H);

			// post_message(String::Format("Requested camera resolution: {0} x {1}", FRAME_W, FRAME_H));
		}
	}
	else
	{
		cap.open(vieo_file_path_name);

		if (!cap.isOpened())
		{
			post_message("Cannot open video file:");
			post_message(vieo_file_path_name.c_str());
			return;
		}
	}

	VideoCaptureCleaner vc; // release video on destruct

	// Process frames.

	cv::Mat mat_painted, mat_original, blob;

	cap >> mat_original; // read 1st frame to estimate

	mat_original.copyTo(mat_painted);

	// char msg[1024];
	snprintf(msg, sizeof(msg), "Started at %d x %d, flip_x: %d, half_size_frame: %d", mat_painted.cols, mat_painted.rows, flip_x, half_size_frame);
	post_message(msg);

	snprintf(msg, sizeof(msg), "Conf: %.2f; Nms: %.2f", conf, nms);
	post_message(msg);

	if (mat_painted.empty())
	{
		post_message("[ERROR] No frame!");

		return;
	}

	// Create a window

	static const std::string win_name_dnn = "DNN Object Detection"; //  in OpenCV
	static const std::string win_name_source = "Source";			//  in OpenCV

	if (is_debug)
	{
		cv::namedWindow(win_name_dnn, cv::WindowFlags::WINDOW_AUTOSIZE);
		// cv::namedWindow(win_name_source, cv::WindowFlags::WINDOW_AUTOSIZE);

		// cv::imshow(kWinName, frame); // show it before loop to adjust sizes
	}

	//if (asyncNumReq)
	//	CV_Error(Error::StsNotImplemented, "Asynchronous forward is supported only with Inference Engine backend.");

	CV_Grouped_Boxes::init();

	try
	{
		_is_running = true;

		while (_is_running)
		{
			// Start timer

			double timer = (double)cv::getTickCount();

			cap >> mat_original;

			if (mat_original.empty())
			{
				if (is_debug)
				{
					// post_message(gcnew String("Ended. Click the Window '") + gcnew String(win_name_dnn.c_str()) + "' and press any Key...");

					// paint on prev. frame:

					cv::putText(mat_painted, std::string("Ended. Press any Key..."), cv::Point(30, mat_painted.rows - 30),
						cv::FONT_HERSHEY_SIMPLEX, 0.75, CV_RGB(200, 200, 200), 2);

					cv::imshow(win_name_dnn, mat_painted);

					cv::waitKey();
				}
				else
				{
					post_message("Ended...");
				}

				break;
			}

			_preprocess_frame(mat_original, flip_x, false, half_size_frame);

			if (is_debug)
			{
				// cv::imshow(win_name_source, mat_original);
			}

			mat_original.copyTo(mat_painted);

			_preprocess_frame(mat_painted, flip_x, grayscale_yolo, false /*half_size_frame ---- already done */);

			if (_mat_mask.empty() == false) // TODO: it is possible to optimize copyTo logic
			{
				// https://stackoverflow.com/questions/7479265/how-to-apply-mask-to-image-in-opencv/18161322#18161322
				// mat_painted.copyTo(mat_painted, _mat_mask);
				// https://stackoverflow.com/questions/7479265/how-to-apply-mask-to-image-in-opencv/18161322#18161322
				cv::bitwise_and(mat_painted, _mat_mask, mat_painted);
			}

			//////////////////////////////////////////////////

			int key = cv::waitKey(frame_delay);

			if (key == 27) // ESC
			{
				break;
			}
			else if (key == 32) // SPACE
			{
				post_message("ROI selection started...");

				cv::Mat mat_selection;

				if (flip_x)
				{
					cv::flip(mat_original, mat_selection, 1);
				}
				else
				{
					mat_original.copyTo(mat_selection); // show frame without text
				}

				//cv::putText(mat_painted, std::string(msg), cv::Point(30, mat_painted.rows - 30),
				//	cv::FONT_HERSHEY_SIMPLEX, 0.75, CV_RGB(200, 200, 200), 2);

				cv::imshow(win_name_dnn, mat_painted);

				std::string roi_window_name = "Select ROI, then press SPACE to confirm. Press 'C' or 'c' to cancel.";

				auto tmp_roi = cv::selectROI(roi_window_name, mat_selection, /* showCrosshair */ false); // === panedrone: it is modal

				if (tmp_roi.width > 0) // === panedrone: it is 0 is [C, c] pressed
				{
					post_message("ROI changed");

					roi = tmp_roi;

					cv::resizeWindow(win_name_dnn, roi.width, roi.height);
				}
				else
				{
					post_message("ROI selection cancelled");
				}

				// https://answers.opencv.org/question/215788/selectroi-does-not-close-the-window-after-enter-pressed/
				// So pressing space simply finishes the selection, it doesn't close the window. The solution is to manually delete the window, just as you did.

				cv::destroyWindow(roi_window_name);
			}

			////////////////////////////////////////////////////////

			_pre_process(mat_painted, blob, net, cv::Size(inp_width, inp_height), scale, mean, swapRB);

			std::vector<cv::Mat> outs;

			try
			{
				net.forward(outs, outNames);
			}
			catch (...)
			{
				post_message("[ERRROR] net.forward FAILED. check grayscale/rgb.");
				return;
			}

			cv::Point gc;

			double score = _post_process(mat_painted, mat_original, outs, net, gc);

			// Calculate FPS === panedrone: considering merging boxes, tracking, callbacks with cropped images, etc.

			fps = cv::getTickFrequency() / ((double)cv::getTickCount() - timer);

			if (key == 'p' || key == 'P') // === panedrone: pause capturing/playing
			{
				cv::Mat mat_paused;

				mat_painted.copyTo(mat_paused);

				cv::putText(mat_paused, std::string("Paused..."), cv::Point(30, mat_painted.rows - 30),
					cv::FONT_HERSHEY_SIMPLEX, 0.75, CV_RGB(200, 200, 200), 2);

				cv::imshow(win_name_dnn, mat_paused);

				cv::waitKey(); // === panedrone: wait infiniteliy till any key pressed
			}

			if (is_debug)
			{
				cv::imshow(win_name_dnn, mat_painted);
			}
		}
	}
	catch (...)
	{
		post_message("Unknown C++ Error in Tracking Mode");
	}

	_is_running = false;

	cv::destroyAllWindows(); // on exit from while
}

void CV_Item_Tracker::_pre_process(const cv::Mat &frame, cv::Mat &blob, cv::dnn::Net &net, cv::Size inpSize, float scale, const cv::Scalar &mean, bool swapRB)
{
	// static cv::Mat blob;
	// Create a 4D blob from a frame.

	if (inpSize.width <= 0)
	{
		inpSize.width = frame.cols;
	}

	if (inpSize.height <= 0)
	{
		inpSize.height = frame.rows;
	}

	cv::dnn::blobFromImage(frame, blob, 1.0, inpSize, cv::Scalar(), swapRB, false, CV_8U);

	// int ch = blob.channels();

	// === panedrone:

	// blob - єто какая-то четірехмерная хрень, см. оригинальній комент "Create a 4D blob from a frame"
	// а четірехмерная она наверное вот почему:

	// [region]
	// anchors = 0.57273, 0.677385, 1.87446, 2.06253, 3.33843, 5.47434, 7.88282, 3.52778, 9.77052, 9.16828
	//	bias_match = 1
	//	classes = 10
	//	coords = 4
	//	num = 5
	//	softmax = 1
	//	jitter = .2
	//	rescore = 0

	//cv::Mat blobN;
	//cv::normalize(blob, blobN, 0, 255);
	// cv::imwrite("d:\\++++++++++++blob.bmp", blobN);
	// cv::imshow("dnn BLOB", blobN);

	// Run a model.
	net.setInput(blob, "", scale, mean); // === panedrone: just copy-pasted from https://github.com/opencv/opencv/blob/master/samples/dnn/object_detection.cpp

	if (net.getLayer(0)->outputNameToIndex("im_info") != -1) // Faster-RCNN or R-FCN
	{
		resize(frame, frame, inpSize);
		cv::Mat imInfo = (cv::Mat_<float>(1, 3) << inpSize.height, inpSize.width, 1.6f);
		net.setInput(imInfo, "im_info");
	}
}

double CV_Item_Tracker::_post_process(cv::Mat &mat_painted, const cv::Mat &mat_original, const std::vector<cv::Mat> &outs, cv::dnn::Net &net, cv::Point &gc)
{
	gc.x = -1;
	gc.y = -1;

	static std::vector<int> outLayers = net.getUnconnectedOutLayers();
	static std::string outLayerType = net.getLayer(outLayers[0])->type;

	std::vector<int> class_ids;
	std::vector<float> confidences;
	std::vector<cv::Rect> boxes;

	// double best_confidence = -1;

	if (outLayerType == "DetectionOutput")
	{
		// Network produces output blob with a shape 1x1xNx7 where N is a number of
		// detections and an every detection is a vector of values
		// [batchId, classId, confidence, left, top, right, bottom]
		CV_Assert(outs.size() > 0);
		for (size_t k = 0; k < outs.size(); k++)
		{
			float *data = (float *)outs[k].data;
			for (size_t i = 0; i < outs[k].total(); i += 7)
			{
				float confidence = data[i + 2];
				if (confidence > conf)
				{
					int left = (int)data[i + 3];
					int top = (int)data[i + 4];
					int right = (int)data[i + 5];
					int bottom = (int)data[i + 6];
					int width = right - left + 1;
					int height = bottom - top + 1;
					if (width <= 2 || height <= 2)
					{
						left = (int)(data[i + 3] * mat_painted.cols);
						top = (int)(data[i + 4] * mat_painted.rows);
						right = (int)(data[i + 5] * mat_painted.cols);
						bottom = (int)(data[i + 6] * mat_painted.rows);
						width = right - left + 1;
						height = bottom - top + 1;
					}
					class_ids.push_back((int)(data[i + 1]) - 1); // Skip 0th background class id.
					boxes.push_back(cv::Rect(left, top, width, height));
					confidences.push_back(confidence);
				}
			}
		}
	}
	else if (outLayerType == "Region")
	{
		for (size_t i = 0; i < outs.size(); ++i)
		{
			// Network produces output blob with a shape NxC where N is a number of
			// detected objects and C is a number of classes + 4 where the first 4
			// numbers are [center_x, center_y, width, height]
			float *data = (float *)outs[i].data;
			for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
			{
				cv::Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
				cv::Point classIdPoint;
				double confidence;
				cv::minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
				if (confidence > conf)
				{
					int centerX = (int)(data[0] * mat_painted.cols);
					int centerY = (int)(data[1] * mat_painted.rows);
					int width = (int)(data[2] * mat_painted.cols);
					int height = (int)(data[3] * mat_painted.rows);
					int left = centerX - width / 2;
					int top = centerY - height / 2;

					int class_id = classIdPoint.x;

					class_ids.push_back(class_id);
					confidences.push_back((float)confidence);
					boxes.push_back(cv::Rect(left, top, width, height));

					gc.x = left + width / 2;
					gc.y = top + height / 2;
				}
			}
		}
	}
	else
	{
		CV_Error(cv::Error::StsNotImplemented, "Unknown output layer type: " + outLayerType);
	}

	std::vector<int> indices;

	cv::dnn::NMSBoxes(boxes, confidences, conf, nms, indices);

	if (grayscale_yolo)
	{
		cv::cvtColor(mat_painted, mat_painted, cv::COLOR_GRAY2BGR);
	}

	std::vector<cv::Rect> filtered_boxes; // === panedrone: collect filtered boxes to use in CV_Grouped_Boxes
	std::vector<float> filtered_confidences;

	for (size_t i = 0; i < indices.size(); ++i)
	{
		int idx = indices[i];

		cv::Rect box = boxes[idx];
		float conf = confidences[idx];

		filtered_boxes.push_back(box);
		filtered_confidences.push_back(conf);

		_draw_pred(class_ids[idx], conf, box.x, box.y, box.x + box.width, box.y + box.height, mat_painted);
	}

	CV_Grouped_Boxes::process_groups(mat_painted, mat_original, rect_distance_ratio, filtered_boxes, filtered_confidences);

	return 0;
}

void CV_Item_Tracker::_draw_pred(int classId, float conf, int left, int top, int right, int bottom, cv::Mat &frame)
{
	int box_width = right - left;
	int box_height = bottom - top;

	if (box_width < min_bounding_box && box_height < min_bounding_box)
	{
		cv::rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), CV_RGB(255, 0, 0));

		return;
	}

	cv::rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), CV_RGB(0, 0, 255));

	std::string label = cv::format("%.2f", conf);

	//if (!classes.empty())
	//{
	//	CV_Assert(classId < (int)classes.size());
	//	label = classes[classId] + ": " + label;
	//}

	int baseLine;

	cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

	top = std::max(top, labelSize.height);

	left += 2;

	cv::rectangle(frame, cv::Point(left, top),
		cv::Point(left + labelSize.width, top + baseLine + labelSize.height), cv::Scalar::all(255), cv::LineTypes::FILLED);

	cv::putText(frame, label, cv::Point(left, top + labelSize.height), cv::FONT_HERSHEY_PLAIN, 0.75, cv::Scalar());
}
