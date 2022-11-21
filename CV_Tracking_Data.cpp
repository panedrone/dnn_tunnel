#include "pch.h"
#include "CV_Tracking_Data.h"

#include "CV_Item_Tracker.h"
#include "CV_Utils.h"

static double _total_track_length = 0;
static double _total_track_age = 0;
static int64 _total_tracks_count = 0;

void CV_Tracking_Data::reset()
{
	curr_frame_no = 0;

	_total_track_age = 0;
	_total_tracks_count = 0;
	_total_track_length = 0;

	tracker.max_track_age = std::numeric_limits<long>::min();

	prev_gc.clear();

	prev_gc_frame_indexes.clear();
}

size_t CV_Tracking_Data::get_tracks_count()
{
	return prev_gc.size();
}

const cv::Point & CV_Tracking_Data::get_track(size_t i)
{
	return prev_gc[i];
}

void CV_Tracking_Data::next_frame()
{
	curr_frame_no++;
}

void CV_Tracking_Data::add_track(const cv::Point &gc)
{
	prev_gc.push_back(gc);

	prev_gc_frame_indexes.push_back(curr_frame_no);
}

void CV_Tracking_Data::remove_unused()
{
	size_t count = prev_gc.size();

	for (size_t i = 0; i < count; i++)
	{
		auto trigger_point = prev_gc[i];

		int64 age = curr_frame_no - prev_gc_frame_indexes[i];

		const int MAX_FRAMES_WITHOUT_OWNER = 20;

		if (age >= MAX_FRAMES_WITHOUT_OWNER) // lifetime of track without owner
		{
			remove_track_at(i);
		}

		count = prev_gc.size();
	}
}

bool CV_Tracking_Data::find_prev(const int MAX_ALLOWED_DISTANCE, const cv::Point &curr_gc, cv::Point &res)
{
	int prev_gc_index = -1;

	double distance_to_nearest = std::numeric_limits<double>::max();

	for (int i = 0; i < prev_gc.size(); i++)
	{
		if (prev_gc_frame_indexes[i] > curr_frame_no)
		{
			continue;
		}

		double distance = CV_Utils::get_distance(prev_gc[i], curr_gc);

		if (distance > MAX_ALLOWED_DISTANCE)
		{
			continue;
		}

		if (distance < distance_to_nearest)
		{
			distance_to_nearest = distance;

			prev_gc_index = i;
		}
	}

	if (prev_gc_index != -1) // prev. location found, just paint track
	{
		// если точка хоть раз вішла за нижнюю линию, то пред. позиция удаляется

		// такой критерий удаления ненадежній: 
		// 1) новая точка может появиться за зоной срабатівания через неопределенное время - просто йолке так захочется
		// 2) точка может запрігнуть назад, а следа нет уже - как предотвратить срабатівание? опять критерии на критерии?
		// короче доверяю только сборщику мусора!

		//if (curr_gc.y > max_line_y)
		//{
		//	remove_track_at(prev_gc_index);

		//	return false; // it is removed, so no need to paint it
		//}

		int64 age = curr_frame_no - prev_gc_frame_indexes[prev_gc_index];

		if (age > tracker.max_track_age) // єто не сборщик мусора! єто просто статистика!
		{
			tracker.max_track_age = (int)age;
		}

		_total_track_length += distance_to_nearest;
		_total_track_age += age;
		_total_tracks_count++;

		tracker.avg_track_age = (int)((double)_total_track_age / _total_tracks_count);
		tracker.avg_track_length = (int)((double)_total_track_length / _total_tracks_count);

		res = prev_gc[prev_gc_index];

		prev_gc[prev_gc_index] = curr_gc; // copy current gc to prev. one

		prev_gc_frame_indexes[prev_gc_index] = curr_frame_no; // the track found! prevent removal by GC

		return true; // the track found!
	}

	return false;
}
