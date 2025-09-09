#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <stdlib.h>
#include <string.h>
#include <confuse.h>

#include "util.h"
#include "dat.h"
#include "fns.h"
#include "keys.h"

static int parsefunc(cfg_t *, cfg_opt_t *, const char *, void *);
static int parselayout(cfg_t *, cfg_opt_t *, const char *, void *);
static int parsekey(cfg_t *, cfg_opt_t *, const char *, void *);
static int parsebutton(cfg_t *, cfg_opt_t *, const char *, void *);
static int parsewhere(cfg_t *, cfg_opt_t *, const char *, void *);

static void freekey(void *);
static void freebutton(void *);

static void loadappearance(cfg_t *cfg);
static void loadlayout(cfg_t *cfg);
static void loadtags(cfg_t *cfg);
static void loadrules(cfg_t *cfg);
static void loadkeys(cfg_t *cfg);
static void loadbuttons(cfg_t *cfg);

static Arg ploadarg(cfg_t *cfg);

/* See LICENSE file for copyright and license details. */

/* appearance */
const char col_gray1[]       = "#222222";
const char col_gray2[]       = "#444444";
const char col_gray3[]       = "#bbbbbb";
const char col_gray4[]       = "#eeeeee";
const char col_cyan[]        = "#005577";

static const char *dmenucmd[] = { "rofi", "-show", "drun", NULL };
static const char *termcmd[]  = { "st", NULL };

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

const Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },
	{ MODKEY|ShiftMask,             XK_r,      restart,        {0} },
};

const Button buttons[] = {
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


Config defaultconfig = {
	.appearance = {
		.borderpx  = 1,        /* border pixel of windows */
		.snap      = 32,       /* snap pixel */
		.showbar = 1,        /* 0 means no bar */
		.topbar = 1,        /* 0 means bottom bar */
		.gappx = 4,
		.fonts = (const char*[]){ "monospace:size=10" },
		.fontscount = 1,
		.colors = {
			/*               fg         bg         border   */
			[SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
			[SchemeSel]  = { col_gray4, col_cyan,  col_cyan  },
		}
	},
	.lsettings = {
		.mfact     = 0.55, /* factor of master area size [0.05..0.95] */
		.nmaster     = 1,    /* number of clients in master area */
		.resizehints = 1,    /* 1 means respect size hints in tiled resizals */
		.lockfullscreen = 1, /* 1 will force focus on the fullscreen window */
		.refreshrate = 120,  /* refresh rate (per second) for client move/resize */
	},
	.tags = (char*[]){ "1", "2", "3", "4", "5", "6", "7", "8", "9" },
	.tagscount = 9,

	.rules = (Rule[]){
		{ "Gimp",     NULL,       NULL,       0,            1,           -1 },
		{ "Firefox",  NULL,       NULL,       1 << 8,       0,           -1 },
	},
	.rulescount = 2,

	.keys = (Key*)keys,
	.keyscount = LENGTH(keys),

	.buttons = (Button*)buttons,
	.buttonscount = LENGTH(buttons)
};

/* layout(s) */
const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};
int layoutscount = LENGTH(layouts);

Config *currentconfig = &defaultconfig;

void
loadconfig(const char *path)
{
	static cfg_opt_t color_type_opts[] = {
		CFG_STR("bg", NULL, CFGF_NODEFAULT),
		CFG_STR("fg", NULL, CFGF_NODEFAULT),
		CFG_STR("border", NULL, CFGF_NODEFAULT),
		CFG_END(),
	};
	static cfg_opt_t colors_opts[] = {
		CFG_SEC("normal", color_type_opts, CFGF_NODEFAULT),
		CFG_SEC("selected", color_type_opts, CFGF_NODEFAULT),
		CFG_END(),
	};
	static cfg_opt_t appearance_opts[] = {
		CFG_INT("borderpx", 0, CFGF_NODEFAULT),
		CFG_INT("snap", 0, CFGF_NODEFAULT),
		CFG_BOOL("showbar", 0, CFGF_NODEFAULT),
		CFG_BOOL("topbar", 0, CFGF_NODEFAULT),
		CFG_INT("gappx", 0, CFGF_NODEFAULT),
		CFG_STR_LIST("fonts", NULL, CFGF_NODEFAULT),
		CFG_SEC("colors", colors_opts, CFGF_NODEFAULT),
		CFG_END(),
	};
	static cfg_opt_t layout_opts[] = {
		CFG_FLOAT("mfact", 0, CFGF_NODEFAULT),
		CFG_INT("nmaster", 0, CFGF_NODEFAULT),
		CFG_BOOL("resizehints", 0, CFGF_NODEFAULT),
		CFG_BOOL("lockfullscreen", 0, CFGF_NODEFAULT),
		CFG_INT("refreshrate", 0, CFGF_NODEFAULT),
		CFG_END(),
	};
	static cfg_opt_t rule_opts[] = {
		CFG_STR("class", NULL, CFGF_NODEFAULT),
		CFG_STR("instance", NULL, CFGF_NONE),
		CFG_STR("title", NULL, CFGF_NONE),
		CFG_INT("tagmask", 0, CFGF_NONE),
		CFG_BOOL("floating", 0, CFGF_NONE),
		CFG_INT("monitor", -1, CFGF_NONE),
		CFG_END(),
	};
	static cfg_opt_t keys_opts[] = {
		CFG_PTR_CB("bind", NULL, CFGF_NODEFAULT, parsekey, freekey),
		CFG_PTR_CB("func", NULL, CFGF_NODEFAULT, parsefunc, NULL), 
		CFG_STR("cmd", NULL, CFGF_NODEFAULT),
		CFG_INT("i", 0, CFGF_NODEFAULT), 
		CFG_FLOAT("f", 0.0, CFGF_NODEFAULT),
		CFG_PTR_CB("layout", NULL, CFGF_NODEFAULT, parselayout, NULL),
		CFG_END(),
	};
	static cfg_opt_t button_opts[] = {
		CFG_PTR_CB("bind", NULL, CFGF_NODEFAULT, parsebutton, freebutton),
		CFG_INT_CB("where", 0, CFGF_NODEFAULT, parsewhere),
		CFG_PTR_CB("func", NULL, CFGF_NODEFAULT, parsefunc, NULL), 
		CFG_STR("cmd", NULL, CFGF_NODEFAULT),
		CFG_INT("i", 0, CFGF_NODEFAULT), 
		CFG_FLOAT("f", 0.0, CFGF_NODEFAULT),
		CFG_PTR_CB("layout", NULL, CFGF_NODEFAULT, parselayout, NULL),
		CFG_END(),
	};
	static cfg_opt_t config_opts[] = {
		CFG_SEC("appearance", appearance_opts, CFGF_NODEFAULT),
		CFG_SEC("layout", layout_opts, CFGF_NODEFAULT),
		CFG_STR_LIST("tags", NULL, CFGF_NODEFAULT),
		CFG_SEC("rule", rule_opts, CFGF_MULTI),
		CFG_SEC("key", keys_opts, CFGF_MULTI),
		CFG_SEC("button", button_opts, CFGF_MULTI),
		CFG_END(),
	};

	static cfg_t *cfg = NULL;

	if(cfg) {
		cfg_free(cfg);
		cfg = NULL;
	}

	cfg = cfg_init(config_opts, CFGF_NONE);
	if(!cfg || cfg_parse(cfg, path) == CFG_PARSE_ERROR) {
		perror("cfg_parse");
		return;
	}

	if(currentconfig != &defaultconfig) {
		free(currentconfig->appearance.fonts);
		free(currentconfig->tags);
		free(currentconfig->rules);
		free(currentconfig->keys);
		free(currentconfig);
	}
	currentconfig = malloc(sizeof(*currentconfig));
	*currentconfig = defaultconfig;

	loadappearance(cfg_getsec(cfg, "appearance"));
	loadlayout(cfg_getsec(cfg, "layout"));
	loadtags(cfg);
	loadrules(cfg);
	loadkeys(cfg);
	loadbuttons(cfg);
}

void
loadappearance(cfg_t *sec)
{
	currentconfig->appearance.borderpx = cfg_getint(sec, "borderpx");
	currentconfig->appearance.snap     = cfg_getint(sec, "snap");
	currentconfig->appearance.showbar  = cfg_getbool(sec, "showbar");
	currentconfig->appearance.topbar   = cfg_getbool(sec, "topbar");
	currentconfig->appearance.gappx    = cfg_getint(sec, "gappx");
	currentconfig->appearance.fontscount = cfg_size(sec, "fonts");
	currentconfig->appearance.fonts = ecalloc(currentconfig->appearance.fontscount, sizeof(currentconfig->appearance.fonts));

	for(int i = 0; i < currentconfig->appearance.fontscount; i++) {
		currentconfig->appearance.fonts[i] = cfg_getnstr(sec, "fonts", i);
	}

	cfg_t *colors = cfg_getsec(sec, "colors");
	
	currentconfig->appearance.colors[SchemeSel][0] = cfg_getstr(cfg_getsec(colors, "selected"), "fg");
	currentconfig->appearance.colors[SchemeSel][1] = cfg_getstr(cfg_getsec(colors, "selected"), "bg");
	currentconfig->appearance.colors[SchemeSel][2] = cfg_getstr(cfg_getsec(colors, "selected"), "border");

	currentconfig->appearance.colors[SchemeNorm][0] = cfg_getstr(cfg_getsec(colors, "normal"), "fg");
	currentconfig->appearance.colors[SchemeNorm][1] = cfg_getstr(cfg_getsec(colors, "normal"), "bg");
	currentconfig->appearance.colors[SchemeNorm][2] = cfg_getstr(cfg_getsec(colors, "normal"), "border");
}

void
loadlayout(cfg_t *sec)
{
	currentconfig->lsettings.mfact          = cfg_getfloat(sec, "mfact");
	currentconfig->lsettings.nmaster        = cfg_getint(sec, "nmaster");
	currentconfig->lsettings.resizehints    = cfg_getbool(sec, "resizehints");
	currentconfig->lsettings.lockfullscreen = cfg_getbool(sec, "lockfullscreen");
	currentconfig->lsettings.refreshrate    = cfg_getint(sec, "refreshrate");
}

void
loadtags(cfg_t *sec)
{
	currentconfig->tagscount = cfg_size(sec, "tags");
	currentconfig->tags      = ecalloc(currentconfig->tagscount, sizeof(currentconfig->tags[0]));

	for(int i = 0; i < currentconfig->tagscount; i++) {
		currentconfig->tags[i] = cfg_getnstr(sec, "tags", i);
	}
}

void
loadrules(cfg_t *sec)
{
	currentconfig->rules = NULL;
	currentconfig->rulescount = cfg_size(sec, "rule");
	if(currentconfig->rulescount == 0)
		return;

	currentconfig->rules = ecalloc(currentconfig->rulescount, sizeof(currentconfig->rules[0]));
	for(int i = 0; i < currentconfig->rulescount; i++) {
		cfg_t *rsec = cfg_getnsec(sec, "rule", i);
		
		Rule *r = currentconfig->rules + i;
		r->instance = cfg_getstr(rsec, "instance");
		r->class    = cfg_getstr(rsec, "class");
		r->title    = cfg_getstr(rsec, "title");

		r->tags       = cfg_getint(rsec, "tagmask");
		r->monitor    = cfg_getint(rsec, "monitor");
		r->isfloating = cfg_getbool(rsec, "floating");
	}
}

void
loadkeys(cfg_t *sec)
{
	currentconfig->keys = NULL;
	currentconfig->keyscount = cfg_size(sec, "key");
	if(currentconfig->keyscount == 0)
		return;

	currentconfig->keys = ecalloc(currentconfig->keyscount, sizeof(currentconfig->keys[0]));
	for(int i = 0; i < currentconfig->keyscount; i++) {
		Key *k = currentconfig->keys + i;

		cfg_t *ksec = cfg_getnsec(sec, "key", i);
		KeySymbol *ks = cfg_getptr(ksec, "bind");

		k->keysym = ks->sym;
		k->mod    = ks->mod;

		k->func = (void(*)(const Arg*))cfg_getptr(ksec, "func");
		k->arg  = ploadarg(ksec);
	}
}

void
loadbuttons(cfg_t *sec)
{
	currentconfig->buttons = NULL;
	currentconfig->buttonscount = cfg_size(sec, "button");
	if(currentconfig->buttonscount == 0)
		return;

	currentconfig->buttons = ecalloc(currentconfig->buttonscount, sizeof(currentconfig->buttons[0]));
	for(int i = 0; i < currentconfig->buttonscount; i++) {
		Button *b = currentconfig->buttons + i;

		cfg_t *ksec = cfg_getnsec(sec, "button", i);
		ButtonSymbol *bs = cfg_getptr(ksec, "bind");

		b->click  = cfg_getint(ksec, "where");
		b->button = bs->button;
		b->mask   = bs->mod;

		b->func = (void(*)(const Arg*))cfg_getptr(ksec, "func");
		b->arg  = ploadarg(ksec);
	}
}

int 
parsefunc(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result)
{
	static struct {
		const char *name;
		void (*fn)(const Arg *);
	} funcs[] = {
#define FN(name) { #name, name }
		FN(spawn),
		FN(togglebar),
		FN(focusstack),
		FN(incnmaster),
		FN(setmfact),
		FN(zoom),
		FN(view),
		FN(killclient),
		FN(setlayout),
		FN(togglefloating),
		FN(tag),
		FN(focusmon),
		FN(tagmon),
		FN(toggleview),
		FN(toggletag),
		FN(movemouse),
		FN(resizemouse),
		FN(quit),
		FN(restart),
#undef FN
	};

	for(int i = 0; i < LENGTH(funcs); i++) {
		if(strcmp(funcs[i].name, value) == 0) {
			*(void**)result = (void*)funcs[i].fn;
			return 0;
		}
	}

	result = NULL;
	return 1;
}

int 
parselayout(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result)
{
	static struct {
		const char *name;
		const Layout *lt;
	} lts[] = {
		{ .name = "tile",     .lt = layouts + 0 },
		{ .name = "floating", .lt = layouts + 1 },
		{ .name = "monocle",  .lt = layouts + 2 },
	};

	for(int i = 0; i < LENGTH(lts); i++) {
		if(strcmp(lts[i].name, value) == 0) {
			*(void**)result = (void*)lts[i].lt;
			return 0;
		}
	}

	result = NULL;
	return 1;
}

Arg
ploadarg(cfg_t *sec)
{
	Arg a = {0};

	if(cfg_size(sec, "cmd") > 0) a.v = cfg_getstr(sec, "cmd");
	if(cfg_size(sec, "i") > 0) a.i = cfg_getint(sec, "i");
	if(cfg_size(sec, "f") > 0) a.f = cfg_getfloat(sec, "f");
	if(cfg_size(sec, "layout") > 0) a.v = cfg_getptr(sec, "layout");

	return a;
}

int 
parsekey(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result)
{
	KeySymbol *ks = ecalloc(1, sizeof(*ks));
	*ks = parse_keysymbol(value);
	*(void**)result = ks;
	return ks->error;
}

int 
parsebutton(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result)
{
	ButtonSymbol *ks = ecalloc(1, sizeof(*ks));
	*ks = parse_buttonsymbol(value);
	*(void**)result = ks;
	return ks->error;
}

int 
parsewhere(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result)
{
	static struct {
		const char *name;
		int   click;
	} lts[] = {
		{ .name = "ltsymbol", .click = ClkLtSymbol },
		{ .name = "wintitle", .click = ClkWinTitle },
		{ .name = "window",  .click  = ClkClientWin },
		{ .name = "tagbar",  .click  = ClkTagBar },
		{ .name = "status",  .click  = ClkStatusText },
	};

	for(int i = 0; i < LENGTH(lts); i++) {
		if(strcmp(lts[i].name, value) == 0) {
			*(long int*)result = lts[i].click;
			return 0;
		}
	}
	return 1;
}

void 
freekey(void *ptr)
{
	free(ptr);
}

void
freebutton(void *ptr)
{
	free(ptr);
}
