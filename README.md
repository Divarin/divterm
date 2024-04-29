DivTerm 128 is a SwiftLink based terminal emulator for the Commodore 128 computer.
Development started: April 2024
High-level roadmap of intended features:

* Run in native 128 mode to easily enable usage of numeric keypad and VDC (80 column output) [done]
* Pause mode [done]
* Scroll-back / Scroll-forward [done but could use some polish]
* Toggle between VIC (40 column output) and VDC (80 column output) [done]
* ASCII/ANSI emulation [done but may need more later testing will tell]
* Extended ASCII character set in ASCII mode [not done]
* ATASCII emulation [not done]
* ATASCII character set in ATASCII mode [not done]
* Direct to printer (Teletype mode) [not done]
* Phonebook with 'last called date' tracking [not done]

What is not planned but I might do:
* Record to buffer, save & print buffer.

What is planned to not do:
* File transfers

 ---
 
Usage:
At this time requires swiftlink compatible device, does not (yet) work with userport modems.

Pause/Scroll:
Runstop key enters pause mode.  While in this mode the VIC border turns red.  Any new data that comes in while you're paused goes into a buffer (until it's full).
In pause mode you can scroll-forward with down arrow and scroll-back with up arrow.
Any other key leaves pause mode.  When leaving pause mode the VIC border turns yellow while spitting out the remaining buffer and bringing you up to live, at that time the border changes back to the default (gray).
At the moment there is no indication on the VDC output if you're in pause mode or not (VDC doesn't support borders so I'm currently looking for a good indicator on the VDC without printing text on the screen)

F1 cycles through baud rates: 300, 1200, 2400, 4800, 9600, 19200, 38400.
F3 toggles between 40 volumn VIC output and 80 column VDC output.
These key assignments may change in the future as more features are added.
