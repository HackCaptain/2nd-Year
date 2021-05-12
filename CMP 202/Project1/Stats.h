#ifndef STATS_H
#define STATS_H

#include <vector>

/* Stats holds the relevant statistics about completion of the mandelbrot,
takes into account the number of slices and the amount of pixels in the image*/

class Stats	
{
public:

	Stats();			// Default constructor / destructor set - up
	~Stats();

	void add(int nospixels);						// Adds a number of pixels to the running total of pixels
	void addtime(int time_, int id);				// Adds time to time taken
	void showtimes();								// Shows time each thread took
	void setstats(int slices_, int nospixels_);		// Setter for stats
	int percentage();								// Calculates the percentage of completion, (completed/total *100) returns an int, tidier.
	int timetaken();								// Returns the time taken to complete the mandelbrot
	int total();									// Returns the total number of completed pixels
	void reset();									// Resets all variables local to the class

private:

	std::vector<int> times;			// Vector holds all the threads times and ID as its easily changeable when the number of threads changes
	std::vector<int> threadid;		

	int all_pixels = 0;				// Count of all pixels in the image
	double percent = 0.0;			// Double allows the percentage to be calculated
	int pixelcounter = 0;			// Counter for the completed number of pixels
	int slices = 0;					// Number of slices should be exact same as number of threads
	int time_taken = 0;				// Counter for total time taken

};
#endif