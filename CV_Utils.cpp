#include "pch.h"
#include "CV_Utils.h"

void CV_Utils::repaint_binary(cv::UMat& umat, const cv::Scalar& color)
{
	// https://stackoverflow.com/questions/20981379/c-change-color-in-cvmat-with-setto

	cv::Scalar color_white(255, 255, 255);

	cv::UMat mask;
	cv::inRange(umat, color_white, color_white, mask);

	umat.setTo(color, mask);
}

void CV_Utils::rect_to_rotated_box(const cv::Rect& rect, cv::RotatedRect& res)
{
	cv::Point2f center((float)(rect.x + rect.width / 2), (float)(rect.y + rect.height / 2));

	cv::Size2f size((float)rect.width, (float)rect.height);

	float angle = 0;

	res = cv::RotatedRect(center, size, angle);
}

void CV_Utils::rotate(const cv::Point2d& center, double angle_degrees, cv::Mat& umat_source)
{
	// OpenCV Python rotate image by X degrees around specific point
// https://stackoverflow.com/questions/9041681/opencv-python-rotate-image-by-x-degrees-around-specific-point

	cv::Mat M = cv::getRotationMatrix2D(center, angle_degrees, 1);

	// https://stackoverflow.com/questions/3112364/how-do-i-choose-an-image-interpolation-method-emgu-opencv
	// Nearest neighbor will be as fast as possible, but you will lose substantial information when resizing.
	// Linear interpolation is less fast, but will not result in information loss unless you're shrinking the image (which you are).
	// Cubic interpolation(probably actually "Bicubic") uses one of many possible formulas that incorporate multiple neighbor pixels

	cv::warpAffine(umat_source, umat_source, M, umat_source.size(), cv::INTER_NEAREST); // source maybe not preprocessed 3 channel image, then result i also 3 channel
}

void CV_Utils::ensure_gray_scale(cv::UMat& umat_source)
{
	// некоторые OpenCV функции выбрасывают искрючение при разной глубине картинок. например BMP и PNG

	int ch_s = umat_source.channels();

	if (ch_s == 4)
	{
		cv::cvtColor(umat_source, umat_source, cv::COLOR_BGRA2GRAY);
	}
	else if (ch_s == 3)
	{
		cv::cvtColor(umat_source, umat_source, cv::COLOR_BGR2GRAY);
	}
}

void CV_Utils::ensure_binary(cv::UMat& umat, int threshold)
{
	int ch = umat.channels();

	if (ch > 1)
	{
		if (ch > 2)
		{
			cv::UMat umat_tmp;

			cv::cvtColor(umat, umat_tmp, cv::ColorConversionCodes::COLOR_BGR2GRAY);

			cv::threshold(umat_tmp, umat, threshold, 255, cv::THRESH_BINARY);	// creates 1 channel img
		}
		else
		{
			cv::threshold(umat, umat, threshold, 255, cv::THRESH_BINARY);	// creates 1 channel img
		}
	}
}

void CV_Utils::ensure_bgr(cv::UMat& umat_source)
{
	// некоторые OpenCV функции выбрасывают искрючение при разной глубине картинок. например BMP и PNG

	int ch2 = umat_source.channels();

	if (ch2 < 3)
	{
		cv::cvtColor(umat_source, umat_source, cv::ColorConversionCodes::COLOR_GRAY2BGR);
	}
	else if (ch2 == 4)
	{
		cv::cvtColor(umat_source, umat_source, cv::ColorConversionCodes::COLOR_BGRA2BGR);
	}
}


void CV_Utils::ensure_gray_scale(cv::InputOutputArray umat_source)
{
	// некоторые OpenCV функции выбрасывают искрючение при разной глубине картинок. например BMP и PNG

	int ch_s = umat_source.channels();

	if (ch_s == 4)
	{
		cv::cvtColor(umat_source, umat_source, cv::COLOR_BGRA2GRAY);
	}
	else if (ch_s == 3)
	{
		cv::cvtColor(umat_source, umat_source, cv::COLOR_BGR2GRAY);
	}
}

// https://docs.opencv.org/3.4/d1/dee/tutorial_introduction_to_pca.html

void CV_Utils::drawAxis(cv::InputOutputArray img, cv::Point p, cv::Point q, cv::Scalar colour, const float scale)
{
	double angle = atan2((double)p.y - q.y, (double)p.x - q.x); // angle in radians
	double hypotenuse = sqrt((double)(p.y - q.y) * (p.y - q.y) + (p.x - q.x) * (p.x - q.x));
	// Here we lengthen the arrow by a factor of scale
	q.x = (int)(p.x - scale * hypotenuse * cos(angle));
	q.y = (int)(p.y - scale * hypotenuse * sin(angle));
	line(img, p, q, colour, 1, cv::LINE_AA);
	// create the arrow hooks
	p.x = (int)(q.x + 9 * cos(angle + CV_PI / 4));
	p.y = (int)(q.y + 9 * sin(angle + CV_PI / 4));
	line(img, p, q, colour, 1, cv::LINE_AA);
	p.x = (int)(q.x + 9 * cos(angle - CV_PI / 4));
	p.y = (int)(q.y + 9 * sin(angle - CV_PI / 4));
	line(img, p, q, colour, 1, cv::LINE_AA);
}

//System::Drawing::Image^ CV_Utils::Bytes_2_Image(array<System::Byte>^ bytes)
//{
//	// https://stackoverflow.com/questions/338950/what-is-the-managed-c-equivalent-to-the-c-sharp-using-statement
//	// To to that in Managed C++ just use stack semantics.
//
//	System::IO::MemoryStream stream(bytes);
//
//	stream.Flush();
//
//	return System::Drawing::Image::FromStream(% stream, true);
//}
//
//System::Drawing::Bitmap^ CV_Utils::Mat_2_Bmp_2(const cv::Mat& inputImage)
//{
//	System::Drawing::Bitmap^ bitmap;
//
//	System::IntPtr matPointer((void*)inputImage.ptr());
//
//	if (inputImage.channels() == 4)
//	{
//		bitmap = gcnew System::Drawing::Bitmap(inputImage.cols, inputImage.rows, (int)inputImage.step1(),
//			System::Drawing::Imaging::PixelFormat::Format32bppArgb, matPointer);
//
//	}// ARGB
//	else if (inputImage.channels() == 3)
//	{
//		bitmap = gcnew System::Drawing::Bitmap(inputImage.cols, inputImage.rows, (int)inputImage.step1(),
//			System::Drawing::Imaging::PixelFormat::Format24bppRgb, matPointer);
//
//	}// RGB
//	else
//	{
//		bitmap = gcnew System::Drawing::Bitmap(inputImage.cols, inputImage.rows, (int)inputImage.step1(),
//			System::Drawing::Imaging::PixelFormat::Format16bppGrayScale, matPointer);
//
//	}// RGB
//
//	return bitmap;
//}
//
//System::Drawing::Bitmap^ CV_Utils::Mat_2_Bmp(cv::InputArray mat)
//{
//	if (mat.cols() == 0 || mat.rows() == 0)
//	{
//		throw gcnew System::Exception("Invalid input of Mat_2_Bmp");
//	}
//
//	std::vector<uchar> buf;
//
//	cv::imencode(".bmp", mat, buf);
//
//	array<System::Byte>^ _Data = gcnew array<System::Byte>((int)buf.size());
//
//	try
//	{
//		System::Runtime::InteropServices::Marshal::Copy(System::IntPtr((void*)&buf.front()), _Data, 0, _Data->GetLength(0));
//
//		return safe_cast<System::Drawing::Bitmap^>(Bytes_2_Image(_Data));
//	}
//	finally
//	{
//		delete _Data;
//	}
//}
//
//cv::Mat CV_Utils::Bitmap_24bppRgb_2_Mat(System::Drawing::Bitmap^ bmp)
//{
//	if (bmp == nullptr)
//	{
//		throw gcnew System::Exception("Input Image?");
//	}
//
//	if (bmp->PixelFormat != System::Drawing::Imaging::PixelFormat::Format24bppRgb)
//	{
//		throw gcnew System::Exception("PixelFormat::Format24bppRgb expected");
//	}
//
//	System::Drawing::Imaging::BitmapData^ bmpData =
//		bmp->LockBits(System::Drawing::Rectangle(0, 0, bmp->Width, bmp->Height),
//			System::Drawing::Imaging::ImageLockMode::ReadWrite, bmp->PixelFormat);
//
//	try
//	{
//		// https://stackoverflow.com/questions/59421739/c-cli-convert-bitmap-to-opencv-mat
//
//		int wb = ((bmp->Width * 24 + 31) / 32) * 4;
//
//		// https://stackoverflow.com/questions/29018442/systemdrawingbitmap-to-cvmat-opencv-c-cli
//
//		cv::Mat mat(cv::Size(bmp->Width, bmp->Height), CV_8UC3, bmpData->Scan0.ToPointer(), wb /*cv::Mat::AUTO_STEP*/);
//
//		return mat;
//	}
//	finally
//	{
//		bmp->UnlockBits(bmpData);
//	}
//}

//cv::Mat CV_Utils::Bitmap_32bppArgb_2_Mat(System::Drawing::Bitmap^ bmp)
//{
//	if (bmp == nullptr)
//	{
//		throw gcnew System::Exception("Input Image?");
//	}
//
//	if (bmp->PixelFormat != System::Drawing::Imaging::PixelFormat::Format32bppArgb)
//	{
//		throw gcnew System::Exception("PixelFormat::Format32bppArgb expected");
//	}
//
//	System::Drawing::Imaging::BitmapData^ bmpData =
//		bmp->LockBits(System::Drawing::Rectangle(0, 0, bmp->Width, bmp->Height),
//			System::Drawing::Imaging::ImageLockMode::ReadWrite, bmp->PixelFormat);
//
//	try
//	{
//		// https://stackoverflow.com/questions/59421739/c-cli-convert-bitmap-to-opencv-mat
//
//		int wb = ((bmp->Width * 32 + 31) / 32) * 4;
//
//		// https://stackoverflow.com/questions/29018442/systemdrawingbitmap-to-cvmat-opencv-c-cli
//
//		cv::Mat mat(cv::Size(bmp->Width, bmp->Height), CV_8UC4, bmpData->Scan0.ToPointer(), wb /*cv::Mat::AUTO_STEP*/);
//
//		return mat;
//	}
//	finally
//	{
//		bmp->UnlockBits(bmpData);
//	}
//}