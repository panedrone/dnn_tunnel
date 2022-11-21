#pragma once

#include <opencv2/opencv.hpp>

class CV_Utils
{
public:

	//static cv::Mat Bitmap_24bppRgb_2_Mat(System::Drawing::Bitmap^ bmp);

	//static System::Drawing::Bitmap^ Mat_2_Bmp(cv::InputArray mat);

	//static System::Drawing::Bitmap^ Mat_2_Bmp_2(const cv::Mat& inputImage);

	//static System::Drawing::Image^ Bytes_2_Image(array<System::Byte>^ bytes);

	static void drawAxis(cv::InputOutputArray img, cv::Point p, cv::Point q, cv::Scalar colour, const float scale = 0.2);

	static void ensure_gray_scale(cv::UMat& umat_source);

	static void ensure_binary(cv::UMat& umat, int threshold);

	static void ensure_gray_scale(cv::InputOutputArray umat_source);

	static void ensure_bgr(cv::UMat& umat_source);

	static void rotate(const cv::Point2d& center, double angle_degrees, cv::Mat& umat_source);

	static void rect_to_rotated_box(const cv::Rect& rect, cv::RotatedRect& res);

	static void repaint_binary(cv::UMat& umat, const cv::Scalar& color);

	//static cv::Mat Bitmap_32bppArgb_2_Mat(System::Drawing::Bitmap^ bmp);

	static double get_distance(const cv::Point &gc0, const cv::Point &gc1)
	{
		double dx = gc0.x - gc1.x;
		double dy = gc0.y - gc1.y;

		double dist = sqrt(dx * dx + dy * dy); // === panedrone: no div 2!!!

		return dist;
	}
};

