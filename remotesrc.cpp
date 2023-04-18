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

int remotehost_pipeline (int argc, char *argv[]) {
    GstBus *bus;
    GstStateChangeReturn ret;
    RemoteCustomData remote_host;
    GstCaps *caps = NULL;

    /* Initialize RemoteHost structure */
    memset(&remote_host, 0, sizeof(remote_host));

    /* Initialize gstreamer */
    gst_init (NULL, NULL);
    
    /* Initialize gstremer elements */
    remote_host.pipeline = gst_pipeline_new("Remote-host");
    remote_host.udp_source = gst_element_factory_make("udpsrc", NULL);
    remote_host.caps_filter = gst_element_factory_make("capsfilter", NULL);
    remote_host.rtp_depay = gst_element_factory_make("rtph264depay", NULL);
    remote_host.video_queue = gst_element_factory_make("queue", NULL);
    remote_host.video_decoder = gst_element_factory_make("avdec_h264", NULL);
    remote_host.video_sink = gst_element_factory_make("autovideosink", NULL);

    /* Check if elements are created */ 
    if (!remote_host.pipeline || !remote_host.udp_source || !remote_host.caps_filter || 
        !remote_host.rtp_depay || !remote_host.video_queue || !remote_host.video_decoder ||
        !remote_host.video_sink) {
            g_printerr ("Not all elements could be created.\n");
            exit(EXIT_FAILURE);
        }
    
    /* Add elements to bin */ 
    gst_bin_add_many(GST_BIN(remote_host.pipeline), remote_host.udp_source, remote_host.caps_filter,
                    remote_host.rtp_depay, remote_host.video_queue, remote_host.video_decoder, 
                    remote_host.video_sink, NULL);
                    
    /* Set the Capability */
    caps = gst_caps_new_simple("application/x-rtp", "encoding-name", "H264",
                                "payload", 96, NULL);
    
    /* Set the element properties */
    g_object_set(G_OBJECT(remote_host.caps_filter), "caps", caps, NULL);
    g_object_set(G_OBJECT(remote_host.udp_source), "port", 5000, NULL);

    /* Link the elements */
    if (gst_element_link_many(remote_host.udp_source, remote_host.caps_filter, remote_host.rtp_depay,
        remote_host.video_queue, remote_host.video_decoder, remote_host.video_sink, NULL) != TRUE) {
        g_printerr("Udpsource to sink elements not linked.\n");
        exit(EXIT_FAILURE);
    }

    /* Set the pipeline to playing state */
    ret = gst_element_set_state(remote_host.pipeline, GST_STATE_NULL);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Colud not set the pipeline to playing state.\n");
        exit(EXIT_FAILURE);
    }

    /* Watch the pipeline adding to the bus */
    bus = gst_element_get_bus(remote_host.pipeline);
    gst_bus_add_signal_watch(bus);

    /* Connect signal messages that came from bus */
    g_signal_connect(bus, "message::eos", G_CALLBACK(callback_message), &remote_host);
    g_signal_connect(bus, "message::error", G_CALLBACK(callback_message), &remote_host);
    g_signal_connect(bus, "message::state-changed", G_CALLBACK(callback_message), &remote_host);

    /* Start the Main event loop */
    remote_host.loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(remote_host.loop);

    gst_element_set_state(remote_host.pipeline, GST_STATE_NULL);
    gst_object_unref(remote_host.pipeline);
    gst_object_unref(bus);
    g_main_loop_unref(remote_host.loop);

    return 0;
}