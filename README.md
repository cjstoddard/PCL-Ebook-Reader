# PCL-Ebook-Reader
PicoCalc Lyra Ebook Reader

This is a small text mode epub reader. I was surprised to find there were very few of this type program out there and the ones that were are all old, outdated and unmaintained. I am targeting the Picocalc with the Lyra mod for the moment, this will run on a desktop Linux machine, but keep in mind, it specifically targets a small 320x320 screen, so there may be some weirdness in rendering.

The goal here was to make an epub reader that could be compiled on the PicoCalc Lyra directly and use as few additional libraries as possible, especially libraries that need to be compiled and installed. Unfortunately the current state of PicoCalc Lyra Linux is it is woefully lacking in this department. It does not contain ncursesw-dev, libzip-dev or even the html2text program, which I would have normally used in a project like this. So I had to depend solely on ncurses, which does not handle UTC-8 encoding at all for the text reader. I also have to depend on a shell script to extract the epub file, strip out as much HTML and UTF-8 encoding as possible and then format it as best I can. Fortunately html2text is a GPL python script, which can be distributed with the project. This solution is not perfect but it does work. Perhaps later versions of PicoCalc Lyra Linux will include a better development environment.

> git clone https://github.com/cjstoddard/PCL-Ebook-Reader.git
>
> cd PCL-Ebook-Reader
>
> make
>
> sudo make install

The program is then invoked with;

> ebook your.epub

You can navigate the book with the up/down curser keys and pressing the space bar will go to the next page. At the moment there are no bookmarks, but it does remember your place in the book.

How this works:
When you run the program, the ebook shell script looks to see if the epub book has been converted to text yet, if it has not been done, it will proceed to convert the epub file to text. The first time you open a specific epub file, the conversion will take a minute or two, but after that, when you open it, will go directly to the text file instead of converting it. The conversion process makes two files, a .txt file and a _ascii.txt file. this program uses the _ascii.txt, you can delete the other one if you want to. I left both files in case you want to use a better program to view the book. the .txt file retains the UTF-8 encoding, _ascii.txt file has the UTF-8 encoding stripped out, because textreader does not support it.

Acknowledgments:

html2text version 3.1, (C) 2004-2008 Aaron Swartz. GNU GPL 3. by Aaron Swartz (me@aaronsw.com), with contributers; Martin 'Joey' Schulze, Ricardo Reyes, Kevin Jay North

Disclaimer: This software is provided "AS IS", without warranty of any kind, express or implied, including but not limited to warranties of merchantability, fitness for a paticular purpose and nonifringment. In no event shall the author or copyright holders be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise, arising from, out of or in connection with the software or the use or other dealings in the software.

