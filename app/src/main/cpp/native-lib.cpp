#include <jni.h>
#include <string>


extern "C"{
#include "include/jpeglib.h"
#include "include/cdjpeg.h"          /* Common decls for cjpeg/djpeg applications */
#include "include/jversion.h"        /* for version message */
#include "include/config.h"
}
extern "C" JNIEXPORT jstring JNICALL
Java_com_james_jpegcompress_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
