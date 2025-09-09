#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <ctype.h>
#include <string.h>

#include "utils.h"
#include "keys.h"

#define LENGTH(X) (sizeof (X) / sizeof (X)[0])

/*
 * key format
 * 
 * <buttonsymbol> ::= <modseq> <button> <EOL>
 *                 | <button> <EOL>
 *
 * <keysymbol> ::= <modseq> <key> <EOL>
 *              | <key> <EOL>
 *
 * <modseq> ::= <modseq> <mod> '+'
 *            | <mod> '+'
 * 
 * <mod> ::= [ anything on a mod list ]
 * <key> ::= [ anything on a key list ]
 */

static struct {
	const char *str;
	unsigned int mod;
} modlist[] = {
	{ .str = "alt",   .mod = Mod1Mask    },
	{ .str = "super", .mod = Mod4Mask    },
	{ .str = "ctrl",  .mod = ControlMask },
	{ .str = "shift", .mod = ShiftMask   },
};

typedef struct
{
	const char *s;
	int   error;
} StateMachine;

static void m_tokn(StateMachine *state, int count);

static unsigned int parse_mod(StateMachine *machine);
static unsigned int parse_modseq(StateMachine *machine);
static KeySym parse_key(StateMachine *machine);
static int parse_button(StateMachine *machine);

KeySymbol
parse_keysymbol(const char *format)
{
	KeySymbol k = {0};
	StateMachine m = { .s = format };

	k.sym = parse_key(&m);
	if(!m.error) {
		return k;
	}

	m.error = 0;
	k.mod = parse_modseq(&m);
	if(m.error) {
		k.error = m.error;
		return k;
	}

	k.sym = parse_key(&m);
	if(m.error) {
		k.error = m.error;
		return k;
	}

	return k;
}

ButtonSymbol
parse_buttonsymbol(const char *format)
{
	ButtonSymbol k = {0};
	StateMachine m = { .s = format };

	k.button = parse_button(&m);
	if(!m.error) {
		return k;
	}

	m.error = 0;
	k.mod = parse_modseq(&m);
	if(m.error) {
		k.error = m.error;
		return k;
	}

	k.button = parse_button(&m);
	if(m.error) {
		k.error = m.error;
		return k;
	}

	return k;
}

unsigned int
parse_modseq(StateMachine *m)
{
	unsigned int mod = 0, seq = 0;

	mod = parse_mod(m);
	if(m->error) {
		m->error = 1;
		return 0;
	}

	if((*m->s) != '+') {
		m->error = 1;
		return 0;
	}
	m_tokn(m, 1);

	seq = parse_modseq(m);
	if(m->error) {
		m->error = 0;
	}

	return seq | mod;
}

unsigned int
parse_mod(StateMachine *m)
{
	for(int i = 0; i < LENGTH(modlist); i++) {
		int len = strlen(modlist[i].str);
		if(memcmp(m->s, modlist[i].str, len) == 0) {
			m_tokn(m, len);
			return modlist[i].mod;
		}
	}

	m->error = 1;
	return 0;
}

KeySym
parse_key(StateMachine *m)
{
	const char *ss = m->s;

	while((*ss) != 0 && !isblank(*ss)) 
		ss++;
	
	int size = ss - m->s;
	if(size == 0) {
		m->error = 1;
		return NoSymbol;
	}

	char *mem;
	if(!(mem = malloc(size + 1))) {
		m->error = 1;
		return NoSymbol;
	}
	memcpy(mem, m->s, size);
	mem[size] = 0;

	KeySym r = XStringToKeysym(mem);
	free(mem);

	if(r == NoSymbol)
		m->error = 1;
	else {
		m_tokn(m, size);
	}

	return r;
}

int
parse_button(StateMachine *m)
{
	static struct {
		const char *bstr;
		int button;
	} list[] = {
		{ "left", Button1 },
		{ "right", Button3 },
		{ "middle", Button2 } 
	};
	int size = strlen(m->s);

	for(int i = 0; i < LENGTH(list); i++) {
		if(strncmp(list[i].bstr, m->s, size) == 0) {
			m_tokn(m, strlen(list[i].bstr));
			return list[i].button;
		}
	}
	
	m->error = 1;
	
	return -1;
 }

void
m_tokn(StateMachine *m, int size)
{
	for(int i = 0; i < size; i++)
		if((*m->s) == 0)
			return;
		else
		 	m->s++;

	while((*m->s) != 0 && isblank(*m->s)) 
		m->s++;
}
