
/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#define HTTP_PARSER_VERSION_MAJOR ...
#define HTTP_PARSER_VERSION_MINOR ...

#define HTTP_MAX_HEADER_SIZE ...


typedef struct http_parser http_parser;
typedef struct http_parser_settings http_parser_settings;


/* Callbacks should return non-zero to indicate an error. The parser will
 * then halt execution.
 *
 * The one exception is on_headers_complete. In a HTTP_RESPONSE parser
 * returning '1' from on_headers_complete will tell the parser that it
 * should not expect a body. This is used when receiving a response to a
 * HEAD request which may contain 'Content-Length' or 'Transfer-Encoding:
 * chunked' headers that indicate the presence of a body.
 *
 * http_data_cb does not return data chunks. It will be call arbitrarally
 * many times for each string. E.G. you might get 10 callbacks for "on_url"
 * each providing just a few characters more data.
 */
typedef int (*http_data_cb) (http_parser*, const char *at, size_t length);
typedef int (*http_cb) (http_parser*);

enum http_method {
  HTTP_DELETE,
  HTTP_GET,
  HTTP_HEAD,
  HTTP_POST,
  HTTP_PUT,
  HTTP_CONNECT,
  HTTP_OPTIONS,
  HTTP_TRACE,
  HTTP_COPY,
  HTTP_LOCK,
  HTTP_MKCOL,
  HTTP_MOVE,
  HTTP_PROPFIND,
  HTTP_PROPPATCH,
  HTTP_SEARCH,
  HTTP_UNLOCK,
  HTTP_REPORT,
  HTTP_MKACTIVITY,
  HTTP_CHECKOUT,
  HTTP_MERGE,
  HTTP_MSEARCH,
  HTTP_NOTIFY,
  HTTP_SUBSCRIBE,
  HTTP_UNSUBSCRIBE,
  HTTP_PATCH,
  HTTP_PURGE,
};


enum http_parser_type { HTTP_REQUEST, HTTP_RESPONSE, HTTP_BOTH };


/* Flag values for http_parser.flags field */
enum flags {
  F_CHUNKED = ...,
  F_CONNECTION_KEEP_ALIVE = ...,
  F_CONNECTION_CLOSE = ...,
  F_TRAILING = ...,
  F_UPGRADE = ...,
  F_SKIPBODY = ...,
};


enum http_errno {
  HPE_OK = ...,
  HPE_CB_message_begin = ...,
  HPE_CB_status_complete = ...,
  HPE_CB_url = ...,
  HPE_CB_header_field = ...,
  HPE_CB_header_value = ...,
  HPE_CB_headers_complete = ...,
  HPE_CB_body = ...,
  HPE_CB_message_complete = ...,
  HPE_INVALID_EOF_STATE = ...,
  HPE_HEADER_OVERFLOW = ...,
  HPE_CLOSED_CONNECTION = ...,
  HPE_INVALID_VERSION = ...,
  HPE_INVALID_STATUS = ...,
  HPE_INVALID_METHOD = ...,
  HPE_INVALID_URL = ...,
  HPE_INVALID_HOST = ...,
  HPE_INVALID_PORT = ...,
  HPE_INVALID_PATH = ...,
  HPE_INVALID_QUERY_STRING = ...,
  HPE_INVALID_FRAGMENT = ...,
  HPE_LF_EXPECTED = ...,
  HPE_INVALID_HEADER_TOKEN = ...,
  HPE_INVALID_CONTENT_LENGTH = ...,
  HPE_INVALID_CHUNK_SIZE = ...,
  HPE_INVALID_CONSTANT = ...,
  HPE_INVALID_INTERNAL_STATE = ...,
  HPE_STRICT = ...,
  HPE_PAUSED = ...,
  HPE_UNKNOWN = ...,
  ...
};



/* Get an http_errno value from an http_parser */
// #define HTTP_PARSER_ERRNO(p)            ((enum http_errno) (p)->http_errno)


struct http_parser {
  /** PRIVATE **/
  unsigned char type : 2;     /* enum http_parser_type */
  unsigned char flags : 6;    /* F_* values from 'flags' enum; semi-public */
  unsigned char state;        /* enum state from http_parser.c */
  unsigned char header_state; /* enum header_state from http_parser.c */
  unsigned char index;        /* index into current matcher */

  uint32_t nread;          /* # bytes read in various scenarios */
  uint64_t content_length; /* # bytes in body (0 if no Content-Length header) */

  /** READ-ONLY **/
  unsigned short http_major;
  unsigned short http_minor;
  unsigned short status_code; /* responses only */
  unsigned char method;       /* requests only */
  unsigned char http_errno : 7;

  /* 1 = Upgrade header was present and the parser has exited because of that.
   * 0 = No upgrade header present.
   * Should be checked when http_parser_execute() returns in addition to
   * error checking.
   */
  unsigned char upgrade : 1;

  /** PUBLIC **/
  void *data; /* A pointer to get hook to the "connection" or "socket" object */
};


struct http_parser_settings {
  http_cb      on_message_begin;
  http_data_cb on_url;
  http_cb      on_status_complete;
  http_data_cb on_header_field;
  http_data_cb on_header_value;
  http_cb      on_headers_complete;
  http_data_cb on_body;
  http_cb      on_message_complete;
};


enum http_parser_url_fields {
  UF_SCHEMA,
  UF_HOST,
  UF_PORT,
  UF_PATH,
  UF_QUERY,
  UF_FRAGMENT,
  UF_USERINFO,
  UF_MAX,
};


/* Result structure for http_parser_parse_url().
 *
 * Callers should index into field_data[] with UF_* values iff field_set
 * has the relevant (1 << UF_*) bit set. As a courtesy to clients (and
 * because we probably have padding left over), we convert any port to
 * a uint16_t.
 */
struct http_parser_url {
  uint16_t field_set;           /* Bitmask of (1 << UF_*) values */
  uint16_t port;                /* Converted UF_PORT string */

  struct {
    uint16_t off;               /* Offset into buffer in which field starts */
    uint16_t len;               /* Length of run in buffer */
  } field_data[7];
};


void http_parser_init(http_parser *parser, enum http_parser_type type);


size_t http_parser_execute(http_parser *parser,
                           const http_parser_settings *settings,
                           const char *data,
                           size_t len);


/* If http_should_keep_alive() in the on_headers_complete or
 * on_message_complete callback returns 0, then this should be
 * the last message on the connection.
 * If you are the server, respond with the "Connection: close" header.
 * If you are the client, close the connection.
 */
int http_should_keep_alive(const http_parser *parser);

/* Returns a string version of the HTTP method. */
const char *http_method_str(enum http_method m);

/* Return a string name of the given error */
const char *http_errno_name(enum http_errno err);

/* Return a string description of the given error */
const char *http_errno_description(enum http_errno err);

/* Parse a URL; return nonzero on failure */
int http_parser_parse_url(const char *buf, size_t buflen,
                          int is_connect,
                          struct http_parser_url *u);

/* Pause or un-pause the parser; a nonzero value pauses */
void http_parser_pause(http_parser *parser, int paused);

/* Checks if this is the final chunk of the body. */
int http_body_is_final(const http_parser *parser);
