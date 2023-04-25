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

typedef struct _RemoteMp4 {
    GstElement *pipeline;
    GstElement *udp_source;
    GstElement *caps_filter;
    GstElement *rtp_depay;
    GstElement *video_queue;
    GstElement *video_decoder;
    GstElement *video_sink;

    GstElement *udp_audio_source;
    GstElement *audio_rtp_buffer;
    GstElement *audio_rtp_depay;
    GstElement *audio_decoder;
    GstElement *audio_queue;
    GstElement *audio_convert;
    GstElement *audio_sink;
    gboolean is_live;
    GMainLoop *loop;
}RemoteMp4;

typedef struct _RemoteMp3 {
    GstElement *pipeline;
    GstElement *udp_source;
    GstElement *rtp_depay;
    GstElement *parser;
    GstElement *audio_decoder;
    GstElement *queue;
    GstElement *audio_convert;
    GstElement *audio_sink;
    GMainLoop *loop;
}RemoteMp3;

typedef struct _RemoteWebM {
    GstElement *pipeline;
    GstElement *udp_video_source;
    GstElement *video_rtp_buffer;
    GstElement *video_rtp_depay;
    GstElement *video_queue;
    GstElement *video_decoder;
    GstElement *video_convert;
    GstElement *video_sink;

    GstElement *udp_audio_source;
    GstElement *audio_rtp_buffer;
    GstElement *audio_rtp_depay;
    GstElement *audio_decoder;
    GstElement *audio_queue;
    GstElement *audio_convert;
    GstElement *audio_sink;
    GMainLoop *loop;
}RemoteWebM;

typedef struct _RemoteAvi {
    GstElement *pipeline;
    GstElement *udp_video_source;
    GstElement *video_rtp_buffer;
    GstElement *video_rtp_depay;
    GstElement *video_queue;
    GstElement *video_decoder;
    GstElement *video_convert;
    GstElement *video_sink;

    GstElement *udp_audio_source;
    GstElement *audio_rtp_buffer;
    GstElement *audio_rtp_depay;
    GstElement *audio_decoder;
    GstElement *audio_queue;
    GstElement *audio_convert;
    GstElement *audio_sink;
    GMainLoop *loop;
}RemoteAvi;


extern int localhost_pipeline (int, char *[]);

extern int remotehost_Mp4_pipeline (int, char *[]);

extern int remotehost_Mp3_pipeline (int, char *[]);

extern int remotehost_WebM_pipeline (int, char *[]);

extern int remotehost_Avi_pipeline (int, char *[]);


#endif