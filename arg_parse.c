#include "arg_parse_gen.h"
#include "div.h"
#include <stdio.h>
#define GET_ARG_STR argv[0][2] ? *argv +2 :*++argv
#define GET_ARG strtol(GET_ARG_STR, NULL, 0);
#define GET_ARG_F strtof(GET_ARG_STR, NULL);
#define SET(C, VAR, VALUE) case C: VAR = VALUE; break

#ifndef VERSION
#define VERSION ".9"
#endif

const char* arg_string = "SXYZcdehnrtvw";
const char** defaultSingleArgParse(const char **argv, bool* stop) {
    switch(argv[0][1]) {
        SET('S', state.scale_mode, getFromEnumValue(GET_ARG_STR));
        SET('X', state.align_mode_x, getFromEnumValue(GET_ARG_STR));
        SET('Y', state.align_mode_y, getFromEnumValue(GET_ARG_STR));
        SET('Z', state.zoom, GET_ARG_F(GET_ARG_STR));
        SET('c', state.cols, GET_ARG);
        SET('d', state.right_to_left, 1);
        SET('e', state.parent, GET_ARG);
        SET('n', state.file_index, GET_ARG);
        SET('r', state.rows, GET_ARG);
        SET('t', state.user_title, GET_ARG_STR);
        SET('w', state.wrap, 1);
        case 'h':
            printf("div: usage [-%s]\n", arg_string);
            exit(0);
        case 'v':
            printf("Version %s\n", VERSION);
            exit(0);
        default:
            *stop =1;
    }
    return argv;
}
