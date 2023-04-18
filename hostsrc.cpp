#include "header.h"

static void callback_message (GstBus *bus, GstMessage *msg, HostCustomData *data) {

  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR: {
        GError *err;
        gchar *debug;
        gst_message_parse_error (msg, &err, &debug);
        g_print ("Error: %s\n", err->message);
        g_error_free (err);
        g_free (debug);
        g_main_loop_quit (data->loop);
    }
    break;
    case GST_MESSAGE_EOS: {
        g_print("Reached End of Stream.\n");
        g_main_loop_quit (data->loop);
    }
     break;
    case GST_MESSAGE_STATE_CHANGED: {
      if (GST_MESSAGE_SRC(msg) == GST_OBJECT(data->pipeline)) {
        GstState old_state, new_state, pending_state;
        gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
        g_print("Pipeline state changed from '%s' to '%s'\n", gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));
      }
    }
    break;
    default: {
      g_printerr("Got unexpected messages.\n");
    }
    break;
  }
}

static void host_pad_handler (GstElement *src, GstPad *pad, HostCustomData *data) {
    GstPad *sink_pad = gst_element_get_static_pad(data->video_decoder, "sink");
    GstPadLinkReturn ret;
    GstCaps *new_pad_caps = NULL;
    GstStructure *new_pad_struct = NULL;
    const gchar *new_pad_type = NULL;

    g_print("\nReceived new pad '%s' from '%s'\n", GST_PAD_NAME(pad), GST_ELEMENT_NAME(src));
    
    new_pad_caps = gst_pad_get_current_caps (pad);
    new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
    new_pad_type = gst_structure_get_name (new_pad_struct);

    if (g_str_has_prefix (new_pad_type, "video/")) {
        
        ret = gst_pad_link (pad, sink_pad);
        if (GST_PAD_LINK_FAILED (ret)) {
            g_print ("Type is '%s' but link failed.\n", new_pad_type);
        } 
        else {
            g_print ("Link succeeded (type '%s').\n", new_pad_type);
        }
    }   
    
    if (new_pad_caps != NULL)
        gst_caps_unref (new_pad_caps);
    
    gst_object_unref (sink_pad);
} 

int localhost_pipeline (int argc, char *argv[]) {
    GstBus *bus;
    GstStateChangeReturn ret;
    HostCustomData server_data;

    // Initializing structure varilable to 0.
    memset(&server_data, 0, sizeof(server_data));

    // Initialize gstreamer
    gst_init(NULL, NULL);
   
    // Initilize elements 
    server_data.pipeline = gst_pipeline_new ("host-pipeline");
    server_data.source = gst_element_factory_make ("filesrc", NULL);
    server_data.demuxer = gst_element_factory_make ("qtdemux", NULL);
    server_data.video_decoder = gst_element_factory_make ("avdec_h264", NULL);
    server_data.video_queue = gst_element_factory_make ("queue", NULL);
    server_data.video_convert = gst_element_factory_make("videoconvert", NULL);
    server_data.video_encoder = gst_element_factory_make("x264enc", NULL);
    server_data.rtp_payload = gst_element_factory_make("rtph264pay", NULL);
    server_data.udp_sink = gst_element_factory_make("udpsink", NULL);

    // Check the elements are created 
    if (!server_data.pipeline || !server_data.source || !server_data.demuxer || 
        !server_data.video_decoder || !server_data.video_queue || !server_data.video_convert ||
        !server_data.video_encoder || !server_data.rtp_payload || !server_data.udp_sink) {
            g_printerr("Not all the elements could be created.\n");
            exit(EXIT_FAILURE);
    }

    // Add elements to bin
    gst_bin_add_many(GST_BIN(server_data.pipeline), server_data.source, server_data.demuxer,
                    server_data.video_decoder, server_data.video_queue, server_data.video_convert,
                    server_data.video_encoder, server_data.rtp_payload, server_data.udp_sink, NULL);

    // Set the element properties 
    g_object_set(G_OBJECT(server_data.source), "location", "/home/ee212798/gstreamer/songs/abc1.mp4", NULL);
    g_object_set(G_OBJECT(server_data.udp_sink), "host", "10.1.137.49",
                                                 "port", 5000,
                                                 "clients", "10.1.137.58:5000", NULL);

    // Link the elements 

    if (gst_element_link(server_data.source, server_data.demuxer) != TRUE) {
        g_printerr("Sorce to qtdemux not linked.\n");
        exit(EXIT_FAILURE);
    }

    if (gst_element_link_many(server_data.video_decoder, server_data.video_queue, server_data.video_convert,
            server_data.video_encoder, server_data.rtp_payload, server_data.udp_sink, NULL) != TRUE) {
                g_printerr("Decoder to udpsink not linked.\n");
                exit(EXIT_FAILURE);
        }

    /* Connect pad-added signal */
    g_signal_connect (server_data.demuxer, "pad-added", G_CALLBACK(host_pad_handler), &server_data);

    /* Set the pipeline for playing state */
    ret = gst_element_set_state(server_data.pipeline, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Could not set the pipeline for playing.\n");
        exit(EXIT_FAILURE);
    }

    bus = gst_element_get_bus(server_data.pipeline);
    gst_bus_add_signal_watch(bus);

    /* Connect signal messages that came from bus */
    g_signal_connect(bus, "message::eos", G_CALLBACK(callback_message), &server_data);
    g_signal_connect(bus, "message::error", G_CALLBACK(callback_message), &server_data);
    g_signal_connect(bus, "message::state-changed", G_CALLBACK(callback_message), &server_data);

    /* Start the Main Loop event */
    server_data.loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (server_data.loop);

    gst_element_set_state(server_data.pipeline, GST_STATE_NULL);
    gst_object_unref(server_data.pipeline);
    gst_object_unref(bus);
    g_main_loop_unref(server_data.loop);

    return 0;
}