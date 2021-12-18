#ifndef DIV
#define DIV

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


#define LEN(A) (sizeof(A)/sizeof(A[0]))
#define MIN(A,B) (A<B?A:B)
#define MAX(A,B) (A>B?A:B)
#define MAX_FILE_NAMES 255
#define MAX_IMAGES 16
#define GET(V, DEFAULT) (V?V:DEFAULT)

#define CLASSNAME "div"

#define RUN_EVENT(X) do {if(events[X])events[X]();}while(0)
#define RUN_EVENT_WITH_ARGS(X, ...) do {if(events[X])events[X](__VA_ARGS__);}while(0)
typedef struct {
    uint8_t  mod;
    uint32_t keysym;
    void (*func)();
    int arg;
    uint8_t keycode;
    char type;
} Binding;

extern Binding bindings[];
extern Binding user_bindings[];
extern const char** initial_args;
extern int initial_num_args;

enum {
    ON_STARTUP = 32,
    PROCESS_ARGS,
    PRE_MAP_WINDOW,
    POST_XCONNECTION,
    OPEN_IMAGES,
    RENDER,
    SET_TITLE,
    POST_EVENT,
    LAST_EVENT
};
typedef enum {
    SCALE_NORMAL,
    SCALE_WIDTH,
    SCALE_HEIGHT,
    SCALE_MAX,
    SCALE_MIN,
    SCALE_FILL,
    LAST_SCALE_MODE,
} ScaleMode;

typedef enum {
    ALIGN_CENTER,
    ALIGN_TOP_LEFT,
    ALIGN_BOTTOM_RIGHT,
    LAST_ALIGN_MODE
} AlignMode;

extern void (*events[])();
extern void (*scaleFunc)(const char* buf, uint32_t original_width, uint32_t original_height, char* out_buf, uint32_t width, uint32_t height, int num_channels);
const char** defaultSingleArgParse(const char **argv, bool* stop);
void render();

void maybe_render();

typedef struct {
    short x;
    short y;
    uint16_t width;
    uint16_t height;
} Geometry;
typedef struct {
    const char* name;
    uint32_t image_width;
    uint32_t image_height;
    void* image_data;
    void* raw;

    int16_t offset_x;
    int16_t offset_y;
    float zoom;
    uint16_t padding_x;
    uint16_t padding_y;
    Geometry geometry;
} ImageInfo;

typedef struct State {
    const char** file_names;
    uint16_t num_files;

    int file_index;
    uint32_t rows;
    uint32_t cols;
    ScaleMode scale_mode;
    AlignMode align_mode_x;
    AlignMode align_mode_y;
    float zoom;
    bool right_to_left;

    uint16_t ignore_mask;
    uint32_t xevent_mask;
    uint32_t default_binding_type;
    uint32_t bg_color;
    uint32_t fg_color;
    const char* name;
    const char* user_title;

    int16_t start_x;
    int16_t start_y;
    uint16_t padding_x;
    uint16_t padding_y;

    uint32_t parent;
    uint32_t wid;
    uint32_t drawable;
    uint16_t win_width;
    uint16_t win_height;
    bool dirty;

    void* image_context;
} State;


int addNewEventFD(int fd, short events, void (*callback)());
void removeExtraEvent(int index);


extern ImageInfo image_holders[];
extern State state;

static inline int getNumActiveImages() {
    return MAX(state.rows, 1) * MAX(state.cols, 1);
}
static inline int getRows() {
    return MAX(state.rows, 1);
}

static inline int getCols() {
    return MAX(state.cols, 1);
}

static inline const char* getFilePath(int index) {
    return index < 0 || index >= state.num_files ? NULL : state.file_names[index];
}

#endif
