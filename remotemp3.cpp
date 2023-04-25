#include "header.h"

static void callback_message (GstBus *bus, GstMessage *msg, RemoteMp3 *data) {

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
    case GST_MESSAGE_CLOCK_LOST:
      /* Get a new clock */
      gst_element_set_state (data->pipeline, GST_STATE_PAUSED);
      gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
      break;
    default: 
    break;
  }
}

int remotehost_Mp3_pipeline (int argc, char *argv[]) {
    GstBus *bus;
    GstStateChangeReturn ret;
    RemoteMp3 remote_host;
    GstCaps *audio_caps = NULL;

    /* Initialize RemoteHost structure */
    memset(&remote_host, 0, sizeof(remote_host));

    /* Initialize gstreamer */
    gst_init (NULL, NULL);
    
    /* Initialize gstremer elements */
    remote_host.pipeline = gst_pipeline_new("Remote-host-mp3");
    remote_host.udp_source = gst_element_factory_make("udpsrc", NULL);
    remote_host.rtp_depay = gst_element_factory_make("rtpmpadepay", NULL);
    remote_host.parser = gst_element_factory_make("mpegaudioparse", NULL);
    remote_host.audio_decoder = gst_element_factory_make("avdec_mp3", NULL);
    remote_host.queue = gst_element_factory_make("queue", NULL);
    remote_host.audio_convert = gst_element_factory_make("audioconvert", NULL);
    remote_host.audio_sink = gst_element_factory_make("autoaudiosink", NULL);

    /* Check if the elements are created */ 
    if (!remote_host.pipeline || !remote_host.udp_source || !remote_host.rtp_depay || !remote_host.parser ||
        !remote_host.audio_decoder || !remote_host.queue || !remote_host.audio_convert || !remote_host.audio_sink) {
            g_printerr ("Not all video elements could be created.\n");
            exit(EXIT_FAILURE);
    }
    
    /* Add elements to bin */ 
    gst_bin_add_many(GST_BIN(remote_host.pipeline),remote_host.udp_source, remote_host.rtp_depay, 
                            remote_host.parser, remote_host.queue, remote_host.audio_decoder,
                            remote_host.audio_convert, remote_host.audio_sink, NULL);

    /* Set the Capability */
    audio_caps = gst_caps_new_simple("application/x-rtp", 
                                     "media", G_TYPE_STRING, "audio",
                                     "encoding-name", G_TYPE_STRING, "MPA",
                                     "payload", G_TYPE_INT, 96,
                                     "clock-rate", G_TYPE_INT, 9000 ,NULL);

    /* Set the element properties */
    g_object_set(G_OBJECT(remote_host.udp_source), "caps", audio_caps,
                                                         "port", 5000, NULL);

    /* Link the elements */
    if(gst_element_link_many(remote_host.udp_source, remote_host.rtp_depay, remote_host.parser,
                             remote_host.audio_decoder, remote_host.queue, remote_host.audio_convert,
                             remote_host.audio_sink, NULL) != TRUE) {
        g_printerr("Audio elements not linked.\n");
        exit(EXIT_FAILURE);
    }

    /* Set the pipeline to playing state */
    ret = gst_element_set_state(remote_host.pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Colud not set the pipeline to playing state.\n");
        gst_object_unref (remote_host.pipeline);
    }

    /* Watch the pipeline adding to the bus */
    bus = gst_element_get_bus(remote_host.pipeline);
    gst_bus_add_signal_watch(bus);

    /* Connect signal messages that came from bus */
    g_signal_connect(bus, "message", G_CALLBACK(callback_message), &remote_host);
    // g_signal_connect(bus, "message::error", G_CALLBACK(callback_message), &remote_host);
    // g_signal_connect(bus, "message::state-changed", G_CALLBACK(callback_message), &remote_host);

    /* Start the Main event loop */
    remote_host.loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(remote_host.loop);

    gst_element_set_state(remote_host.pipeline, GST_STATE_NULL);
    gst_object_unref(remote_host.pipeline);
    gst_object_unref(bus);
    g_main_loop_unref(remote_host.loop);
    /* Unref caps */
    gst_caps_unref(audio_caps);

    return 0;
}