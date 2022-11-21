// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// https://github.com/opencv/opencv/issues/12058
#pragma managed(push, off)

#pragma warning(push)
#pragma warning(disable: 4793)
#pragma warning(disable: 26495)

#include <opencv2/opencv.hpp>

#include <string>
#include <fstream>

#pragma warning(pop)

// https://github.com/opencv/opencv/issues/12058
#pragma managed(pop)

#endif //PCH_H
