# Klikacz
Extremely simple Tall Counter (2 digits), Arduino mini pro based.

<img src=Images/Front.PNG>

Functions:
  * Counts :)
    - UP (button   +1). top limit = 99
    - DOWN (button -1)  bottom limit  = 0
  * Extras:
    - show battery level (hold button UP)
    - shows total number pressing UP since power on (hold button DOWN)
		
Use case:

Count people in shop. (Corona Virus prevention in some countries). Every time you let someone (press UP). When someone leaving (press DOWN). You can check anytime how many prersons are in shop (to obey limit) [2 digits only]

As bonus, at end of the day, you can check how many customers you have in shop.


Build from what I had in the house (and 3d printed rest).

What is needed:
  * Arduino Pro mini
  * 2 digits segment LED display, common cathode (19mm height) matching arduino pinout raster
  * 2 x buttons (monostable)
  * 1 power off slider (like this one used for voltage selector in power supply)
  * any AA battery holder (for salvaging needed springs and connectors)
  * big cappacitor (for power stability during drop test). I used 1000uF 16V
  * few wires to connect buttons, battery and cappacitor.
  * 3 x AA battery
  * 6 screews M3 (to assembly the cassing)
  * glue (like superglue) to install display to case
	
Tools:
  * 3d printer, any kind
  * soldering iron
  * USB to serial TTL (to program arduino)
  * screewdriver (matching your screews)

Software:
  * Arduino 1.8.9 (tested)
	
Build instrucion will be added in wiki.
