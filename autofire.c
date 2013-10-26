/*
   gcc -lxosd -lX11 -lXtst -lXi -o autofire autofire.c

   Usage: autofire <DeviceName>
   
          DeviceName: Name of the input keyboard device (as listed in xinput)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/Intrinsic.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include <xosd.h>

/* Display Time in nanoseconds */
#define OSD_TIMEOUT 999999999    
/* Key repeat delay in nanoseconds */
#define KEY_DELAY (BILLION/50)
#define SLEEP_DELAY (BILLION/5)
#define BILLION 1000000000
#define OSD_FONT "-adobe-helvetica-*-r-*-*-24-*-*-*-*-*-*-*"
#define OSD_COLOUR "DarkGoldenrod1"

Display *display;
XDevice *xdevice = NULL;

int get_xdevice (char* name);
int get_key_state (int keycode);
int get_toggle();
void send_key(int keycode);
void send_button(int button);
xosd *osd_init();

int get_xdevice (char* name)
{
  // get the Xdevice by name (as set in xorg.conf)

  XDeviceInfo *devs;
  int num_devs=0;
  int i=0;

  devs = XListInputDevices (display, &num_devs);

  for (i=0; i<num_devs; i++)
    {
      printf ("XID %d, type %2d, name \"%s\"\n",
              (int) devs[i].id, (int) devs[i].type, devs[i].name);

      if ((strncasecmp(devs[i].name, name, strlen(name)) == 0) &&
		  (devs[i].type == XInternAtom(display,XI_KEYBOARD, True)))
        {
          printf("Opening XID %d \"%s\"\n", (int) devs[i].id, devs[i].name);
		  xdevice = XOpenDevice(display, devs[i].id);
		  return 1;
        }
    }
  fprintf(stderr, "Could not find the requested keyboard: %s\n", name);
  return 0;
}

int get_key_state (int keycode) {

  // returns: 0 -> not pressed, 1 -> pressed

  XKeyState *keystate = NULL;

  int state = 1;
  int byte = keycode / 8;
  int bit = keycode % 8;

  state = state << bit;
  keystate = ((XKeyState*) (XQueryDeviceState(display, xdevice))->data);
 
  return (keystate->keys[byte] & state) >> bit;
}

void send_key(int keycode) {
  XTestFakeKeyEvent(display, keycode, True, 0);
  XTestFakeKeyEvent(display, keycode, False, 0);
  XFlush(display); 
}

void send_button(int button) {
  XTestFakeButtonEvent(display, button,True ,0);
  XTestFakeButtonEvent(display, button,False,0);
  XFlush(display);
}

xosd *osd_init() {
  xosd *osd = NULL;
  osd = xosd_create(2);
  if (osd == NULL)
  {
    perror ("Could not create \"osd\"");
    exit (1);
  }
  xosd_set_align(osd, XOSD_center);
  xosd_set_pos(osd, XOSD_middle);
  xosd_set_font(osd,OSD_FONT);
  xosd_set_outline_offset(osd,2 );
  xosd_set_colour(osd, OSD_COLOUR);
  return(osd);
}

int get_toggle() {
  static int toggle = 0;
  static struct timespec ts = { OSD_TIMEOUT / BILLION, OSD_TIMEOUT % BILLION };
  if (get_key_state (XKeysymToKeycode(display, XStringToKeysym("F12")))) {
    xosd* osd = osd_init();
    char text[20] = "Spamming : ";
    strcat(text,(toggle == True)?"Off":"On");
    xosd_display(osd,0,XOSD_string,text);
    printf("Spamming: %s\n",(toggle == True)?"Off":"On");
    nanosleep(&ts,NULL);
    xosd_destroy(osd);
    return(toggle = !toggle);
  }
  return toggle;
}
int main (int argc, char *argv[])
{

  char *display_name = NULL;

  if ((display = XOpenDisplay (display_name)) == NULL)
    {
      printf ("Cannot connect to X server %s\n", XDisplayName (display_name));
      exit (-1);
    }

  if (argc < 2)
    {
      fprintf(stderr, "Usage: %s %s \n", argv[0], "<KeyboardName>");
      exit (-1);
    }

  int keycode = XKeysymToKeycode(display, XK_F12);
  Window grab_window = DefaultRootWindow(display);
  XGrabKey(display,keycode,       0,grab_window,False,GrabModeAsync,GrabModeAsync);
  XGrabKey(display,keycode,Mod2Mask,grab_window,False,GrabModeAsync,GrabModeAsync);
  XGrabKey(display,keycode,LockMask,grab_window,False,GrabModeAsync,GrabModeAsync);
  XGrabKey(display,keycode,Mod2Mask | LockMask,grab_window,False,GrabModeAsync,GrabModeAsync);
  
  char* name = argv[1];

  int keycode_1 = XKeysymToKeycode(display, XStringToKeysym("1"));
  int keycode_2 = XKeysymToKeycode(display, XStringToKeysym("2"));
  int keycode_3 = XKeysymToKeycode(display, XStringToKeysym("3"));
  int keycode_4 = XKeysymToKeycode(display, XStringToKeysym("4"));

  static struct timespec key_delay = { 0, KEY_DELAY };
  static struct timespec sleep_delay = { 0, SLEEP_DELAY };

  if (get_xdevice(name))
    {
	  printf("Spamming: Off - Use F12 to toggle.\n");
      while(1)
        {
          while (!(get_toggle())) { nanosleep(&sleep_delay,NULL); }
          nanosleep(&key_delay,NULL);
          if (get_key_state(keycode_1)) {
            send_key(keycode_1);
            send_button(3);
          }
          if (get_key_state(keycode_2)) { 
            send_key(keycode_2);
			send_button(3);
          }
          if (get_key_state(keycode_3)) {
			send_key(keycode_3);
//          send_button(0);
          }
          if (get_key_state(keycode_4))  {
			send_key(keycode_4);
//          send_button(0);
          }
        }
    }
  return(0);
}


