<h1>Overview</h1>

A simple Pacman game running on a 32x32 RGB LED Matrix.

The LED Matrix is the <a href="https://www.sparkfun.com/products/12584">RGB LED Panel from sparkfun</a> and is driven fast enough to be able to display 7-bits of color information per color (R, G, and B).

The microcontroller used to drive the LED matrix is a <a href="http://www.ti.com/tool/ek-tm4c123gxl">TI Tiva C launchpad board</a> containing a TM4C123GH6PM chip. This chip can run up to 80MHz which is enough to drive the matrix as well as perform game logic.

Currently, the Pacman character is controlled over the UART. You can either connect it to your computer through a USB to UART converter, or by attaching a bluetooth wireless UART (like any common HC-05 module) and connect to your computer over bluetooth.
You move around using the "w", "a", "s", and "d" characters. "w" is up, "a" is left, "s" is down, and "d" is right. Your goal, as in the original pacman game, is to collect as many pellets as you can before a ghost catches you.

<h2>Binary Coded Modulation</h2>
Before understanding how the LED Matrix is being driven, you need to understand the concept of Binary Coded Modulation (BCM). Essentially, BCM is a technique used to dim certain LEDs on the matrix. A common approach to dimming LEDs is through Pulse Width Modulation (PWM). Unfortunately, the LED driver chips on this matrix only support a simple on/off for each LED. Since each "pixel" on the matrix actually contains three LEDs (red, green, and blue), by varying the brightness of those three LEDs you can achieve more than just the eight colors provided by only controlling three LEDs with no brightness control (black, white, red, green, blue, yellow, magenta, cyan).

Binary Coded Modulation works on the principle of turning each LED on and off so quickly that (to the human eye) it appears to have dimmed. Let's say we're going to implement 4-bit BCM (each pixel has 4-bits of color for each red, green, and blue LED). When displaying each pixel, you display the least significant bit for 1 "tick", the second bit for 2 "ticks", the third bit for 4 "ticks", and the last bit for 8 "ticks". If you want to display red at brightness 10 (out of 15 for 4-bit BCM), the red LED would be on for two ticks on the second bit and eight ticks on the fourth bit (in binary, ten is 0b1010 which means the LED will be turned on for the second and fourth bits). This equates to the LED being on for 10 of the 13 ticks, or for 77% of the time. This will dim the LED 23% compared to just driving it at full brightness.

I highly recommend BatSock's tutorial for understanding BCM better: <a href="http://www.batsocks.co.uk/readme/art_bcm_1.htm">http://www.batsocks.co.uk/readme/art_bcm_1.htm</a>

<h2>Understanding the LED Matrix</h2>
The following links will help you understand how the hardware and timing of the LED Matrix actually function:
<ul>
	<li><a href="http://www.rayslogic.com/propeller/Programming/AdafruitRGB/AdafruitRGB.htm">A detailed breakdown and explanation of the LED matrix hardware by Ray's Logic</a></li>
	<li><a href="http://www.penguintech.info/2014/teensy-3-1-16x32-rgb-led-panel/">Example code of how to drive the matrix with a teensy board</a></li>
	<li><a href="https://learn.adafruit.com/32x16-32x32-rgb-led-matrix/">Adafruit's tutorial on how to use the matrix.</a></li>
	<li><a href="https://learn.sparkfun.com/tutorials/rgb-panel-hookup-guide">Sparkfun's tutorial and hookup guide for the matrix</a> (note, I had to swap both of the green and blue pins as well as the OE and latch pins compared to the sparkfun pinout, your mileage may vary)</li>
</ul>

The basic algorithm for driving the matrix is as follows:
<ol>
	<li>Set OE (output enable, it's active low) to high, turning off the display. If you forget to do this, you'll have strange ghosting effects (because the matrix will be on while you're updating it).</li>
	<li>Set the Demux lines to display the current pair of rows. Check LedMatrix.c to figure out in what order you need to set these pins to. If you thought it would just be a sequential order, you'd be wrong. There's a fairly random order you need to set the demux pins to get the rows to update sequentially.</li>
	<li>Clock in the 32-bits of color information. The first 16-bits clocked into the RGB0 pins, correlate to the first 16 columns of the top row you're driving. The second 16-bits of RGB0 correlate to the first 16 columns of the bottom row you're driving. The RGB1 pins are used to display the second 16 columns for the two rows that are being driven. Read the Ray's Logic link for more information on how data is clocked out and displayed.</li>
	<li>Toggle the latch signal, telling the matrix to store the previously clocked color data</li>
	<li>Enable the LEDs for viewing by clearing the OE pin</li>
	<li>If you're using BCM to drive more color values, change the timer interrupt's period that is driving the display, based on the current BCM cycle. A BCM cycle corresponds to the current bit of color that you're driving. If you were driving 7-bits of color for each R, G, and B, then you would have 7 cycles, one for each bit. Read the BatSock tutorial for more information about how BCM works.</li>
	<li>Perform checks to see if you've reached the last BCM cycle, if so, increment the current row variable. What this says is that we're performing the full amount of BCM cycles for each pair of rows before moving onto the next pair of rows.</li>
	<li>Rinse, lather, and repeat so fast that no flickering on the matrix occurs</li>
</ol>

<h2>Building the Code</h2>
To build the code you're going to need both TI Code Composer Studio as well as a copy of the latest version of TivaWare (we're just using the headers to provide easier access to registers). Once those are downloaded, create a new project, and link TivaWare's include and library directories. Download the pacman code and add it to your project. To hook up the LED matrix, check out the GPIO map in LedMatrix.h. The UART (used to move pacman) has it's transmitter located on pin PC5, and the receiver on pin PC4.

If all goes according to plan, you should be able to move pacman (the yellow dot) around the level (with serial commands) and pick up pellets.

<h2>Unfinished Features</h2>
Currently, the game lacks any enemy AI and any way to win the game (eventually, you'll win by grabbing every pellet without dying). Besides that, the code to drive the matrix is complete as well as the basic game logic for moving Pacman around and eating pellets.