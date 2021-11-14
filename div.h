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

#define RUN_EVENT(X, ...) do {if(events[X])events[X](__VA_ARGS__);}while(0)
typedef struct {
    uint8_t  mod;
    uint32_t keysym;
    void (*func)();
    int arg;
    uint8_t keycode;
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
    RENDER,
    SET_TITLE,

    OPEN_IMAGES,
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
const char** defaultSingleArgParse(const char **argv);
void render();

void maybe_render();

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
} ImageInfo;

typedef struct State {
    int num_active_images;
    const char** file_names;
    uint16_t num_files;

    uint32_t file_index;
    uint32_t rows;
    uint32_t cols;
    ScaleMode scale_mode;
    AlignMode align_mode_x;
    AlignMode align_mode_y;
    float zoom;
    bool right_to_left;

    uint16_t ignore_mask;
    uint32_t xevent_mask;
    uint32_t bg_color;
    uint32_t fg_color;
    const char* name;

    uint16_t padding_x;
    uint16_t padding_y;

    uint32_t parent;
    uint32_t wid;
    uint32_t drawable;
    uint16_t win_width;
    uint16_t win_height;
    uint16_t event_counter;
    bool dirty;

    void* image_context;
} State;


int addNewEventFD(int fd, short events, void (*callback)());
void removeExtraEvent(int index);


extern ImageInfo image_holders[];
extern State state;

static inline int getNumActiveImages() {
    return state.rows && state.cols ? state.rows * state.cols : state.num_active_images;
}
static inline int getRows() {
    return state.rows ? state.rows : state.cols ? (state.num_active_images + state.cols -1 ) / state.cols : 1;
}

static inline int getCols() {
    return state.cols ? state.cols : state.rows ? (state.num_active_images + state.rows - 1 ) / state.rows : state.num_active_images;
}

static inline const char* getFilePath(int index) {
    return index < 0 || index >= state.num_files ? NULL : state.file_names[index];
}

#endif
