#include "VariableNameGenerator.h"
#include <stdio.h>
#include <string.h>

#define VAR_PREFIX "T"

static int sequenceId = 0;
static char buffer[32];

void vng_init(void) {
    sequenceId = 0;
}

char* vng_gen(void) {
    sequenceId++;
    sprintf(buffer, "%s%d", VAR_PREFIX, sequenceId);
    return buffer;
}

void vng_clear(void) {
    sequenceId = 0;
}

int vng_get_count(void) {
    return sequenceId;
}
