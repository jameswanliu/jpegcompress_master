#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/bitmap.h>
#include <csetjmp>

extern "C" {
#include "include/jpeglib.h"
#include "include/cdjpeg.h"          /* Common decls for cjpeg/djpeg applications */
#include "include/jversion.h"        /* for version message */
#include "include/config.h"
}
#define TAG "My_compress"
#define LOG_D(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)

typedef uint8_t BYTE;

char error;

int
generatJPEG(BYTE *addstr, jint w, jint h, const char *name, jint quality, jboolean opmized);//定义方法

struct myErrorMsr {
    struct jpeg_error_mgr jmgr;
    jmp_buf jumpBuffer;
};

typedef myErrorMsr *myerror;


/**
 * 定义一个函数将 filename bytearray 返回 char *
 */

char *jstringToString(JNIEnv *env, jbyteArray array) {
    char *str = nullptr;
    jsize length = env->GetArrayLength(array);
    jbyte *elemt = env->GetByteArrayElements(array, JNI_FALSE);
    if (length > 0) {
        //获取bytearray 元素
        str = (char *) malloc(length + 1);
        str[length] = 0;
        memcpy(str, elemt, length);
    }
    env->ReleaseByteArrayElements(array, elemt, 0);
    return str;
}


extern "C" JNIEXPORT jstring JNICALL
Java_com_james_jpegcompress_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}



/**
 *
 * 压缩
 *
 *   bit: Bitmap, w: Int, h: Int, quality: Int, fileNameBytes: Array<Byte>,
        optimize: Boolean
 */
extern "C" JNIEXPORT jint JNICALL
Java_com_james_jpegcompress_MainActivity_compressJpeg(
        JNIEnv *env,
        jobject bitmap, jint width, jint height, jint quality, jbyteArray bytearray,
        jboolean optimize) {
    char * filename = jstringToString(env,bytearray);

    char *addptr = nullptr;

    //获取bitmap info 以及锁住bitmap 像素
    AndroidBitmapInfo info;
    if(AndroidBitmap_getInfo(env,bitmap,&info)<0){
        LOG_D("GET BITMAP INFO ERROR");
        return 0;
    }
    if(AndroidBitmap_lockPixels(env,bitmap,(void**)addptr)<0){
        LOG_D("LOCK BITMAP ERROR");
        return 0;
    }







    return 1;
}



