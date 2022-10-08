#ifndef DIV
#define DIV

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


#define LEN(A) (sizeof(A)/sizeof(A[0]))
#define MIN(A,B) (A<B?A:B)
#define MAX(A,B) (A>B?A:B)
#define MAX_FILE_NAMES 255
#define MAX_IMAGES 16
#define MAX_FDS 8
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

// The default bindings
extern Binding bindings[];
// User specified bindings that override the defaults in case of conflicts
extern Binding user_bindings[];
// used to display help text
extern const char* arg_string;

// points to the original arguments passed into main; Useful if one wanted to restart
extern const char** initial_args;
extern int initial_num_args;

enum {
    ON_STARTUP = 32,  /* Very first hook run */
    PROCESS_ARGS,     /* Used to process command line args*/
    IMG_INIT,         /* Used to setup an image context if needed */
    PRE_MAP_WINDOW,   /* Called before the window is mapped */
    POST_XCONNECTION, /* Called after the window has been mapped and bindings grabbed */

    /* These hooks are triggered more than once */
    OPEN_IMAGES,      /* Used to actually open the image and update image_holders */
    RENDER,           /* Called to draw the image to the screen */
    SET_TITLE,        /* Called to set the window title */
    POST_EVENT,       /* Run after every batch of X events */
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

// An array of function pointers of size LAST_EVENT
extern void (*events[])();
// Used to scale an image up/down
extern void (*scaleFunc)(const char* buf, uint32_t original_width, uint32_t original_height, char* out_buf, uint32_t width, uint32_t height, int num_channels);
const char** defaultSingleArgParse(const char **argv, bool* stop);
// The default render function that just draws to the screen
void render();

// Default function used to load images. It should use the global state to know which image indexes to load
// and update image_holders and possibly state.num_files
void defaultOpenImages();

typedef struct {
    short x;
    short y;
    uint16_t width;
    uint16_t height;
} Geometry;

typedef struct {
    uint32_t render_count;
    uint16_t width;
    uint16_t height;
} CacheInfo;

typedef struct {
    const char* name;
    uint32_t image_width;
    uint32_t image_height;
    void* image_data;
    void* raw;
    void* scaled_cached_image_data;
    CacheInfo cache_info;

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
    bool wrap;

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

    uint32_t root;
    uint32_t parent;
    uint32_t wid;
    uint32_t drawable;
    uint32_t depth;
    uint16_t win_width;
    uint16_t win_height;

    uint16_t render_count;

    void* image_context;
} State;


// Add a new FD to listen to. Everytime this fd has some event (specified by events), callback is called
void addNewEventFD(int fd, short events, void (*callback)());
// Removes an existing fd
void removeExtraEventByFd(int fd);


// An array of size MAX_IMAGES that holds info about the loaded images
extern ImageInfo image_holders[];
// Holds all global state
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
