#pragma once
class CV_Grouped_Boxes
{
public:

	static void init();

	static void process_groups(cv::Mat &mat_painted, const cv::Mat &mat_original, const double rect_distance_ratio, const std::vector<cv::Rect> &boxes, const std::vector<float> &confidences)
	{
		// === panedrone: assign a group for each box

		std::vector<int> group_ids;

		assign_groups(rect_distance_ratio, boxes, group_ids);

		process_groups(mat_painted, mat_original, boxes, confidences, group_ids);
	}

private:

	static void assign_groups(const double rect_distance_ratio, const std::vector<cv::Rect> &boxes, std::vector<int> &group_ids);

	static void process_groups(cv::Mat &mat_painted, const cv::Mat &mat_original, const std::vector<cv::Rect> &boxes, const std::vector<float> &confidences, const std::vector<int> &group_ids);
};
