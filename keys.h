
typedef struct {
	unsigned int mod;
	KeySym sym;
	int error;
} KeySymbol;

KeySymbol parse_keysymbol(const char *format);
