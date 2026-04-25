/* See LICENSE file for copyright and license details. */
/* multimedia keys */
#include <X11/XF86keysym.h>
#include "movestack.c"
/* appearance */

static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
/* systray */
static const int showsystray             = 1;  /* 0 means no systray */
static const int systrayonleft           = 0;  /* 0 means systray on right side */
static const int systraypinning          = 0;  /* 0 means follow selected monitor */
static const int systraypinningfailfirst = 1;
static const unsigned int systrayspacing = 2;  /* systray spacing */
static const char *fonts[]          = { "monospace:size=10" };
static const char dmenufont[]       = "monospace:size=10";
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#005577";
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
	[SchemeSel]  = { col_gray4, col_cyan,  col_cyan  },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "Gimp",     NULL,       NULL,       0,            1,           -1 },
	{ "Firefox",  NULL,       NULL,       1 << 8,       0,           -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */
static const int refreshrate = 120;  /* refresh rate (per second) for client move/resize */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }


/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */

static const char *dmenucmd[] = {
	"rofi",
	"-show", "drun",
	"-theme", "/home/l/.config/rofi/launcher.rasi",
	"-hover-select",
	NULL
};

static const char *termcmd[] = { "kitty", NULL };
static const char *codcmd[]     = { "code", NULL };
static const char *wlogoutcmd[] = { "wlogout", NULL };
static const char *browser[] = { "firefox", NULL };
static const char *nvimcmd[]    = { "kitty", "-e", "nvim", NULL };
static const char *emacscmd[]   = { "emacsclient", "-c", "-a", "emacs", NULL };

/* volume — pactl + dunst notification with progress bar */
static const char *volupcmd[]   = { "/bin/sh", "-c",
	"pactl set-sink-mute @DEFAULT_SINK@ false; "
	"pactl set-sink-volume @DEFAULT_SINK@ +5%; "
	"VOL=$(pactl get-sink-volume @DEFAULT_SINK@ | awk '{print $5}'); "
	"notify-send -h string:x-canonical-private-synchronous:vol "
	"-h int:value:${VOL%%%} -t 1000 'Volume' \"$VOL\"",
	NULL };
static const char *voldowncmd[] = { "/bin/sh", "-c",
	"pactl set-sink-mute @DEFAULT_SINK@ false; "
	"pactl set-sink-volume @DEFAULT_SINK@ -5%; "
	"VOL=$(pactl get-sink-volume @DEFAULT_SINK@ | awk '{print $5}'); "
	"notify-send -h string:x-canonical-private-synchronous:vol "
	"-h int:value:${VOL%%%} -t 1000 'Volume' \"$VOL\"",
	NULL };
static const char *mutecmd[]    = { "/bin/sh", "-c",
	"pactl set-sink-mute @DEFAULT_SINK@ toggle; "
	"MUTE=$(pactl get-sink-mute @DEFAULT_SINK@ | awk '{print $2}'); "
	"if [ \"$MUTE\" = 'yes' ]; then "
	"notify-send -h string:x-canonical-private-synchronous:vol -t 1000 'Volume' 'Muted'; "
	"else "
	"VOL=$(pactl get-sink-volume @DEFAULT_SINK@ | awk '{print $5}'); "
	"notify-send -h string:x-canonical-private-synchronous:vol "
	"-h int:value:${VOL%%%} -t 1000 'Volume' \"$VOL\"; fi",
	NULL };

/* brightness — brightnessctl + dunst notification with progress bar */
static const char *britupcmd[]   = { "/bin/sh", "-c",
	"brightnessctl set 5%+; "
	"BRIT=$(brightnessctl -m | awk -F, '{print $4}'); "
	"notify-send -h string:x-canonical-private-synchronous:brit "
	"-h int:value:${BRIT%%%} -t 1000 'Brightness' \"$BRIT\"",
	NULL };
static const char *britdowncmd[] = { "/bin/sh", "-c",
	"brightnessctl set 5%-; "
	"BRIT=$(brightnessctl -m | awk -F, '{print $4}'); "
	"notify-send -h string:x-canonical-private-synchronous:brit "
	"-h int:value:${BRIT%%%} -t 1000 'Brightness' \"$BRIT\"",
	NULL };

static const Key keys[] = {
	/* modifier                     key                      function        argument */
	{ MODKEY,                       XK_d,                    spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_b,                    spawn,          {.v = browser } },
	{ MODKEY,                       XK_Return,               spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_n,                    togglebar,      {0} },
	{ MODKEY,                       XK_j,                    focusstackvis,  {.i = +1 } },
	{ MODKEY,                       XK_k,                    focusstackvis,  {.i = -1 } },
	{ MODKEY,                       XK_i,                    incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_h,                    setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,                    setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_Return,               zoom,           {0} },
	{ MODKEY,                       XK_Tab,                  view,           {0} },
	{ MODKEY,                       XK_q,                    killclient,     {0} },
	{ MODKEY,                       XK_t,                    setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,                    setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,                    setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_space,                setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,                togglefloating, {0} },
	{ MODKEY,                       XK_0,                    view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,                    tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,                focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period,               focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,                tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period,               tagmon,         {.i = +1 } },
	TAGKEYS(                        XK_1,                                    0)
	TAGKEYS(                        XK_2,                                    1)
	TAGKEYS(                        XK_3,                                    2)
	TAGKEYS(                        XK_4,                                    3)
	TAGKEYS(                        XK_5,                                    4)
	TAGKEYS(                        XK_6,                                    5)
	TAGKEYS(                        XK_7,                                    6)
	TAGKEYS(                        XK_8,                                    7)
	TAGKEYS(                        XK_9,                                    8)
	TAGKEYS(                        XK_0,                                    9)


	{ MODKEY|ShiftMask,             XK_F1,                    hide,           {0} },
	{ MODKEY|ShiftMask,             XK_F2,                    show,           {0} },
	{ MODKEY|ControlMask,           XK_s,                    showall,        {0} },
	{ MODKEY|ControlMask,           XK_j,                    focusstackhid,  {.i = +1 } },
	{ MODKEY|ControlMask,           XK_k,                    focusstackhid,  {.i = -1 } },
	{ MODKEY,                       XK_w,                    togglewin,      {0} },

  { MODKEY,           XK_e,      spawn, {.v = emacscmd   } },
  { MODKEY,           XK_c,      spawn, {.v = codcmd      } },
  { MODKEY,           XK_v,      spawn, {.v = nvimcmd     } },
  { MODKEY,           XK_p,      spawn, {.v = wlogoutcmd  } },
  { MODKEY|ShiftMask,             XK_j,      movestack,      {.i = +1 } },
  { MODKEY|ShiftMask,             XK_k,      movestack,      {.i = -1 } },

	/* volume keys */
	{ 0, XF86XK_AudioRaiseVolume,                            spawn,          {.v = volupcmd   } },
	{ 0, XF86XK_AudioLowerVolume,                            spawn,          {.v = voldowncmd } },
	{ 0, XF86XK_AudioMute,                                   spawn,          {.v = mutecmd    } },

	/* brightness keys */
	{ 0, XF86XK_MonBrightnessUp,                             spawn,          {.v = britupcmd   } },
	{ 0, XF86XK_MonBrightnessDown,                           spawn,          {.v = britdowncmd } },

	{ MODKEY|ShiftMask,             XK_q,                    quit,           {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};
