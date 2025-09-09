
typedef struct {
	unsigned int mod;
	KeySym sym;
	int error;
} KeySymbol;

typedef struct {
	unsigned int mod;
	int button;
	int error;
} ButtonSymbol;

KeySymbol parse_keysymbol(const char *format);
ButtonSymbol parse_buttonsymbol(const char *format);
