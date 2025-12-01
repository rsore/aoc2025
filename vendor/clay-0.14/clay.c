#define CLAY_IMPLEMENTATION
#include "clay.h"

void
SatisfyCompiler(void)
{
    CLAY({ .id = CLAY_ID("SatisfyCompiler") }) {
        CLAY_TEXT(CLAY_STRING("Test"), CLAY_TEXT_CONFIG({ .fontId = 0, .fontSize = 24 }));
    }
}
