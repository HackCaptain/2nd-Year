To use this program simply open the executable and follow the instructions on the interface.

If there are programs with external libraries please retarget the solution, no external libraries other than those 
included standard with visual studios version of c++ were used.

The program should be compiled in x64. Using x86 will work but the program will run significantly slower.

If you wish to change the number of slices that will be executed then please change the variable "slices" - 
found on line 438. It is recomended not choose more than 12 threads. However, you may have a cpu with
more threads in which case this isnt an issue. Alternatively, set this to 1 and the program will 
behave as if its single threaded.

Any issues that that might appear can be addressed by contacting me at 1800326@uad.ac.uk

For your benefit, I will describe what each part of the user interface will do to the Mandelbrot set

Step 1:

	The first option you will get is what type of set you want to create.

	Option 1 - This option allows you to choose the whole set to customize. 
	Option 2 - This option allows you to choose the zoomed in set to customize.
	Option 3 - This option will create a standard black and white mandelbrot set.

Step 2:

	If you chose Option 1 or 2 you will then be asked to select 2 colours from the list.

	Options 1 - 10 - Standard colours. 
	Option 11 - Rainbow mode, affectionately known as "unicorn vomit"
			
		This option will then cause an additional menu to appear after step 3.

Step 3: 

	Field lines allows you to specify if you want the set to display the field lines.

	Option "Y" - Then a special Menu will appear
	Option "N" - Then you will be sent to step 4.

Special Step:

	This is the interface for choosing more than 2 colours to colour parts of the set. 
	It will appear if the user has selected Option 11 at any point during picking colours 
	or if they chose to add field lines to the set

	You will be notified you have selected a multicolour option and will be asked 
	what colour set you want to use:

	Entering the number of a colour, as seen in step 2, simply cuts the colour array after
	the number and uses that as the default "rainbow".

	Entering 11 will pop up the "custom colour list" it allows the user to populate a 
	vector with as many colours as they enter, until they enter the number 11 
	which stops the intake.
	1

Step 4:

	This is the interface that allows the user to indicate if they want the colours
	to reverse per thread. E.g For every even number thread ID, If the colours are 
	black for pixels inside the set and white for those outside, then the inverse
	is true for every odd numbered ID.

	If the user also chose a rainbow mode a warning will pop up showing them
	that alternating colours may effect the final output. If the user wishes to create a 
	rainbow with contrasting colours they need only enter "11" twice at the colour set up
	and select a number of a colour, normally 6 or 7, during the special step.

	Choosing "Y" - Mandelbrot is sliced into chosen colour segments
	Choosing "N" - Mandelbrot is not sliced according to each thread and produces
			a seamless image.

Once the user is done the user interface will then display the progress of the set, and once done
they will be notified how long each slice took ( this is sorted by when each thread joined) , the 
estimated time for a single threaded application, as well as the name of the file that was 
created. If this failed an error will pop up.

Step 5:

	The user will be asked if they wish to make another set

	Choosing "Y" - Resets the program to make another set
	Choosing "N" - Allows the user to exit.

