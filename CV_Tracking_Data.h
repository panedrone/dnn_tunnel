#pragma once

#include <opencv2/opencv.hpp>

class CV_Tracking_Data
{
private:

	int curr_frame_no = 0;

	std::vector<cv::Point> prev_gc;

	std::vector<int64> prev_gc_frame_indexes;

	void remove_track_at(size_t i)
	{
		prev_gc.erase(prev_gc.begin() + i);

		prev_gc_frame_indexes.erase(prev_gc_frame_indexes.begin() + i);
	}

public:

	void reset();

	size_t get_tracks_count();

	const cv::Point& get_track(size_t i);

	void next_frame();

	void add_track(const cv::Point &gc);

	void remove_unused();

	bool find_prev(const int MAX_ALLOWED_DISTANCE, const cv::Point &curr_gc, cv::Point &res);
};
