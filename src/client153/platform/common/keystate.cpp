//	Browser input adapter state table; the adapter writes real key events.
#include "Windows.h"

unsigned char pw153_key_state[256] = {0};

//	The message-loop quit flag the loop adapter consumes.
int pw153_quit_code = 0;
