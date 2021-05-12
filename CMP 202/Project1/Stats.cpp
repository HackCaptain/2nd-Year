#include "Stats.h"
#include <vector>

#include <iostream>
using std::cout;
using std::endl;

Stats::Stats()			// Constructor and destructors for class
{
}

Stats::~Stats()
{
}

void Stats::setstats(int slices_, int nospixels_)			// Setter for setting nos slices and the total amount of pixels
{
	slices = slices_;
	all_pixels = nospixels_;
}

void Stats::add(int nospixels)				// Setter for updating the number of pixels completed by the application
{
	pixelcounter = pixelcounter + nospixels;
}

void Stats::addtime(int time_, int id)		//  Add time adds to the total counter of time taken, plus, 
{											// It adds a time taken in the time vector, as well as recording the id of the thread
	time_taken = time_taken + time_;		// That completed that particular time / slice
	times.push_back(time_);
	threadid.push_back(id);
}

void Stats::showtimes()					// Prints out all contents of the vectors
{
	for (int v = 0; v < threadid.size(); v++) {
		cout << " - Thread " << threadid[v] << " , Time taken was " << times[v] << " ms " << endl;
	}
}

int Stats::percentage()					// Returns an integer value of the double percent, 
{										// I done this because it looks tidier.
	percent = (double(pixelcounter) / double(all_pixels) * 100);
	return percent;
}

int Stats::timetaken()			// Returns total time taken to complete the mandelbrot
{
	return time_taken;
}

int Stats::total()				// Returns the total number of completed pixels
{
	return pixelcounter;
}

void Stats::reset()				// Resets all vectors and variables, used when user makes another mandelbrot
{
	percent = 0.0;
	pixelcounter = 0;
	time_taken = 0;
	times.clear();
	threadid.clear();
}
