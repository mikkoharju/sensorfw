TEMPLATE = subdirs
SUBDIRS = coordinatealignfilter \
          declinationfilter \
          orientationinterpreter \
          rotationfilter \
          downsamplefilter

include(../common-install.pri)
publicheaders.files = *.h
publicheaders.path = $${publicheaders.path}/filters

