#include "pch.h"
#include "CV_Grouped_Boxes.h"

#include "CV_Utils.h"

#include "CV_Item_Tracker.h"

#include "CV_Tracking_Data.h"

const int TRIGGER_BAND_HEIGHT = 120;

const int CIRCLE_RADIUS = 3;

static CV_Tracking_Data _tracking_data;

static double _conf_sum = 0;

static int _items_count = 0;

/////////////////////////////////////////////////////////////////////////////

void CV_Grouped_Boxes::init()
{
	_tracking_data.reset();

	_items_count = 0;

	_conf_sum = 0;
}

cv::Rect _item_rect_to_crop_rect(const cv::Mat &mat_original, const cv::Rect &r_src)
{
	cv::Rect r_dst = r_src;

	if (r_dst.width < 300)
	{
		r_dst.width = (int)(r_dst.width * 1.3);
		r_dst.height = (int)(r_dst.height * 1.3);
	}
	else
	{
		r_dst.width += 50;
		r_dst.height += 50;
	}

	const cv::Point gc_src(r_src.x + r_src.width / 2, r_src.y + r_src.height / 2);

	r_dst.x = gc_src.x - r_dst.width / 2;
	r_dst.y = gc_src.y - r_dst.height / 2;

	if (r_dst.x < 0)
	{
		r_dst.x = 0;
	}

	if (r_dst.y < 0)
	{
		r_dst.y = 0;
	}

	if (r_dst.width >= mat_original.cols - r_dst.x - 1)
	{
		r_dst.width = mat_original.cols - r_dst.x - 1;
	}

	if (r_dst.height >= mat_original.rows - r_dst.y - 1)
	{
		r_dst.height = mat_original.rows - r_dst.y - 1;
	}

	return r_dst;
}

static void _get_gc(const cv::Rect &rect, cv::Point &gc)
{
	int a = rect.width / 2;
	int b = rect.height / 2;

	gc.x = rect.x + a;
	gc.y = rect.y + b;
}

static double _get_critical_distance(const cv::Rect &rect)
{
	//      /|
	//     / |  
	//    +  |
	//   /   | b
	//  /____|
	//    a

	int a = rect.width;
	int b = rect.height;

	double c = sqrt(a * a + b * b) / 2;

	return c;
}

static void _find_nearest(const double rect_distance_ratio, const std::vector<cv::Rect> &boxes, int curr_box_index, std::vector<int> &nearest_box_indexes)
{
	if (boxes.size() == 0)
	{
		return;
	}

	//   _____
	//  | +  /|
	//  |   / |
	//  |  +  |
	//  | /   |
	//  |/____|
	//

	cv::Point gc0;

	_get_gc(boxes[curr_box_index], gc0);

	for (int i = 0; i < boxes.size(); i++)
	{
		if (i == curr_box_index)
		{
			continue;
		}

		cv::Point gc1;

		_get_gc(boxes[i], gc1);

		double critical_distance = _get_critical_distance(boxes[i])  * rect_distance_ratio;

		double dist = CV_Utils::get_distance(gc0, gc1);

		if (dist < critical_distance)
		{
			nearest_box_indexes.push_back(i);
		}
	}
}

static int _get_nearest_group_id(const std::vector<int> &nearest_box_indexes)
{
	int nearest_group_index = -1;

	for (int i = 0; i < nearest_box_indexes.size(); i++)
	{
		if (nearest_box_indexes[i] != -1)
		{
			return nearest_box_indexes[i];
		}
	}

	return nearest_group_index;
}

static void _assign_group_id(std::vector<int> &box_indexes, int group_id)
{
	for (int i = 0; i < box_indexes.size(); i++)
	{
		box_indexes[i] = group_id;
	}
}

void CV_Grouped_Boxes::assign_groups(const double rect_distance_ratio, const std::vector<cv::Rect> &boxes, std::vector<int> &group_ids)
{
	int next_group_id = -1;

	for (int i = 0; i < boxes.size(); i++)
	{
		std::vector<int> nearest_box_indexes;

		_find_nearest(rect_distance_ratio, boxes, i, nearest_box_indexes);

		if (group_ids.size() <= i)
		{
			group_ids.push_back(-1);
		}

		int nearest_group_id = _get_nearest_group_id(nearest_box_indexes);

		if (nearest_group_id == -1)
		{
			group_ids[i] = ++next_group_id;
		}
		else
		{
			group_ids[i] = nearest_group_id;
		}

		CV_Assert(group_ids[i] != -1);

		_assign_group_id(nearest_box_indexes, group_ids[i]);
	}
}

static void _process_group(int min_line_y, int max_line_y, const cv::Mat &mat_painted, const cv::Mat &mat_original, const std::vector<cv::Rect> &boxes, const std::vector<float> &confidences)
{
	if (boxes.size() == 0)
	{
		return;
	}

	std::vector<cv::Point> points;

	float avg_conf = 0;

	for (int i = 0; i < boxes.size(); i++)
	{
		const cv::Rect &b = boxes[i];

		int left = b.x;
		int top = b.y;
		int right = left + b.width;
		int bottom = top + b.height;

		points.push_back(cv::Point(left, top));
		points.push_back(cv::Point(left, bottom));
		points.push_back(cv::Point(right, top));
		points.push_back(cv::Point(right, bottom));

		if (confidences.size() > 0)
		{
			avg_conf += confidences[i];
		}
	}

	if (confidences.size() > 0)
	{
		avg_conf /= confidences.size();
	}

	cv::Rect item_rect = cv::boundingRect(points);

	cv::Scalar markup_color = CV_RGB(20, 100, 20); //CV_RGB(34, 177, 76); // CV_RGB(200, 100, 0);

	cv::Scalar track_color = markup_color; //  CV_RGB(180, 180, 0);

	cv::rectangle(mat_painted, item_rect, CV_RGB(255, 255, 0), 1, cv::LineTypes::LINE_4);

	const cv::Point gc(item_rect.x + item_rect.width / 2, item_rect.y + item_rect.height / 2);

	int MAX_ALLOWED_DISTANCE = (int)((double)(item_rect.width + item_rect.height) / 2 * 0.4);

	if (MAX_ALLOWED_DISTANCE < 50)
	{
		MAX_ALLOWED_DISTANCE = 50;
	}

	cv::circle(mat_painted, gc, MAX_ALLOWED_DISTANCE, CV_RGB(0, 255, 0), 1, cv::LineTypes::LINE_AA);

	cv::circle(mat_painted, gc, CIRCLE_RADIUS, markup_color, cv::LineTypes::FILLED);

	cv::Point prev_gc;

	if (_tracking_data.find_prev(MAX_ALLOWED_DISTANCE, gc, prev_gc))
	{
		if (prev_gc.x == 0 || prev_gc.y == 0)
		{
			CV_Item_Tracker::post_message("[ERROR] Invalid track");

			return;
		}

		cv::line(mat_painted, prev_gc, gc, track_color, 1, cv::LineTypes::LINE_AA);

		cv::circle(mat_painted, prev_gc, CIRCLE_RADIUS, track_color, cv::LineTypes::FILLED); // repaint red one with green

		return;
	}

	if (gc.y >= min_line_y && gc.y <= max_line_y)
	{
		cv::Rect rect_crop = _item_rect_to_crop_rect(mat_original, item_rect);

		cv::Mat mat_crop = mat_original(rect_crop);

		cv::Mat mat_gray;
		cv::cvtColor(mat_crop, mat_gray, cv::COLOR_BGR2GRAY);
		int low_threshold = 50;
		int ratio = 3;
		int kernel_size = 3;
		cv::Canny(mat_gray, mat_gray, low_threshold, low_threshold * ratio, kernel_size);

		int area = cv::countNonZero(mat_gray);

		if (area < 50)
		{
			CV_Item_Tracker::post_message("Dirt on the belt?");
		}
		else
		{
			cv::circle(mat_painted, gc, CIRCLE_RADIUS * 3, CV_RGB(255, 0, 255), cv::LineTypes::FILLED); // paint GC in orange

			_tracking_data.add_track(gc);

			_items_count++;

			_conf_sum += avg_conf;

			{
				std::vector<unsigned char> bytes;
				cv::imencode(".jpg", mat_crop, bytes);
				int bytes_count = (int)bytes.size();
				CV_Item_Tracker::post_image(bytes_count, &bytes.begin()[0]);
			}

			char msg[1024]; // (int)boxes.size(): https://stackoverflow.com/questions/5140871/sprintf-for-unsigned-int64
			snprintf(msg, sizeof(msg), "%0.3f; boxes: %d; conf_avg: %0.3f", avg_conf, (int)boxes.size(), _conf_sum / _items_count);
			CV_Item_Tracker::post_message(msg);
		}
	}
}

void CV_Grouped_Boxes::process_groups(cv::Mat &mat_painted, const cv::Mat &mat_original, const std::vector<cv::Rect> &boxes, const std::vector<float> &confidences, const std::vector<int> &group_ids)
{
	_tracking_data.next_frame();

	cv::Scalar line_color = CV_RGB(255, 0, 60);

	int min_line_y = mat_painted.rows / 2 - TRIGGER_BAND_HEIGHT / 2; // === top line must be higher because tall items cover the edges of belt
	int max_line_y = mat_painted.rows / 2 + TRIGGER_BAND_HEIGHT / 2; // === on other hand, if it is too hight, yolo starts working at the last moment, and it makes to be nerveous )

	cv::line(mat_painted, cv::Point(0, min_line_y), cv::Point(mat_painted.cols, min_line_y), line_color, 1, cv::LineTypes::LINE_4);
	cv::line(mat_painted, cv::Point(0, max_line_y), cv::Point(mat_painted.cols, max_line_y), line_color, 1, cv::LineTypes::LINE_4);

	typedef std::map<int, std::vector<cv::Rect>> GroupsMap;
	GroupsMap groups;

	typedef std::map<int, std::vector<float>> GroupsConf;
	GroupsConf groups_conf;

	for (int i = 0; i < group_ids.size(); i++)
	{
		int group_id = group_ids[i];

		groups[group_id].push_back(boxes[i]);

		float conf = confidences[i];

		groups_conf[group_id].push_back(conf);
	}

	// https://stackoverflow.com/questions/4207346/how-can-i-traverse-iterate-an-stl-map

	for (auto it : groups) // for (MyMap::const_iterator it = my_map.begin(); it != my_map.end(); ++it)
	{
		const std::vector<cv::Rect> &boxes_of_group = it.second;

		int group_id = it.first;

		const std::vector<float> &conf_of_group = groups_conf[group_id];

		_process_group(min_line_y, max_line_y, mat_painted, mat_original, boxes_of_group, conf_of_group);
	}

	_tracking_data.remove_unused();

	for (int i = 0; i < _tracking_data.get_tracks_count(); i++)
	{
		const cv::Point &trigger_point = _tracking_data.get_track(i);

		cv::circle(mat_painted, trigger_point, CIRCLE_RADIUS, CV_RGB(0, 255, 0), cv::LineTypes::FILLED);
	}

	std::string label = cv::format("%d", _items_count);

	cv::putText(mat_painted, label, cv::Point(20, 40), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(180, 180, 180), 2);
}
