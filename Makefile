CXX = g++-4.9
CXXFLAGS = -Ofast -g -Wall -std=c++11
AV_CFLAGS = `pkg-config --cflags libavformat libavcodec libavutil`
AV_LIBS = `pkg-config --libs libavformat libavcodec libavutil`
AO_CFLAGS = `pkg-config --cflags ao`
AO_LIBS = `pkg-config --libs ao`
GTKMM_CFLAGS = `pkg-config --cflags gtkmm-3.0`
GTKMM_LIBS = `pkg-config --libs gtkmm-3.0`

PROG = aplayer mkwf apicker
all: $(PROG)

areader.o: areader.h
areader.o: CXXFLAGS += $(AV_CFLAGS)
aplayer.o: aplayer.h areader.h hipass.h
aplayer.o: CXXFLAGS += $(AO_CFLAGS)
aplayer: aplayer.o areader.o hipass.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(AV_LIBS) $(AO_LIBS)

hipass.o: hipass.h sample.h
mkwf.o: waveformgen.h rms.h hipass.h
mkwf: mkwf.o hipass.o areader.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(AV_LIBS)

cursor.o: cursor.h
cursor.o: CXXFLAGS += $(GTKMM_CFLAGS)

apicker.o: waveform.h waveformview.h overviewbar.h csec.h
apicker.o: CXXFLAGS += $(GTKMM_CFLAGS)
apicker: apicker.o waveform.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(AV_LIBS) $(GTKMM_LIBS)
