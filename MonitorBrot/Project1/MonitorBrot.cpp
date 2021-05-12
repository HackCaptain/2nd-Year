/* Peter Captains Parallelised Multithreaded Multicolour Mandelbrot Maker
 Peter Captain 1800326@uad.ac.uk

 Original Mandelbrot code by :
 Adam Sampson <a.sampson@abertay.ac.uk>

 This Mandelbrot Calculator can quickly produce a wide variety of Mandelbrot Images of varying shapes and colours.
 By default it uses 12 threads parallelisation to compute the set, this is because it was compiled with Ryzen 3600, 
 6 cores, 12 threads. It also concurrently runs a monitoring thread as it computes the set, providing an update as 
 to the percentage of completion of the set. This is activated by a control variable to and locked via a mutex
 to protect the contents of the object from data races.

 This code has the possibility to execute on as many threads as you want. It should scale accordingly.

 Please, have some fun, my personal favourite combination is to go for :
 the detailed view, 11 - "unicorn vomit", 10 - black, no switching, either 6 or 7 colours 
 */

#pragma region includes
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <complex>
#include <fstream>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

#include "Stats.h"


// Import things we need from the standard library
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::complex;
using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::thread;
using std::lock;
using std::mutex;
using std::condition_variable;
using std::unique_lock;
using std::vector;
#pragma endregion

#pragma region image data
// Define the alias "the_clock" for the clock type we're going to use.
typedef std::chrono::steady_clock the_clock;


// The size of the image to generate.
//const int WIDTH = 1920;
//const int HEIGHT = 1200;

const int SCALE = 1;				// For experimental purposes this enlarges the image, results in more detail. At the massive expense of time.

const int WIDTH = SCALE * 1920;
const int HEIGHT = SCALE * 1200;

// The number of times to iterate before we assume that a point isn't in the
// Mandelbrot set.
// (You may need to turn this up if you zoom further into the set.)
const int MAX_ITERATIONS = 1000;

// The image data.
// Each pixel is represented as 0xRRGGBB.
uint32_t image[HEIGHT][WIDTH];

condition_variable mandelbrot_cv;			//Setting up mutex, condition variable and boolean for monitor pattern.
mutex mandelbrot_mutex;
bool mandelbrot_ready = false;
#pragma endregion 

#pragma region Colour Information
int colours[10] =			{ 0xFF0000, 0xFF7F00, 0xFFFF00, 0x00FF00, 0x0000FF, 0x4F0082, 0xFF00FF, 0x654321, 0xFFFFFF, 0x000000 };		// rgb values for Red, Orange, Yellow... Brown, White, Black;
int opposite_colours[10] =	{ 0x00FF00, 0x0000FF, 0x4F0082, 0xFF0000, 0xFF7F00, 0xFFFF00, 0x654321, 0xFF00FF, 0x000000, 0xFFFFFF };		// rgb values for contrasting colours. Essentially the rainbow jumbled up

int selection[2] = { 0,0 };				// This is how the user can select their colours
bool flag = false;						// This flag is used to indicate if the user wants to alternate their colours per thread. Was a debugging technique but it looks cool.

			// The rest of this is for the rainbow... I like to call this selection "unicorn vomit"

int colour_shift = 0;					// This is to shift the colourset when rainbow is on to avoid going out of scope
bool rainbow_mode_hit = false;			// This is the flag that indicates the user wants to make the mandelbrot hits multicoloured
bool rainbow_mode_miss = false;			// Or the misses multicoloured. Uses opposite colours otherwise mandelbrot would just be stripes.
int rainbow_length = 6;					// This indicates how many colours the rainbow shoes. Can be as big as the list of colours. And the list could be theoretically 16 million colours. By default its 6.
bool field_lines = false;

vector<int> rainbow_vector;				// This stores the users custom pattern values
bool custom_rainbow = false;

#pragma endregion

Stats MandelStats;						// This is the object that holds relevant statistical data produced during computation of the mandelbrot

int parameters() {							// Parameters take in the mandelbrot parameters. This is the user interface with the program.

	field_lines = false;					// Reset Values for when the user decides to make another mandelbrot
	rainbow_length = 6;
	selection[0] = 0;
	selection[1] = 0;
	flag = false;
	rainbow_vector.clear();
	

	int option;								// Declare local Variables, option is for selecting colour
	int viewselection = 0;					// View Selection allows the user to select where in the mandelbrot they want to look at
	char indication = ' ';					// Allows the user to select if they want the colours to alternate between threads.

	cout << "Would you like the full set or a bit of detail? \n";									// Ask the user what set they want to look at.
	cout << " 1 - The Whole Set \n 2 - View of interesting detail \n 3 - Demonstration \n	";		// 3 is the default option that the mandelbrot would calculate anyway	
	
	while (viewselection < 1 || viewselection > 3) {			// User input validation makes certain the selection is within the parameters
		cin >> viewselection;
	}
	if (viewselection == 3) {			// User has selected a predefined mode so the rest of this function is not required
		field_lines = false;			// No fancy lines
		rainbow_length = 6;				// If there is a slip up (there won't) then the rainbow may as well be standard
		selection[0] = 9;				// Make these one less than the colour labels		Black
		selection[1] = 8;				// Unless you really like bright green.				White
		flag = true;					// Want to show the threads are being utilised
		return viewselection;			// Return 3
	}
		
	cout << " Please select 2 colours from the list below, by entering their corresponding number : \n";												// This is the menu interface for selecting colours
	cout << " 1 - Red \n 2 - Orange \n 3 - Yellow \n 4 - Green \n 5 - Blue \n 6 - Purple \n 7 - Pink \n 8 - Brown \n 9 - White \n 10 - Black \n" ;		// List all colours
	cout << " 11 - Or, for the hell of it...  Unicorn vomit? \n \n";																					// And le spezial, rainbow mode, aka unicorn vomit

	for (int p = 0; p <= 1; p++) {																	// Ask the user for two numbers, one after the other

		if (p == 0) { cout << " Please enter your colour for mandelbrot pixels \n	"; }				// Asks the user what colour they want Mandelbrot hits to be
		else { cout << " And your other choice of colour for pixels outside the set : \n	"; }		// And after asking that they want misses to be

		cin >> option;											// Take in user input

		switch (option) {										// Switch statement does selection to determine what setting the user wanted
		case 1:	selection[p] = 0; break;						// Red
		case 2:	selection[p] = 1; break;						// Orange
		case 3:	selection[p] = 2; break;						// Yellow
		case 4:	selection[p] = 3; break;						// Green
		case 5:	selection[p] = 4; break;						// Blue
		case 6:	selection[p] = 5; break;						// Purple
		case 7:	selection[p] = 6; break;						// Pink
		case 8:	selection[p] = 7; break;						// Brown
		case 9:	selection[p] = 8; break;						// White
		case 10:selection[p] = 9; break;						// Black
		case 11:												// Rainbow
			
		if (p == 0) { rainbow_mode_hit = true; }					// For the first rainbow selection
			else {rainbow_mode_miss = true;}						// For alternating rainbow section
			cout << "\n\ No unicorns will be harmed!!! \n";			// For legal purposes. 
			break;						
		}															// End of switch statement
	}

	cout << " Do you want field lines, these operate a bit like rainbow lines, but will overwrite anything outside the shape! y/n? \n	";
	cin >> indication;									// Take in user preference for field lines
	if (indication == 'Y' || indication == 'y') {		// Input validation ensures the user has selected an option
		field_lines = true;
	}
	else {
		field_lines = false;
	}

	if (rainbow_mode_hit || rainbow_mode_hit == true || field_lines == true) {																								// If the user chose multicoloured mode
		cout << " You chose a Multicoloured Option ! Please select how many colours you want present, \n e.g, for every colour up to blue enter 5 \n";						// This statement asks how 
		cout << " Or if you're feeling creative and would like to specify a colour set enter 11 \n	";

		cin >> option;

		if (option == 11) {														// Input validation ensures the user has selected an option
			cout << " Custom Colour List\n Please enter your colour selection below, when you're done enter 11 \n";		// As there are 10 usable colours, 11 can be used to stop looping
			custom_rainbow = true;
			option = 0;
			while (option != 11) {
				cout << "	";	
				cin >> option;
				rainbow_vector.push_back(option - 1);							// Using a vector allows for an infinite number of selections to used
			}
			rainbow_length = rainbow_vector.size() -1;							// It also allows the user to specify as many numbers as they want
		}
		else {
			option = rainbow_length;
			for (int i = 0; i < rainbow_length + 1; i++) {
				rainbow_vector.push_back(i);
			}
		}																																			// many colours are in it
	}

	cout << " Please indicate if you would like colours to alternate for each thread? Y/N \n";								// Ask the user if they want the colours to alternate between threads

	if (rainbow_mode_hit == true || rainbow_mode_miss == true) {															// Obligatory warning opposite rainbow colours can look funky 
		cout << " Alternating colours in threads can be funky! \n Be careful as output mightnot be as expected! \n	";		// "It's not a bug, it's a feature!"
	}

	cin >> indication;									// Take in user preference for colour switching
	if (indication == 'Y' || indication == 'y') {		// Input validation ensures the user has selected an option
		flag = true;
	}
	else {
		flag = false;
	}

	return viewselection;					// This will be used to calculate what co - ordinates the mandelbrot should be
}

bool colour_switch(bool current_flag) {				// This is the function that switches the colours between threads

	if (flag == true) {								// Looks to see if the flag is up if so...
		if (current_flag == true) {					// Switches from T > F
			current_flag = false;				
		}
		else if (current_flag == false) {			// And vice versa, 
			current_flag = true;					// Switches from F > T
		}
	}
	return current_flag;							// If the flag is not up, return existing value

}

int paint_bucket(bool in_the_set, int col_choice, int id, int iterations) {					// The paint bucket function is what decides what each pixel colour should be.

	// Adapted from Adam Sampsons original Mandelbrot code <a.sampson@abertay.ac.uk>
	// Essentially takes in a value indicating the pixel is within the mandelbrot, and picks a colour

	colour_shift = id % rainbow_length;						// This ensures the rainbow doesnt show unwanted colours or nothing at all

	if (in_the_set == true) {								// If point is in the set
		// z didn't escape from the circle.									
		// This point is in the Mandelbrot set.
		if (col_choice == 1) {											// If the colour should be switched for each thread, by default this is true		
			if (rainbow_mode_hit == true) {								// If unicorn vomit mode is active
				if (custom_rainbow == true) {							// If user specified set should be used
					return colours[rainbow_vector.at(colour_shift)];	// Return a user specified colour
				}														// If user hasnt selected a certain colour set
				return colours[colour_shift];							// then return the appropriate colour
			}
			return colours[selection[0]];								// Return the user specified switch colour
		}
		else {
			return colours[selection[1]];								// Return the user specified colour
		}
	}

	// This point is not in the Mandelbrot set.
	// A couple things will happen here, the lines may be added,
	// or the inverse rainbow will be added, 
	// or the colours will be swapped for each thread.

	else if (field_lines == true) {												// Field line calculator checks to see if the user wants field lines
		for (int p = 1; p <= 1000; p++) {										// If so, then no matter the iterations it will print
			if (p == iterations) {												// out coloured lines where the iterations are getting closer to the
				if (custom_rainbow == true) {									// edge of the Mandelbrot
					return colours[rainbow_vector.at(p % rainbow_length)];		// Return a specified colour
				}
				return colours[p % rainbow_length];								// Return a default colour
			}
		}
	}

	if (col_choice == 1) {												// If the colour should be switched for each thread, by default this is true	
		if (rainbow_mode_miss == true) {								// If unicorn vomit mode is active
			if (custom_rainbow == true) {								// If user specified set should be used
				return colours[rainbow_vector.at(colour_shift)];		// Return a user specified colour
			}															// If user hasnt selected a certain colour set
			return opposite_colours[colour_shift];						// then return the appropriate colour
		}
		return colours[selection[1]];									// Return the user specified switch colour
	}
	else {
		return colours[selection[0]];									// Return the user specified colour
	}
}

void notify() {

	unique_lock<mutex> lock(mandelbrot_mutex);				// This locks the mutex, allowing the result to be recorded	
		
	while (!mandelbrot_ready) {																				// While the Mandelbrot is being computed 
		cout << " -UPDATE - Mandelbrot is " << MandelStats.percentage() << " % complete \n";				// This will access a function to calculate the percentage, then show it
		mandelbrot_cv.wait(lock);																			// "spurious wake - up" protection
		if (MandelStats.percentage() > 95) {																// If the mandelbrot is completed
			cout << " -UPDATE - Mandelbrot is 100% complete! \n";
			cout << "\n Mandelbrot threads completed in :\n\n";												// Send message to the user it is as well as
			MandelStats.showtimes();																		// showing the times of each thread
			cout << "\n Estimated time for single thread was " << MandelStats.timetaken() << "ms \n\n";
			mandelbrot_ready = true;																		// Break out of the while loop, ending thread
		}
	}
}

void write_tga(const char* filename)
{

	// Write the image to a TGA file with the given name.
	// Format specification: http://www.gamers.org/dEngine/quake3/TGA.txt

	ofstream outfile(filename, ofstream::binary);

	uint8_t header[18] = {
		0, // no image ID
		0, // no colour map
		2, // uncompressed 24-bit image
		0, 0, 0, 0, 0, // empty colour map specification
		0, 0, // X origin
		0, 0, // Y origin
		WIDTH & 0xFF, (WIDTH >> 8) & 0xFF, // width
		HEIGHT & 0xFF, (HEIGHT >> 8) & 0xFF, // height
		24, // bits per pixel
		0, // image descriptor
	};
	outfile.write((const char*)header, 18);

	for (int y = 0; y < HEIGHT; ++y)
	{
		for (int x = 0; x < WIDTH; ++x)
		{
			uint8_t pixel[3] = {
				image[y][x] & 0xFF, // blue channel
				(image[y][x] >> 8) & 0xFF, // green channel
				(image[y][x] >> 16) & 0xFF, // red channel
			};
			outfile.write((const char*)pixel, 3);
		}
	}

	outfile.close();
	if (!outfile)
	{
		// An error has occurred at some point since we opened the file.
		cout << " Error writing to " << filename << endl;
		exit(1);
	}
		cout << " File " << filename << " written succesfully!";
}

class Threadbrot			// This is the class where the mandelbrot is calculated
{
public:
	void compute_mandelbrot(double left, double right, double top, double bottom, int starty, int endy, bool colour, int id)			// Recieve arguments neccasary for calculation
	{
		int completed_pixels = 0;							// Allows each thread to track how many pixels it has completed

		the_clock::time_point start = the_clock::now();		// Begin timing the individual thread

		for (int y = starty; y < endy; ++y)					// For each vertical value in the slice 
		{
			for (int x = 0; x < WIDTH; ++x)					// For each horizontal value in the slice
			{

				// Work out the point in the complex plane that
				// corresponds to this pixel in the output image.
				complex<double> c(left + (x * (right - left) / WIDTH),
					top + (y * (bottom - top) / HEIGHT));

				// Start off z at (0, 0).
				complex<double> z(0.0, 0.0);

				// Iterate z = z^2 + c until z moves more than 2 units
				// away from (0, 0), or we've iterated too many times.
				int iterations = 0;
				while (abs(z) < 2.0 && iterations < MAX_ITERATIONS)
				{
					z = (z * z) + c;

					++iterations;
				}
				//cout << " Iterations = " << iterations << endl;
				if (iterations == MAX_ITERATIONS) { image[y][x] = paint_bucket(true, colour, id, iterations); }			// In the set
				else { image[y][x] = paint_bucket(false, colour, id, iterations); }										// Not in the set
				++completed_pixels;
			}
		}

		// Stop the clock and count time ellapsed
		the_clock::time_point end = the_clock::now();
		auto time_taken = duration_cast<milliseconds>(end - start).count();

		/*DEBUG CODE*/ //cout << endl << " Thread " << id << ", Section calculated in " << time_taken << " ms." << endl;

		mandelbrot_mutex.lock();							// Locks the mutex that stops collisions when another thread finishes a slice
		MandelStats.addtime(time_taken, id);				// Statisitics are then updated, in this case, the time taken and ID of the thread
		MandelStats.add(completed_pixels);					// This is used for percentage completion, by comparison to the total amount of pixels in the image
		mandelbrot_mutex.unlock();							// Unlock the mutex so other threads can safely write their stats
		mandelbrot_ready = false;							// The mandelbrot should not be completed by now
		mandelbrot_cv.notify_one();							// Allow the consumer to decide this, so wake it up.
	}
};

int main(int argc, char* argv[])
{
	char decision = ' ';				// Declare local variables, this being the users decision to keep the program running
	double left = 0.0;					// The left co - ordinate
	double right = 0.0;					// The right co - ordinate
	double top = 0.0;					// The top co - ordinate
	double bottom = 0.0;				// The bottom co - ordinate
	int choice = 0;						// The selection for  co - ordinates

	while (decision != 'n' && decision !=  'N') {							// While user wants to make Mandelbrots

		cout << " \n \n P.Captains Multithreaded Multicolour Mandelbrot Maker, \n \n";

		choice = parameters();				// Calls parameter function to determine parameters for the set

		if (choice == 1) {					// If the user chose the whole set 
			left = -2.0;
			right = 1.0;
			top = 1.125;
			bottom = -1.125;
		}
		else if (choice == 2) {				// If the user chose the close up view
			left = -0.751085;
			right = -0.734975;
			top = 0.118378;
			bottom = 0.134488;
		}
		else if (choice == 3) {
			left = -0.751085;
			right = -0.734975;
			top = 0.118378;
			bottom = 0.134488;
		}

		thread notifythread = thread(notify);				// Begin the consumer thread that show the mandelbrot progress
		Threadbrot* MandelbrotPtr = new Threadbrot();		// Create a pointer to a new thread object, this is used to parallelise the application later on

		// ----------------------

		const int nosslices = 12;							// Number of Threads the Mandelbrot will be calculated on, can be as many as needed, but for demonstration is 12, see intro

		//-----------------------

		double starty = 0;									// The start Y value for a slice
		double slice = HEIGHT / nosslices;					// The size of a slice to be executed by a thread
		double endy = slice;								// The end Y value for a slice
		bool colour_flag = 1;								// Colours flag
		int ap = HEIGHT * WIDTH;							// Sum of all pixels

		MandelStats.setstats(nosslices, ap);				// Access stats and populate with data

		std::thread mandelthread[nosslices];				// Start as many threads as slices

		cout << " Please wait..." << endl;					// Asks the user to be patient, but isnt strictly necessary 

		// Start timing
		the_clock::time_point startmain = the_clock::now();

		for (int id = 1; id < nosslices + 1; ++id) {		// For each slice 

			colour_flag = colour_switch(colour_flag);		// Switch the colours, if necessary 

			//DEBUG CODE cout << " Computing Slice " << slicenumber << ", starting Y value is " << starty << " and ending Y value is " << endy << ", Colour is " << colour << endl;

			mandelthread[id - 1] = std::thread(&Threadbrot::compute_mandelbrot, MandelbrotPtr, left, right, top, bottom, starty, endy, colour_flag, id); 

			starty = slice * id;							// Next start y value is start y is id * a slice
			endy = slice * id + slice;						// Next end y value is start y is id * a slice
		}

		for (int id = 1; id < nosslices + 1; ++id) {		// Join each thread when available
			if (mandelthread[id - 1].joinable())			
				mandelthread[id - 1].join();
		}

		// Compute the difference between the two times in milliseconds
		the_clock::time_point endmain = the_clock::now();
		auto time_taken = duration_cast<milliseconds>(endmain - startmain).count();

		notifythread.join();			// Join the consumer thread

		write_tga("Monitorbrot output.tga");		// Write the mandelbrot out to a file

		cout << endl << "\n Computing the whole Mandelbrot set took " << time_taken << " ms." << endl;			// Show user the times taken
		cout << " Do you want to make anther mandelbrot set? Enter N or n to exit." << endl;					// Ask user if they want to make another set
		cin >> decision;

		mandelbrot_ready = false;
		MandelStats.reset();

	}
	return 0;			// Assuming the user entered "n" or "N", then the program will exit.
}