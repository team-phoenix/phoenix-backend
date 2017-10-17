//#pragma once

//#include <QObject>
//#include <QtTest>

//#include "libretrolibrary.h"

//class Test_LibretroLibrary : public QObject {
//    Q_OBJECT

//    LibretroLibrary *library;

//private slots:

//    void init() {
//        library = new LibretroLibrary;
//    }

//    void cleanup() {
//        delete library;
//    }

//    void should_initializeCallbacksToNull() {
//        QVERIFY( library->retro_api_version == nullptr );
//        QVERIFY( library->retro_cheat_reset == nullptr );
//        QVERIFY( library->retro_cheat_set == nullptr );
//        QVERIFY( library->retro_deinit == nullptr );
//        QVERIFY( library->retro_init == nullptr );
//        QVERIFY( library->retro_get_memory_data == nullptr );
//        QVERIFY( library->retro_get_memory_size == nullptr );
//        QVERIFY( library->retro_get_region == nullptr );
//        QVERIFY( library->retro_get_system_av_info == nullptr );
//        QVERIFY( library->retro_get_system_info == nullptr );
//        QVERIFY( library->retro_load_game == nullptr );
//        QVERIFY( library->retro_load_game_special == nullptr );
//        QVERIFY( library->retro_reset == nullptr );
//        QVERIFY( library->retro_run == nullptr );
//        QVERIFY( library->retro_serialize == nullptr );
//        QVERIFY( library->retro_serialize_size == nullptr );
//        QVERIFY( library->retro_unload_game == nullptr );
//        QVERIFY( library->retro_unserialize == nullptr );

//        QVERIFY( library->retro_set_audio_sample == nullptr );
//        QVERIFY( library->retro_set_audio_sample_batch == nullptr );
//        QVERIFY( library->retro_set_controller_port_device == nullptr );
//        QVERIFY( library->retro_set_environment == nullptr );
//        QVERIFY( library->retro_set_input_poll == nullptr );
//        QVERIFY( library->retro_set_input_state == nullptr );
//        QVERIFY( library->retro_set_video_refresh == nullptr );

//        QVERIFY( library->retro_audio == nullptr );
//        QVERIFY( library->retro_audio_set_state == nullptr );
//        QVERIFY( library->retro_frame_time == nullptr );
//        QVERIFY( library->retro_keyboard_event == nullptr );
//    }


//};
