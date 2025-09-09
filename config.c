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

/* layout(s) */
const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};
int layoutscount = LENGTH(layouts);

Config *currentconfig = NULL;

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
		CFG_FUNC("include", &cfg_include),
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

	if(currentconfig) {
		free(currentconfig->appearance.fonts);
		free(currentconfig->tags);
		free(currentconfig->rules);
		free(currentconfig->keys);
		free(currentconfig);
	}
	currentconfig = malloc(sizeof(*currentconfig));

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
