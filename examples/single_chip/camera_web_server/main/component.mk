#
# Main Makefile. This is basically the same as a component makefile.
#
# This Makefile should, at the very least, just include $(SDK_PATH)/make/component.mk. By default, 
# this will take the sources in the src/ directory, compile them and link them into 
# lib(subdirectory_name).a in the build directory. This behaviour is entirely configurable,
# please read the SDK documents if you need to do this.
#
COMPONENT_EMBED_FILES := www/index_ov2640.html.gz
COMPONENT_EMBED_FILES += www/index_ov3660.html.gz
COMPONENT_EMBED_FILES += www/index_ov5640.html.gz
COMPONENT_EMBED_FILES += www/monitor.html.gz

COMPONENT_ADD_INCLUDEDIRS += include
COMPONENT_ADD_INCLUDEDIRS += eng/certs
COMPONENT_ADD_INCLUDEDIRS += eng/dev_model
COMPONENT_ADD_INCLUDEDIRS += eng/dev_sign
COMPONENT_ADD_INCLUDEDIRS += eng/infra
COMPONENT_ADD_INCLUDEDIRS += eng/mqtt
COMPONENT_ADD_INCLUDEDIRS += eng/wrappers
COMPONENT_ADD_INCLUDEDIRS += eng/eng
COMPONENT_ADD_INCLUDEDIRS += eng/wrappers/ubuntu


COMPONENT_SRCDIRS += src
COMPONENT_SRCDIRS += eng/dev_sign
COMPONENT_SRCDIRS += eng/infra
COMPONENT_SRCDIRS += eng/wrappers
COMPONENT_SRCDIRS += eng/mqtt
COMPONENT_SRCDIRS += eng/dev_model
COMPONENT_SRCDIRS += eng/certs
