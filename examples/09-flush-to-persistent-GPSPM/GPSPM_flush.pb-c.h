/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: GPSPM_flush.proto */

#ifndef PROTOBUF_C_GPSPM_5fflush_2eproto__INCLUDED
#define PROTOBUF_C_GPSPM_5fflush_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1000000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003003 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _GPSPMFlushRequest GPSPMFlushRequest;
typedef struct _GPSPMFlushResponse GPSPMFlushResponse;


/* --- enums --- */


/* --- messages --- */

struct  _GPSPMFlushRequest
{
  ProtobufCMessage base;
  uint64_t offset;
  uint64_t length;
  uint64_t op_context;
};
#define GPSPM_FLUSH_REQUEST__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gpspm_flush_request__descriptor) \
    , 0, 0, 0 }


struct  _GPSPMFlushResponse
{
  ProtobufCMessage base;
  uint64_t op_context;
};
#define GPSPM_FLUSH_RESPONSE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gpspm_flush_response__descriptor) \
    , 0 }


/* GPSPMFlushRequest methods */
void   gpspm_flush_request__init
                     (GPSPMFlushRequest         *message);
size_t gpspm_flush_request__get_packed_size
                     (const GPSPMFlushRequest   *message);
size_t gpspm_flush_request__pack
                     (const GPSPMFlushRequest   *message,
                      uint8_t             *out);
size_t gpspm_flush_request__pack_to_buffer
                     (const GPSPMFlushRequest   *message,
                      ProtobufCBuffer     *buffer);
GPSPMFlushRequest *
       gpspm_flush_request__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gpspm_flush_request__free_unpacked
                     (GPSPMFlushRequest *message,
                      ProtobufCAllocator *allocator);
/* GPSPMFlushResponse methods */
void   gpspm_flush_response__init
                     (GPSPMFlushResponse         *message);
size_t gpspm_flush_response__get_packed_size
                     (const GPSPMFlushResponse   *message);
size_t gpspm_flush_response__pack
                     (const GPSPMFlushResponse   *message,
                      uint8_t             *out);
size_t gpspm_flush_response__pack_to_buffer
                     (const GPSPMFlushResponse   *message,
                      ProtobufCBuffer     *buffer);
GPSPMFlushResponse *
       gpspm_flush_response__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gpspm_flush_response__free_unpacked
                     (GPSPMFlushResponse *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*GPSPMFlushRequest_Closure)
                 (const GPSPMFlushRequest *message,
                  void *closure_data);
typedef void (*GPSPMFlushResponse_Closure)
                 (const GPSPMFlushResponse *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor gpspm_flush_request__descriptor;
extern const ProtobufCMessageDescriptor gpspm_flush_response__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_GPSPM_5fflush_2eproto__INCLUDED */
