//============================================================================
// Name        : automationd.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include "engine.h"
#include "hue.h"
#include "insteonplm.h"

#ifdef _WIN32
#include <winsock2.h>
#endif
int main() {
#ifdef _WIN32
  WSADATA       wsd;

  if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
  {
      printf("Failed to load Winsock!\n");
  }
#endif

  InsteonPLM insteon("/dev/ttyUSB1", "A1B2C3");
  PhilipsHue hue("192.168.x.x", "apikey1");
  PhilipsHue hue_s("192.168.x.x", "apikey2");
  hue.add_light("laundry", "lights/23/state");
  hue.add_light("basement", "groups/22/action");
  hue.add_light("kitchen", "groups/21/action");
  hue.add_light("kitchen_bar", "groups/9/action");
  hue.add_light("foyer", "groups/20/action");
  hue_s.add_light("hallway", "groups/1/action");
  hue.add_light("bedroomlamp", "lights/55/state");
  hue.add_light("livingroom", "groups/19/action");
  hue.add_light("upstairs", "groups/15/action");
  hue.add_light("upstairsbath", "groups/28/action");
  hue.add_light("office", "groups/18/action");
  hue.add_light("2ndfloorbath", "groups/17/action");
  hue.add_light("bedroomlamp2", "lights/59/state");
  hue_s.add_light("bedroomlamp3", "lights/1/state");
  hue.add_light("masterbed", "groups/13/action");
  hue_s.add_light("frontdoor", "groups/4/action");
  hue.set_location(43.7, -79.4);
  hue_s.set_location(43.7, -79.4);
  hue.set_sun_settings("laundry", -5, 10, 6500, 2000);
  hue.set_sun_settings("basement", -5, 10, 6500, 2000);
  hue.set_sun_settings("kitchen", -5, 10, 6500, 2000);
  hue.set_sun_settings("foyer", -5, 10, 6500, 2000);
  hue_s.set_sun_settings("hallway", -5, 10, 6500, 2000);
  hue.set_sun_settings("bedroomlamp", -5, 10, 6500, 2000);
  hue.set_sun_settings("livingroom", -5, 10, 6500, 2000);
  hue.set_sun_settings("upstairs", -5, 10, 6500, 2000);
  hue.set_sun_settings("upstairsbath", -5, 10, 6500, 2000);
  hue.set_sun_settings("office", -5, 10, 6500, 2000);
  hue.set_sun_settings("2ndfloorbath", -5, 10, 6500, 2000);
  hue.set_sun_settings("bedroomlamp2", -5, 10, 6500, 2000);
  hue_s.set_sun_settings("bedroomlamp3", -5, 10, 6500, 2000);
  hue.set_sun_settings("masterbed", -5, 10, 6500, 2000);
  //insteon.add_device("hub", "2CACF7");
  insteon.add_device("basement", "AAAAAA");
  insteon.add_device("basement2", "BBBBBB");
  insteon.bind_device("basement2", "basement");
  insteon.add_device("kitchen", "CCCCCC");
  insteon.add_device("foyer", "DDDDDD");
  insteon.add_device("foyer2", "EEEEEE");
  insteon.bind_device("foyer2", "foyer");
  insteon.add_device("hallway", "FFFFFF");
  insteon.add_device("hallway2", "111111");
  insteon.add_device("hallway3", "222222");
  insteon.bind_device("hallway2", "hallway");
  insteon.bind_device("hallway3", "hallway");
  insteon.add_device("livingroom", "333333");
  insteon.add_device("livingroom2", "444444");
  insteon.bind_device("livingroom2", "livingroom");
  insteon.add_device("upstairs", "555555");
  insteon.add_device("upstairs2", "666666");
  insteon.bind_device("upstairs2", "upstairs");
  insteon.add_device("upstairsbath", "777777");
  insteon.add_device("bedroomlamp", "888888");
  insteon.add_device("2ndfloorbath", "999999");
  insteon.add_device("office", "A0A0A0");
  insteon.add_device("bedroomlamp2", "B0B0B0");
  insteon.add_device("bedroomlamp3", "C0C0C0");
  insteon.add_device("masterbed", "D0D0D0");
  insteon.add_device("frontdoor", "E0E0E0");

  AutomationEngine engine;
  engine.add_controller(&insteon);
  engine.add_binding("basement", -1, &hue, LevelControl);
  engine.add_secondary_binding("basement", -1, "laundry", false, &hue, LevelControl);
  engine.add_binding("kitchen", -1, &hue, LevelControl);
  engine.add_secondary_binding("kitchen", -1, "kitchen_bar", true, &hue, LevelControl);
  engine.add_binding("foyer", -1, &hue, LevelControl);
  engine.add_binding("hallway", -1, &hue_s, LevelControl);
  engine.add_binding("livingroom", -1, &hue, LevelControl);
  engine.add_binding("upstairs", -1, &hue, LevelControl);
  engine.add_binding("upstairsbath", -1, &hue, LevelControl);
  engine.add_binding("bedroomlamp", -1, &hue, LevelControl);
  engine.add_binding("2ndfloorbath", -1, &hue, LevelControl);
  engine.add_binding("office", -1, &hue, LevelControl);
  engine.add_binding("bedroomlamp2", -1, &hue, LevelControl);
  engine.add_binding("bedroomlamp3", -1, &hue_s, LevelControl);
  engine.add_binding("masterbed", -1, &hue, LevelControl);
  engine.add_binding("frontdoor", -1, &hue_s, LevelControl);
  engine.run();

  return 0;
}
