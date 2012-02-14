
#define PNDR_FRAME_START    0x7E
    #define PNDR_FRAME_ESCAPE_START 0x5E
#define PNDR_FRAME_END  0x7C
    #define PNDR_FRAME_ESCAPE_END   0x5C
#define PNDR_FRAME_ESCAPE   0x7D
    #define PNDR_FRAME_ESCAPE_ESCAPE    0x5D
    
#define PNDR_FRAME_TYPE_DATA 0x00
#define PNDR_FRAME_TYPE_ACK 0x01

typedef struct {
    uint8 start;
    uint8 type;
    uint8 sequence;
    uint32 payload_length;
    uint8 *payload;
    uint32 crc;
    uint8 end;
}PandoraDataPkt;

typedef struct {
    uint8 start;
    uint8 type;
    uint8 sequence;
    uint32 payload_length;
    uint32 crc;
    uint8 end;
}PandoraAckPkt;

/* COMMAND */
/* Accessory to Mobile App */
#define PNDR_CONNECT    0x00
#define PNDR_UPDATE_BRANDING_IMAGE  0x01
#define PNDR_RETURN_BRANDING_IMAGE_SEGMENT  0x02
#define PNDR_GET_STATUS 0x03
#define PNDR_GET_NOTICE_TEXT    0x04
#define PNDR_GET_TRACK_INFO 0x10
#define PNDR_GET_TRACK_TITLE    0x11
#define PNDR_GET_TRACK_ARTIST   0x12
#define PNDR_GET_TRACK_ALBUM    0x13
#define PNDR_GET_TRACK_ALBUM_ART    0x14
#define PNDR_SET_TRACK_ELAPSED_POLLING  0x15
#define PNDR_EVENT_TRACK_PLAY    0x30
#define PNDR_EVENT_TRACK_PLAY    0x31
#define PNDR_EVENT_TRACK_SKIP    0x32
#define PNDR_EVENT_TRACK_RATE_POSITIVE    0x33
#define PNDR_EVENT_TRACK_RATE_NEGATIVE    0x34
#define PNDR_EVENT_TRACK_EXPLAIN    0x35
#define PNDR_GET_TRACK_EXPLAIN    0x36
#define PNDR_EVENT_TRACK_BOOKMARK_SONG    0x37
#define PNDR_EVENT_TRACK_BOOKMARK_ARTIST    0x38
#define PNDR_GET_STATION_ACTIVE    0x40
#define PNDR_GET_STATION_COUNT    0x41
#define PNDR_GET_STATION_TOKENS    0x42
#define PNDR_GET_ALL_STATION_TOKENS    0x43
#define PNDR_GET_STATION_INFO    0x44
#define PNDR_GET_STATIONS_ORDER    0x45
#define PNDR_EVENT_STATIONS_SORT    0x46
#define PNDR_EVENT_STATION_SELECT    0x47
#define PNDR_EVENT_STATION_DELETE    0x48
#define PNDR_EVENT_SEARCH_AUTO_COMPLETE    0x60
#define PNDR_EVENT_SEARCH_EXTENDED    0x61
#define PNDR_GET_SEARCH_RESULT_INFO    0x62
#define PNDR_EVENT_SEARCH_SELECT    0x63
#define PNDR_SEARCH_DISCARD    0x64
/* Mobile App to Accessory */
#define PNDR_GET_BRANDING_IMAGE    0x80
#define PNDR_UPDATE_STATUS    0x81
#define PNDR_RETURN_STATUS    0x82
#define PNDR_UPDATE_NOTICE    0x83
#define PNDR_RETURN_NOTICE_TEXT    0x84
#define PNDR_UPDATE_TRACK    0x90
#define PNDR_RETURN_TRACK_INFO    0x91
#define PNDR_RETURN_TRACK_TITLE    0x92
#define PNDR_RETURN_TRACK_ARTIST    0x93
#define PNDR_RETURN_TRACK_ALBUM    0x94
#define PNDR_RETURN_TRACK_ALBUM_ART_SEGMENT    0x95
#define PNDR_UPDATE_TRACK_ALBUM_ART    0x96
#define PNDR_UPDATE_TRACK_ELAPSED    0x97
#define PNDR_UPDATE_TRACK_RATING    0x98
#define PNDR_UPDATE_TRACK_EXPLAIN    0x99
#define PNDR_RETURN_TRACK_EXPLAIN_SEGMENT    0x9A
#define PNDR_UPDATE_TRACK_BOOKMARK_SONG    0x9B
#define PNDR_UPDATE_TRACK_BOOKMARK_ARTIST    0x9C
#define PNDR_RETURN_STATION_ACTIVE    0xB1
#define PNDR_RETURN_STATION_COUNT    0xB2
#define PNDR_RETURN_STATION_TOKENS    0xB3
#define PNDR_RETURN_STATION_INFO    0xB4
#define PNDR_RETURN_STATIONS_ORDER    0xB5
#define PNDR_UPDATE_STATIONS_ORDER    0xB6
#define PNDR_UPDATE_STATION_DELETED    0xB7
#define PNDR_UPDATE_STATION_ACTIVE    0xBA
#define PNDR_UPDATE_SEARCH    0xD0
#define PNDR_UPDATE_SEARCH    0xD1
#define PNDR_UPDATE_STATION_ADDED    0xD2
#define PNDR_ECHO_REQUEST    0x7F
#define PNDR_ECHO_RESPONSE    0xFF

/* enumaration */
typedef enum{
    PNDR_IMAGE_NONE = 0x00,
    PNDR_IMAGE_JPEG = 0x01,
    PNDR_IMAGE_PNG = 0x02,
    PNDR_IMAGE_RGB565 = 0x03
}album_art_type_t;

typedef enum{
    PNDR_CONNECT_FLAG_SIMULATE_BAD_CONNECTION = 0x01,
    PNDR_CONNECT_FLAG_PAUSE_ON_CONNECT = 0x02,
    PNDR_CONNECT_FLAG_SIMULATE_RANDOM_ERRORS = 0x04
}conn_flags_t;

typedef enum {
    PNDR_NOTICE_SKIP_LIMIT_REACHED = 0x00,
    PNDR_NOTICE_STATION_LIMIT_REACHED = 0x01,
    PNDR_NOTICE_ERROR_TRACK_RATING = 0x02,
    PNDR_NOTICE_ERROR_STATION_DELETE = 0x03,
    PNDR_NOTICE_ERROR_SEARCH_EXTENDED = 0x04,
    PNDR_NOTICE_ERROR_SEARCH_SELECT = 0x05,
    PNDR_NOTICE_ERROR_BOOKMARK = 0x06,
    PNDR_NOTICE_ERROR_MAINTENANCE = 0x07
}pndr_notice_code_t;

typedef enum {
    PNDR_FALSE = 0x00,
    PNDR_TRUE = 0x01
}enabled_t;

typedef enum {
    PNDR_SORT_BY_DATE = 0x00,
    PNDR_SORT_BY_NAME = 0x01
}sort_order_t;

typedef enum {
    PNDR_RATING_NONE = 0x00,
    PNDR_RATING_POSITIVE = 0x01,
    PNDR_RATING_NEGATIVE =0x02
}rating_t;

typedef enum {
    PNDR_TRACK_FLAG_ALLOW_RATING = 0x01,
    PNDR_TRACK_FLAG_ALLOW_SKIP = 0x02, 
    PNDR_TRACK_FLAG_ALLOW_BOOKMARK = 0x04,
    PNDR_TRACK_FLAG_ALLOW_EXPLAIN = 0x08
}flags_t;

typedef enum {
    PNDR_STATUS_PLAYING = 0x01,
    PNDR_STATUS_PAUSED = 0x02,
    PNDR_STATUS_INCOMPATIBLE_API_VERSION = 0x03,
    PNDR_STATUS_SEE_DEVICE = 0x04,
    PNDR_STATUS_NO_STATIONS = 0x05,
    PNDR_STATUS_NO_STATION_ACTIVE = 0x06
}status_code_t;

/* structure */
#define ACCESSORY2MOBILE

typedef struct {
    uint8 command; /* 0x00 */
    uint16 api_version; /* 0x00 0x01 */
    uint8 accessory_id[8];
    uint16 album_art_dimension;
    uint8 album_art_type; /* album_art_type_t */
    uint8 conn_flags; /* conn_flags_t */
}PNDR_CONNECT_t;

typedef struct {
    uint8 command; /* 0x01 */
    uint32 image_length;
}PNDR_UPDATE_BRANDING_IMAGE_t;

typedef struct {
    uint8 command; /* 0x02 */
    uint8 segment_index;
    uint8 total_segments;
    uint8 bytes[0];
}PNDR_RETURN_BRANDING_IMAGE_SEGMENT_t;

typedef struct {
    uint8 command; /* 0x03 */
}PNDR_GET_STATUS_t;

typedef struct {
    uint8 command; /* 0x04 */
    uint8 pndr_notice_code; /* pndr_notice_code_t */
}PNDR_GET_NOTICE_TEXT_t;

typedef struct {
    uint8 command; /* 0x10 */
}PNDR_GET_TRACK_INFO_t;

typedef struct {
    uint8 command; /* 0x11 */
}PNDR_GET_TRACK_TITLE_t;

typedef struct {
    uint8 command; /* 0x12 */
}PNDR_GET_TRACK_ARTIST_t;

typedef struct {
    uint8 command; /* 0x13 */
}PNDR_GET_TRACK_ALBUM_t;

typedef struct {
    uint8 command; /* 0x14 */
    uint32 max_payload_length;
}PNDR_GET_TRACK_ALBUM_ART_t;

typedef struct {
    uint8 command; /* 0x15 */
    uint8 enabled; /* enabled_t */
}PNDR_SET_TRACK_ELAPSED_POLLING_t;

typedef struct {
    uint8 command; /* 0x30 */
}PNDR_EVENT_TRACK_PLAY_t;

typedef struct {
    uint8 command; /* 0x31 */
}PNDR_EVENT_TRACK_PLAY_t;

typedef struct {
    uint8 command; /* 0x32 */
}PNDR_EVENT_TRACK_SKIP_t;

typedef struct {
    uint8 command; /* 0x33 */
}PNDR_EVENT_TRACK_RATE_POSITIVE_t;

typedef struct {
    uint8 command; /* 0x34 */
}PNDR_EVENT_TRACK_RATE_NEGATIVE_t;

typedef struct {
    uint8 command; /* 0x35 */
}PNDR_EVENT_TRACK_EXPLAIN_t;

typedef struct {
    uint8 command; /* 0x36 */
    uint32 max_payload_length;
}PNDR_GET_TRACK_EXPLAIN_t;

typedef struct {
    uint8 command; /* 0x37 */
}PNDR_EVENT_TRACK_BOOKMARK_SONG_t;

typedef struct {
    uint8 command; /* 0x38 */
}PNDR_EVENT_TRACK_BOOKMARK_ARTIST_t;

typedef struct {
    uint8 command; /* 0x40 */
}PNDR_GET_STATION_ACTIVE_t;

typedef struct {
    uint8 command; /* 0x41 */
}PNDR_GET_STATION_COUNT_t;

typedef struct {
    uint8 command; /* 0x42 */
    uint8 start_index;
    uint8 count;
}PNDR_GET_STATION_TOKENS_t;

typedef struct {
    uint8 command; /* 0x43 */
}PNDR_GET_ALL_STATION_TOKENS_t;

typedef struct {
    uint8 command; /* 0x44 */
    uint32 station_token[];
}PNDR_GET_STATION_INFO_t;

typedef struct {
    uint8 command; /* 0x45 */
}PNDR_GET_STATIONS_ORDER_t;

typedef struct {
    uint8 command; /* 0x46 */
    uint8 sort_order; /* sort_order_t */
}PNDR_EVENT_STATIONS_SORT_t;

typedef struct {
    uint8 command; /* 0x47 */
    uint32 station_token;
}PNDR_EVENT_STATION_SELECT_t;

typedef struct {
    uint8 command; /* 0x48 */
    uint32 station_token;
}PNDR_EVENT_STATION_DELETE_t;

typedef struct {
    uint8 command; /* 0x60 */
    uint32 search_id;
    uint32 search_input[248];
}PNDR_EVENT_SEARCH_AUTO_COMPLETE_t;

typedef struct {
    uint8 command; /* 0x61 */
    uint32 search_id;
    uint32 search_input[248];
}PNDR_EVENT_SEARCH_EXTENDED_t;

typedef struct {
    uint8 command; /* 0x62 */
    uint32 search_id;
    uint32 music_tokens[];
}PNDR_GET_SEARCH_RESULT_INFO_t;

typedef struct {
    uint8 command; /* 0x63 */
    uint32 search_id;
    uint32 music_token;
}PNDR_EVENT_SEARCH_SELECT_t;

typedef struct {
    uint8 command; /* 0x64 */
    uint32 search_id;
}PNDR_SEARCH_DISCARD_t;

#define MOBILE2ACCESSORY

typedef struct {
    uint8 command; /* 0x80 */
}PNDR_GET_BRANDING_IMAGE_t;

typedef struct {
    uint8 command; /* 0x81 */\
    uint8 status_code; /* status_code_t */
}PNDR_UPDATE_STATUS_t;

typedef struct {
    uint8 command; /* 0x82 */\
    uint8 status; /* status_code_t */
}PNDR_RETURN_STATUS_t;

typedef struct {
    uint8 command; /* 0x83 */
    uint8 code; /* pndr_notice_code_t */
}PNDR_UPDATE_NOTICE_t;

typedef struct {
    uint8 command; /* 0x84 */
    uint8 code; /* pndr_notice_code_t */
    uint8 text[248];
}PNDR_RETURN_NOTICE_TEXT_t;

typedef struct {
    uint8 command; /* 0x90 */
    uint32 track_token;
}PNDR_UPDATE_TRACK_t;

typedef struct {
    uint8 command; /* 0x91 */
    uint32 track_token;
    uint32 album_art_length;
    uint16 duration;
    uint16 elapsed;
    uint8 rating; /* rating_t */
    uint8 flags; /* flags_t */    
}PNDR_RETURN_TRACK_INFO_t;

typedef struct {
    uint8 command; /* 0x92 */
    uint32 track_token;
    uint8 track_title[248];
}PNDR_RETURN_TRACK_TITLE_t;

typedef struct {
    uint8 command; /* 0x93 */
    uint32 track_token;
    uint8 artist_name[248];
}PNDR_RETURN_TRACK_ARTIST_t;

typedef struct {
    uint8 command; /* 0x94 */
    uint32 track_token;
    uint8 album_name[248];
}PNDR_RETURN_TRACK_ALBUM_t;


typedef struct {
    uint8 command; /* 0x95 */
    uint32 track_token;
    uint8 segment_index;
    uint8 total_segments;
    uint8 data[];
}PNDR_RETURN_TRACK_ALBUM_ART_SEGMENT_t;

typedef struct {
    uint8 command; /* 0x96 */
    uint32 track_token;
    uint32 image_length;
}PNDR_UPDATE_TRACK_ALBUM_ART_t;

typedef struct {
    uint8 command; /* 0x97 */
    uint32 track_token;
    uint16 elapsed;
}PNDR_UPDATE_TRACK_ELAPSED_t;

typedef struct {
    uint8 command; /* 0x98 */
    uint32 track_token;
    uint16 rating; /* rating_t */
}PNDR_UPDATE_TRACK_RATING_t;

typedef struct {
    uint8 command; /* 0x99 */
    uint32 track_token;
    uint32 explain_length;
}PNDR_UPDATE_TRACK_EXPLAIN_t;

typedef struct {
    uint8 command; /* 0x9A */
    uint32 track_token;
    uint8 segment_index;
    uint8 total_segments;
    uint8 data[];
}PNDR_RETURN_TRACK_EXPLAIN_SEGMENT_t;

typedef struct {
    uint8 command; /* 0x9B */
    uint32 track_token;
    uint8 is_bookmarked; /* enabled_t */
}PNDR_UPDATE_TRACK_BOOKMARK_SONG_t;

typedef struct {
    uint8 command; /* 0x9C */
    uint32 track_token;
    uint8 is_bookmarked; /* enabled_t */
}PNDR_UPDATE_TRACK_BOOKMARK_ARTIST_t;

typedef struct {
    uint8 command; /* 0xB1 */
    uint32 station_token;
}PNDR_RETURN_STATION_ACTIVE_t;

typedef struct {
    uint8 command; /* 0xB2 */
    uint8 count;
}PNDR_RETURN_STATION_COUNT_t;

typedef struct {
    uint8 command; /* 0xB3 */
    uint8 start_index;
    uint32 tokens[];
}PNDR_RETURN_STATION_TOKENS_t;

/* define later */

typedef struct {
    uint8 command; /* 0xB5 */
    uint8 order; /* sort_order_t */
}PNDR_RETURN_STATIONS_ORDER_t;

typedef struct {
    uint8 command; /* 0xB6 */
    uint8 order; /* sort_order_t */
}PNDR_UPDATE_STATIONS_ORDER_t;

typedef struct {
    uint8 command; /* 0xB7 */
    uint32 station_token;
}PNDR_UPDATE_STATION_DELETED_t;

typedef struct {
    uint8 command; /* 0xBA */
    uint32 station_token;
}PNDR_UPDATE_STATION_ACTIVE_t;

typedef struct {
    uint8 command; /* 0xD0 */
    uint32 search_id;
    uint32 music_tokens[];
}PNDR_UPDATE_SEARCH_t;

/* define later */

typedef struct {
    uint8 command; /* 0xD2 */
    uint32 station_token;
    uint8 index;
}PNDR_UPDATE_STATION_ADDED_t;

typedef struct {
    uint8 command; /* 0x7F */
    uint8 data[100];
}PNDR_ECHO_REQUEST_t;

typedef struct {
    uint8 command; /* 0xFF */
    uint8 data[100];
}PNDR_ECHO_RESPONSE_t;


