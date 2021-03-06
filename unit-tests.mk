# unit tests
check_PROGRAMS += \
	ags_thread_test \
	ags_turtle_test \
	ags_audio_application_context_test \
	ags_devout_test \
	ags_audio_test \
	ags_channel_test \
	ags_recycling_test \
	ags_audio_signal_test \
	ags_recall_test \
	ags_port_test \
	ags_pattern_test \
	ags_notation_test \
	ags_automation_test \
	ags_midi_buffer_util_test \
	ags_xorg_application_context_test

# thread unit test
ags_thread_test_SOURCES = ags/test/thread/ags_thread_test.c
ags_thread_test_CFLAGS = $(CFLAGS) $(LIBXML2_CFLAGS) $(GOBJECT_CFLAGS)
ags_thread_test_LDFLAGS = -lcunit -lm -pthread -lrt $(LDFLAGS) $(LIBXML2_LIBS) $(GOBJECT_LIBS)
ags_thread_test_LDADD = libags_thread.la libags.la

# turtle unit test
ags_turtle_test_SOURCES = ags/test/lib/ags_turtle_test.c
ags_turtle_test_CFLAGS = $(CFLAGS) $(LIBAO_CFLAGS) $(LIBASOUND2_CFLAGS) $(LIBXML2_CFLAGS) $(SNDFILE_CFLAGS) $(LIBINSTPATCH_CFLAGS) $(GOBJECT_CFLAGS) $(JACK_CFLAGS)
ags_turtle_test_LDFLAGS = -pthread $(LDFLAGS)
ags_turtle_test_LDADD = libags_audio.la libags_server.la libags_gui.la libags_thread.la libags.la -lcunit -lrt -lm $(LIBAO_LIBS) $(LIBASOUND2_LIBS) $(LIBXML2_LIBS) $(SNDFILE_LIBS) $(LIBINSTPATCH_LIBS) $(GOBJECT_LIBS) $(JACK_LIBS)

# audio application context unit test
ags_audio_application_context_test_SOURCES = ags/test/audio/ags_audio_application_context_test.c
ags_audio_application_context_test_CFLAGS = $(CFLAGS) $(LIBAO_CFLAGS) $(LIBASOUND2_CFLAGS) $(LIBXML2_CFLAGS) $(SNDFILE_CFLAGS) $(LIBINSTPATCH_CFLAGS) $(GOBJECT_CFLAGS) $(JACK_CFLAGS)
ags_audio_application_context_test_LDFLAGS = $(LDFLAGS) -pthread
ags_audio_application_context_test_LDADD = libags_audio.la libags_server.la libags_gui.la libags_thread.la libags.la -lcunit -lrt -lm $(LIBAO_LIBS) $(LIBASOUND2_LIBS) $(LIBXML2_LIBS) $(SNDFILE_LIBS) $(LIBINSTPATCH_LIBS) $(GOBJECT_LIBS) $(JACK_LIBS)

# devout unit test
ags_devout_test_SOURCES = ags/test/audio/ags_devout_test.c
ags_devout_test_CFLAGS = $(CFLAGS) $(LIBAO_CFLAGS) $(LIBASOUND2_CFLAGS) $(LIBXML2_CFLAGS) $(SNDFILE_CFLAGS) $(LIBINSTPATCH_CFLAGS) $(GOBJECT_CFLAGS) $(JACK_CFLAGS)
ags_devout_test_LDFLAGS = -pthread $(LDFLAGS)
ags_devout_test_LDADD = libags_audio.la libags_server.la libags_gui.la libags_thread.la libags.la -lcunit -lrt -lm $(LIBAO_LIBS) $(LIBASOUND2_LIBS) $(LIBXML2_LIBS) $(SNDFILE_LIBS) $(LIBINSTPATCH_LIBS) $(GOBJECT_LIBS) $(JACK_LIBS)

# audio unit test
ags_audio_test_SOURCES = ags/test/audio/ags_audio_test.c
ags_audio_test_CFLAGS = $(CFLAGS) $(LIBAO_CFLAGS) $(LIBASOUND2_CFLAGS) $(LIBXML2_CFLAGS) $(SNDFILE_CFLAGS) $(LIBINSTPATCH_CFLAGS) $(GOBJECT_CFLAGS) $(JACK_CFLAGS)
ags_audio_test_LDFLAGS = -pthread $(LDFLAGS)
ags_audio_test_LDADD = libags_audio.la libags_server.la libags_gui.la libags_thread.la libags.la -lcunit -lrt -lm $(LIBAO_LIBS) $(LIBASOUND2_LIBS) $(LIBXML2_LIBS) $(SNDFILE_LIBS) $(LIBINSTPATCH_LIBS) $(GOBJECT_LIBS) $(JACK_LIBS)

# channel unit test
ags_channel_test_SOURCES = ags/test/audio/ags_channel_test.c
ags_channel_test_CFLAGS = $(CFLAGS) $(LIBAO_CFLAGS) $(LIBASOUND2_CFLAGS) $(LIBXML2_CFLAGS) $(SNDFILE_CFLAGS) $(LIBINSTPATCH_CFLAGS) $(GOBJECT_CFLAGS) $(JACK_CFLAGS)
ags_channel_test_LDFLAGS = -pthread $(LDFLAGS)
ags_channel_test_LDADD = libags_audio.la libags_server.la libags_gui.la libags_thread.la libags.la -lcunit -lrt -lm $(LIBAO_LIBS) $(LIBASOUND2_LIBS) $(LIBXML2_LIBS) $(SNDFILE_LIBS) $(LIBINSTPATCH_LIBS) $(GOBJECT_LIBS) $(JACK_LIBS)

# recycling unit test
ags_recycling_test_SOURCES = ags/test/audio/ags_recycling_test.c
ags_recycling_test_CFLAGS = $(CFLAGS) $(LIBAO_CFLAGS) $(LIBASOUND2_CFLAGS) $(LIBXML2_CFLAGS) $(SNDFILE_CFLAGS) $(LIBINSTPATCH_CFLAGS) $(GOBJECT_CFLAGS) $(JACK_CFLAGS)
ags_recycling_test_LDFLAGS = -pthread $(LDFLAGS)
ags_recycling_test_LDADD = libags_audio.la libags_server.la libags_gui.la libags_thread.la libags.la -lcunit -lrt -lm $(LIBAO_LIBS) $(LIBASOUND2_LIBS) $(LIBXML2_LIBS) $(SNDFILE_LIBS) $(LIBINSTPATCH_LIBS) $(GOBJECT_LIBS) $(JACK_LIBS)

# audio signal unit test
ags_audio_signal_test_SOURCES = ags/test/audio/ags_audio_signal_test.c
ags_audio_signal_test_CFLAGS = $(CFLAGS) $(LIBAO_CFLAGS) $(LIBASOUND2_CFLAGS) $(LIBXML2_CFLAGS) $(SNDFILE_CFLAGS) $(LIBINSTPATCH_CFLAGS) $(GOBJECT_CFLAGS) $(JACK_CFLAGS)
ags_audio_signal_test_LDFLAGS = -pthread $(LDFLAGS)
ags_audio_signal_test_LDADD = libags_audio.la libags_server.la libags_gui.la libags_thread.la libags.la -lcunit -lrt -lm $(LIBAO_LIBS) $(LIBASOUND2_LIBS) $(LIBXML2_LIBS) $(SNDFILE_LIBS) $(LIBINSTPATCH_LIBS) $(GOBJECT_LIBS) $(JACK_LIBS)

# recall unit test
ags_recall_test_SOURCES = ags/test/audio/ags_recall_test.c
ags_recall_test_CFLAGS = $(CFLAGS) $(LIBAO_CFLAGS) $(LIBASOUND2_CFLAGS) $(LIBXML2_CFLAGS) $(SNDFILE_CFLAGS) $(LIBINSTPATCH_CFLAGS) $(GOBJECT_CFLAGS) $(JACK_CFLAGS)
ags_recall_test_LDFLAGS = -pthread $(LDFLAGS)
ags_recall_test_LDADD = libags_audio.la libags_server.la libags_gui.la libags_thread.la libags.la -lcunit -lrt -lm $(LIBAO_LIBS) $(LIBASOUND2_LIBS) $(LIBXML2_LIBS) $(SNDFILE_LIBS) $(LIBINSTPATCH_LIBS) $(GOBJECT_LIBS) $(JACK_LIBS)

# port unit test
ags_port_test_SOURCES = ags/test/audio/ags_port_test.c
ags_port_test_CFLAGS = $(CFLAGS) $(LIBAO_CFLAGS) $(LIBASOUND2_CFLAGS) $(LIBXML2_CFLAGS) $(SNDFILE_CFLAGS) $(LIBINSTPATCH_CFLAGS) $(GOBJECT_CFLAGS) $(JACK_CFLAGS)
ags_port_test_LDFLAGS = -pthread $(LDFLAGS)
ags_port_test_LDADD = libags_audio.la libags_server.la libags_gui.la libags_thread.la libags.la -lcunit -lrt -lm $(LIBAO_LIBS) $(LIBASOUND2_LIBS) $(LIBXML2_LIBS) $(SNDFILE_LIBS) $(LIBINSTPATCH_LIBS) $(GOBJECT_LIBS) $(JACK_LIBS)

# pattern unit test
ags_pattern_test_SOURCES = ags/test/audio/ags_pattern_test.c
ags_pattern_test_CFLAGS = $(CFLAGS) $(LIBAO_CFLAGS) $(LIBASOUND2_CFLAGS) $(LIBXML2_CFLAGS) $(SNDFILE_CFLAGS) $(LIBINSTPATCH_CFLAGS) $(GOBJECT_CFLAGS) $(JACK_CFLAGS)
ags_pattern_test_LDFLAGS = -pthread $(LDFLAGS)
ags_pattern_test_LDADD = libags_audio.la libags_server.la libags_gui.la libags_thread.la libags.la -lcunit -lm -lrt $(LIBAO_LIBS) $(LIBASOUND2_LIBS) $(LIBXML2_LIBS) $(SNDFILE_LIBS) $(LIBINSTPATCH_LIBS) $(GOBJECT_LIBS) $(JACK_LIBS)

# notation unit test
ags_notation_test_SOURCES = ags/test/audio/ags_notation_test.c
ags_notation_test_CFLAGS = $(CFLAGS) $(LIBAO_CFLAGS) $(LIBASOUND2_CFLAGS) $(LIBXML2_CFLAGS) $(SNDFILE_CFLAGS) $(LIBINSTPATCH_CFLAGS) $(GOBJECT_CFLAGS) $(JACK_CFLAGS)
ags_notation_test_LDFLAGS = -pthread $(LDFLAGS)
ags_notation_test_LDADD = libags_audio.la libags_server.la libags_gui.la libags_thread.la libags.la -lcunit -lm -lrt $(LIBAO_LIBS) $(LIBASOUND2_LIBS) $(LIBXML2_LIBS) $(SNDFILE_LIBS) $(LIBINSTPATCH_LIBS) $(GOBJECT_LIBS) $(JACK_LIBS)

# automation unit test
ags_automation_test_SOURCES = ags/test/audio/ags_automation_test.c
ags_automation_test_CFLAGS = $(CFLAGS) $(LIBAO_CFLAGS) $(LIBASOUND2_CFLAGS) $(LIBXML2_CFLAGS) $(SNDFILE_CFLAGS) $(LIBINSTPATCH_CFLAGS) $(GOBJECT_CFLAGS) $(JACK_CFLAGS)
ags_automation_test_LDFLAGS = -pthread $(LDFLAGS)
ags_automation_test_LDADD = libags_audio.la libags_server.la libags_gui.la libags_thread.la libags.la -lcunit -lm -lrt $(LIBAO_LIBS) $(LIBASOUND2_LIBS) $(LIBXML2_LIBS) $(SNDFILE_LIBS) $(LIBINSTPATCH_LIBS) $(GOBJECT_LIBS) $(JACK_LIBS)

# midi buffer util unit test
ags_midi_buffer_util_test_SOURCES = ags/test/audio/midi/ags_midi_buffer_util_test.c
ags_midi_buffer_util_test_CFLAGS = $(CFLAGS) $(LIBAO_CFLAGS) $(LIBASOUND2_CFLAGS) $(LIBXML2_CFLAGS) $(SNDFILE_CFLAGS) $(LIBINSTPATCH_CFLAGS) $(GOBJECT_CFLAGS) $(JACK_CFLAGS)
ags_midi_buffer_util_test_LDFLAGS = -pthread $(LDFLAGS)
ags_midi_buffer_util_test_LDADD = libags_audio.la libags_server.la libags_gui.la libags_thread.la libags.la -lcunit -lm -lrt  $(LIBAO_LIBS) $(LIBASOUND2_LIBS) $(LIBXML2_LIBS) $(SNDFILE_LIBS) $(LIBINSTPATCH_LIBS) $(GOBJECT_LIBS) $(JACK_LIBS)

# xorg application context unit test
ags_xorg_application_context_test_SOURCES = ags/test/X/ags_xorg_application_context_test.c
ags_xorg_application_context_test_CFLAGS = $(CFLAGS) $(LIBAO_CFLAGS) $(LIBASOUND2_CFLAGS) $(LIBXML2_CFLAGS) $(SNDFILE_CFLAGS) $(LIBINSTPATCH_CFLAGS) $(GOBJECT_CFLAGS) $(JACK_CFLAGS) $(FONTCONFIG_CFLAGS) $(GDKPIXBUF_CFLAGS) $(CAIRO_CFLAGS) $(GTK_CFLAGS)
ags_xorg_application_context_test_LDFLAGS = -pthread $(LDFLAGS)
ags_xorg_application_context_test_LDADD = libgsequencer.la libags_audio.la libags_server.la libags_gui.la libags_thread.la libags.la -lcunit -lm -lrt $(LIBAO_LIBS) $(LIBASOUND2_LIBS) $(LIBXML2_LIBS) $(SNDFILE_LIBS) $(LIBINSTPATCH_LIBS) $(GOBJECT_LIBS) $(JACK_LIBS) $(FONTCONFIG_LIBS) $(GDKPIXBUF_LIBS) $(CAIRO_LIBS) $(GTK_LIBS)
