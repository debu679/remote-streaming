#ifndef PROBE_H
#define PROBE_H

#include "header.h"
#include<bits/stdc++.h>

using namespace std;

GstPadProbeReturn probe_callback(GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
  GstEvent *event = GST_PAD_PROBE_INFO_EVENT(info);
  GstTagList *tag_list;

  if (GST_EVENT_TYPE(event) == GST_EVENT_SEGMENT) {
    const GstSegment *segment;
    gst_event_parse_segment(event, &segment);
    g_print("Segment: %" GST_SEGMENT_FORMAT "\n", GST_SEGMENT_FORMAT);
  } 
  else if (GST_EVENT_TYPE(event) == GST_EVENT_CAPS) {
    g_print("\n*..........Capabilties OF PAD:..........*\n");
    GstCaps *caps;
    gst_event_parse_caps(event, &caps);
    gchar *caps_string = gst_caps_to_string(caps);
    size_t pos = 0;
    string line;
    string cap = string(caps_string);
    while ((pos = cap.find(',')) != string::npos) {
      line = cap.substr(0, pos);
      cout << line <<endl;
      cap.erase(0, pos + 1);
    }
    cout << cap <<endl;
    // g_print("Caps: %s\n", caps_string);
    g_free(caps_string);
  }
    else if (GST_EVENT_TYPE(event) == GST_EVENT_TAG) {
    gst_event_parse_tag(event, &tag_list);
  }
  else {
    g_print("\n*..........EVENT..........*\n");
    if(GST_EVENT_TYPE_NAME(event)!= "tag"){
      g_print("Event: %s\n", GST_EVENT_TYPE_NAME(event));
    }
  }
  return GST_PAD_PROBE_OK;
}

#endif