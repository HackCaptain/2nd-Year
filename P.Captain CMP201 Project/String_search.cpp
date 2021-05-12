#include <iostream>                            // c++ standard libraries in
#include <cstdio>
#include <cassert>

#include <vector>			// Co-Star Data structure of the show
#include <string>			// Star data structure of the show

#include <chrono>			// The clock standard library

#include "utils.h"          // Including all the utilities found in utils.cpp (from lab 7)

using std::chrono::duration_cast;
using std::chrono::nanoseconds;							 // Setting up the clock to measure time in an appropriate format.

typedef std::chrono::steady_clock the_clock;             // Synchronises the clock

std::vector<int> rk_list;               // Using vectors as;	1: An undefined amount of short data items will be stored, into a list. 2: A list wasnt used as c++ recommends vector
std::vector<int> bm_list;				// 3: List does not provide readouts in constant time, which means searching for repetitive chars, like char(97) "a" might mean more than 100000 itterations of grabbing head and tail values.

using namespace std;

void print_results(string pat, string text) {                          
	
	// Simple itterator steps through vectors to show all the positions where a pattern has matched. 
	// Assuming both algorithms are functioning, then the results should be identical. (except time taken)
	// This function also shows the context surrounding the pattern, by grabbing the text before and after the pattern

	std::vector<int> printv;						// Printing vector, not to be used for anything else

	for (int i = 0; i < 2; i++) {					// Loop once for Boyer-Moore results, then again for Rabin-Karp
		if (i == 0) {
			for (int j = 0; j < bm_list.size(); j++) {					// Fill printv with the vector storing data from Boyer Moore result vector
				printv.push_back(bm_list[j]);							// Iterate through the vectors, transfering values using push back
			}
			if (printv.size() == 0) {																	// If there is no results, say so 
				cout << "\n Boyer-Moore Algorithm found no instances of pattern: " << pat << endl;
			}
			else {														// Else, begin outputing results
				cout << " \n  Boyer-Moore found: \n " << endl;
			}

		}
		else if (i == 1) {
			for (int j = 0; j < rk_list.size(); j++) {					// Fill printv with the vector storing data from Rabin Karp result vector
				printv.push_back(rk_list[j]);							// Iterate through the vectors, transfering values using push back
			}
			if (printv.size() == 0) {																	// If there is no results, say so
				cout << "\n Rabin-Karp Algorithm found no instances of pattern: " << pat << endl;
			}
			else {																						// Else, begin outputing results
				cout << " \n  Rabin-Karp found: \n " << endl;
			}
		}

		for (size_t i = 0; i < printv.size(); i++) {															// Iterate through the printv vector and show positions where the pattern matched
			cout << i << "- Instance of '" << pat << "' Found at position : " << printv[i] + 1 << endl;		    

			string before;                                                      // This string holds the portion of text before the pattern
			if (printv[i] - 20 < 0) {											// If the text being searched is short, the set the preceding text to a sensible length
				before = text.substr((text.length() - printv[i]));
			}
			else {																// Otherwise, gather the first 20 elements before the pattern
				before = text.substr((printv[i] - 20), 20);
			}
			
			string after;                                                       // This string holds the portion of text after the pattern
			if (printv[i] + pat.length() > text.length()) {					    // If the text being searched is short, the set the following text to a sensible length
				after = text.substr((text.length() - printv[i]));
			}
			else {																// Otherwise, gather the next 20 elements after the pattern
				after = text.substr((printv[i] + pat.length()), 20);
			}

			cout << "	--->" << before << ">" << pat << "<" << after << endl;		// Display the first string, "highlighted" pattern, then the following text
		}
	}
	return;
}

void rabinKarp(string text, string pat) {							
	
	// The Rabin Karp algorithm takes a pattern, and a smaple from the text, hashes and compares the result. Time complexity is O(n*m)
	// The actual hashing process itself has a time complexity O(1) ( - this can change with complexity)

		int string_hash = 0;					// string_hash hold the hashed value of the string
		int pattern_hash = 0;                   // pattern_hash hold the hashed value of the pattern
		const int alpha = 256;                  // 256 letters in char type (Latin-1 / ISO-8859-1) , however alpha(-bet) can be adjusted for alphabets of more
		const int prime = 101;                  // large prime number is needed to reduce hash collisions,as such, this number will not change
		int msb_mult = 1;                       // h = multiplier for MSB, preventing a hash (multiplication by or =) of 0
		int i, j;								// Iterators for loops

		cout << " \n\n RABIN KARP STRING SEARCH \n" << endl;

		the_clock::time_point start = the_clock::now();					// Start the clock, this is the start of the algorithm

		for (i = 0; i < pat.length() - 1; i++) {						// Value of msb_mult would be eqv to pow (alpha, pattern_length - 1 )
			msb_mult = (msb_mult * alpha) % prime;						// Multipl
		}																
	
		//DEBUG CODE //cout << " MSB mult is "  <<  msb_mult << endl;

		for (i = 0; i < pat.length(); i++) {							// calculating initial hash for string and pattern
			string_hash = (alpha * string_hash + text[i]) % prime;		// A modulus of a prime number prevents a hash collison, which in a hashed string
			pattern_hash = (alpha * pattern_hash + pat[i]) % prime;		// would be disastrous, as it could match with different text with the same hash value
		}

		//DEBUG CODE //cout << "Pattern Hash of '" << pat << "', is = " << pattern_hash << endl;

		for (i = 0; i <= text.length() - pat.length(); i++) {

			if (string_hash == pattern_hash) {						// If the hashes equate start comparing hash of string and pattern
				for (j = 0; j < pat.length(); j++)					// Iterates through sample and checks if values match
					if (text[i + j] != pat[j])						// Checking if hashed text from .txt is the same as the hash produced from the pattern
						break;										// If it isnt, break the loop, hash next sample of text and continue.

				if (j == pat.length()) {							// If the hashed value of the pattern = the hashed value of the text taken from the string
					rk_list.push_back(i);							// Add the position of the pattern to the linked list
				}
			}
																													// Rabin Karp prepares to hash next pattern sized piece of text, thus, 
			string_hash = (alpha * (string_hash - msb_mult * text[i]) + text[i + pat.length()]) % prime;			// this is reffered to as a "Rolling Hash Algorithm"
			if (string_hash < 0) {									
				string_hash = string_hash + prime;					// If the next hash item in the rolling hash is 0, then add the prime number 
			}
		}

		the_clock::time_point end = the_clock::now();					// Finish the clock 

		auto time_taken = duration_cast<nanoseconds>(end - start).count();																							// Work out time taken and display the result
		cout << " Algorithm took " << time_taken << "  nanoseconds to find " << rk_list.size() << " occurences of '" << pat << "' present in the .txt \n";

		rk_list.shrink_to_fit();		// Frees up spare memory by "locking" the size of the vector, mainly for effeciency

		return;
}

void boyerMoore(const string& pat, const string& text) {		
	
	// The Boyer-Moore algorithm creates a lookup matrix, which allows it to skip through the text looking for a match. Best case time complexity is, where the matrix eleminates any non-matches immediately O(n/m)
	// However this can get worse, especially for example where the first "buckets" of a pattern are identical, but the string itself is not. This is O(n*m)

	cout << "\n\n BOYER MOORE STRING SEARCH \n" << endl;

	the_clock::time_point start = the_clock::now();					// Start the clock, this is the start of the algorithm


	int skip[256];				// Lookup matrix, makes program much quicker by checking if the specified char is in the file.
	int j;						// Initialise itterator for the for loops

	for (int i = 0; i < 256; ++i) {
		skip[i] = pat.size();							// Puts the pattern into the skip value
	}

	for (int i = 0; i < pat.size(); ++i) {				// Checks if the characters are in the pattern
		skip[int(pat[i])] = (pat.size() - 1) - i;
	}

	//DEBUG CODE cout << "Text length is " << text_len << endl;

	for (int i = 0; i < text.size() - pat.size(); ++i) {        // Beginning loop to find the occurences.

		//DEBUG CODE show_context(text, i);

		int s = skip[int(text[i + pat.size() - 1])];			// s is the same  

		if (s != 0) {                               // Checks if the patterns last char is in this text extract
			i += s - 1;							    // Skip forwards.
			continue;
		}

		for (j = 0; j < pat.size(); j++) {          // If it is, iterate through to match the pattern using brute force, if its not, break;    
			if (text[i + j] != pat[j]) {			// If the text doesnt contain the chars...
				break;								// Break the statement, causing it to skip this sample of the text.
			}
		}


		if (j == pat.size()) {                      // Assuming break has not been called and the character is present, then the final length of the pattern and text must be the same
			bm_list.push_back(i);					// Meaning the values are identical. If so, add position to vector.
			// DEBUG CODE cout << " Matched at " << i + 1 << " This is occurence " << counter << endl;
		}
	}


	the_clock::time_point end = the_clock::now();

	auto time_taken = duration_cast<nanoseconds>(end - start).count();																							// Work out time taken and display the result
	cout << " Algorithm took " << time_taken << " nanoseconds to find " << bm_list.size() << " occurences of '" << pat << "' present in the .txt \n";

	bm_list.shrink_to_fit();		// Frees up spare memory by "locking" the size of the vector


	return;			// Returns found instances
}


int main(){

	// String searching algorithm comparison by P.Captain
	// Compares the performance times of the Boyer Moore and Rabin Karp algorithms.



	string pat;													        // Pattern is stored as string 
	string filename = "fakelogmonth.txt";						        // This the name of the file being accessed. ( This is mainly to make it easier to change directory if need be, or create a UI)

	cout << "What pattern do you want to search for? : " << endl;       // Ask user for pattern they want found   
	std::getline(std::cin, pat);                                        // Standard library getline is used to search for entire sentences/paragraphs

	cout << "What file do you want to search: " << endl;                // Ask user for file they want to search   
	std::getline(std::cin, filename);                                   // Standard library getline is used to search for entire sentences/paragraphs

	//string filename = "fakelog.txt";			// Smallish text file, 700kb
	//string filename = "lorem.txt";			// Larger, but variation is greater, lorem ipsum...   9000kb 
	//string filename = "fakelogmonth.txt";		// Largest file, essentially the top but over a month where every second a message is sent. 25000kb

	string text;											 	  // This is how the text that will be searched is stored. Its also the main data structure dealt with. 


	load_file(filename, text);									  // Uses the loadfile function from utils.cpp. This was from the lab.

	// BAD IDEA, BUT HEY.. cout << text;

	boyerMoore(pat, text);			// See corresponding functions
	rabinKarp(text, pat);

	char a = ' ';                   // Makes the UI less clutered, if desired. Printing through a vector can produce unexpected lists
	cout << "\n\n Do you wish to see results? (takes longer as found instances reaches infinity... Vectors) y/Y " << endl;
	cin >> a;

	if (a == 'Y' || a == 'y') {
		print_results(pat, text);
	}

	cout << "\n Search complete" << endl;  // End of Program, ask user for conformation.
	cin.get();
}