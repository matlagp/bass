import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst

def make_element(name):
    return Gst.ElementFactory.make(name)

def add_to_pipeline(pipeline, elements):
    for element in elements:
        pipeline.add(element)

def link(elements):
    for i in range(len(elements) - 1):
        elements[i].link(elements[i+1])

def link_tee(tee, element):
    src = tee.get_request_pad('src_%u')
    sink = element.get_static_pad('sink')
    src.link(sink)
    return src

def remove_from_pipeline(pipeline, elements):
    for element in elements:
        pipeline.remove(element)

def unlink(elements):
    for i in range(len(elements) - 1):
        elements[i].unlink(elements[i+1])

def unlink_tee(tee, tee_pad, element):
    tee_pad.unlink(element.get_static_pad('sink'))
    tee.release_request_pad(tee_pad)

def set_null_state(elements):
    for element in elements:
        element.set_state(Gst.State.NULL)

def unref(elements):
    for element in elements:
        element.unref()
