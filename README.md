A C++ home automation daemon I wrote and evolved for my own home over a couple years. 
Sharing for others to use if they want, or contribute if they want.
It's been a little while since I made any changes to this, so I will relay a README to the best I remember everything.

Currently does not read from any configuration, just compiles with devices specified in automationd/automationd.cpp,
which currently has my own home configuration in it as an example.

Currently supports Insteon controls (using either Serial PLM or TCP/IP Hub) and Philips Hue lights

Modeled as controller/responder through AutomationEngine in engine.cpp, where each controller/responder has a common 
name (type std::string)

At some point I added secondary actions, such that double tapping (fast on/fast off) the insteon switch in the
direction it is already in (fast on if switch is on, fast off if switch is off) will scroll through the secondary actions,
and then single tapping the insteon switch in the direction it is already in will toggle that secondary action.
(Such that you could have a light group inside a light group that you can turn on and off with the same switch
 without modifying the rest of the light group)
 
As well there is automatic colour temperature control built in to the PhilipsHue class based on location and sun position so that
light will automatically change from a specified temperature to a specified temperature at a specified height range of the sun
(6500K to 2000K at -5 min, 10 max in the example "config" located in automationd.cpp), this is configurable on a per light/light
group basis.

Currently Serial is only supported under Linux (as it uses /dev/tty to access the serial device), 
while TCP/IP will work under both Windows and Linux.

On my eventually TODO list is:

-Allow Insteon devices to be responders as well (So that for example, pressing a button on a 6 button scene controller can 
 light/unlight other buttons

-Add a mode for insteon on/off switches for dimming (i.e. an insteon microswitch on a lamp, with each subsequent press of the
 lamp switch it will start from dim->less dim->full on->off, with an optional timer so that if you set a specific dimness level,
 and then wait 5 minutes, and press the button again it turns off instead of getting brighter.)

-Fix the dimmer. It doesn't work well.
