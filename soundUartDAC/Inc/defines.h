#define PLAY 


#define BUFFER_SIZE 8192
#define REQUEST_NEW_DATA '1'

// 
typedef enum {
    IDLE,
    PLAYING,
    PAUSED, // not used
    STOPPED
} SystemState;

typedef enum {
    CMD_PING = '0',
    CMD_PLAY = '1',
    CMD_PAUSE = '2', // not used
    CMD_STOP = '3',
    CMD_UNKNOWN
} Command;