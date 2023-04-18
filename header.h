#ifndef HEADER_H
#define HEADER_H
#include <gst/gst.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

typedef struct _HostCustomData {
    GstElement *pipeline;
    GstElement *source;
    GstElement *demuxer;
    GstElement *video_decoder;
    GstElement *video_queue;
    GstElement *video_convert;
    GstElement *video_encoder;
    GstElement *rtp_payload;
    GstElement *udp_sink;
    GMainLoop *loop;
}HostCustomData;

typedef struct _RemoteCustomData {
    GstElement *pipeline;
    GstElement *udp_source;
    GstElement *caps_filter;
    GstElement *rtp_depay;
    GstElement *video_queue;
    GstElement *video_decoder;
    GstElement *video_sink;
    GMainLoop *loop;
}RemoteCustomData;

extern int localhost_pipeline (int, char *[]);

extern int remotehost_pipeline (int, char *[]);

#endif