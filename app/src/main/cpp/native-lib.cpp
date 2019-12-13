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
char *error;

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
 * 自定义 错误退出函数
 * @param cinfo
 */
METHODDEF(void)
my_error_exit(j_common_ptr cinfo) {
    myerror myerr = (myerror) cinfo->err;
    (*cinfo->err->output_message)(cinfo);//函数指针调用
    error = const_cast<char *>(myerr->jmgr.jpeg_message_table[myerr->jmgr.msg_code]);
    LOG_D("jpeg_message_table[%d]:%s", myerr->jmgr.msg_code,
          myerr->jmgr.jpeg_message_table[myerr->jmgr.msg_code]);
    // LOGE("addon_message_table:%s", myerr->pub.addon_message_table);
//  LOGE("SIZEOF:%d",myerr->pub.msg_parm.i[0]);
//  LOGE("sizeof:%d",myerr->pub.msg_parm.i[1]);
    longjmp(myerr->jumpBuffer, 1);
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
        JNIEnv *env, jobject jobj,
        jobject bitmap, jint width, jint height, jint quality, jbyteArray bytearray,
        jboolean optimize) {
    char *filename = jstringToString(env, bytearray);

    BYTE *addptr = nullptr;

    //获取bitmap info 以及锁住bitmap 像素
    AndroidBitmapInfo info;
    if (AndroidBitmap_getInfo(env, bitmap, &info) < 0) {
        LOG_D("GET BITMAP INFO ERROR");
        return 0;
    }
    if (AndroidBitmap_lockPixels(env, bitmap, reinterpret_cast<void **>(&addptr)) < 0) {
        LOG_D("LOCK BITMAP ERROR");
        return 0;
    }


    BYTE *data;//存储每个像素rgb的指针
    BYTE r, g, b;
    BYTE *tempale;//临时bitmap 指针变量
    data = static_cast<BYTE *> (malloc(width * height * 3));
    int color;
    tempale = data;
    int i, j;
    for (i = 0; i < height; i++) {// 行高遍历
        for (j = 0; j < width; j++) {
            color = *((int *) addptr);
            r = ((color & 0x00FF0000) >> 16);
            g = ((color & 0x0000FF00) >> 8);
            b = color & 0x000000FF;

            *data = b;//低位存储b
            *(data + 1) = g;//移到下一个字节后存储g
            *(data + 2) = r;//高位存储r

            data = data + 3;//存储完一个像素的rgb后 移动到下一个存储位置
            addptr += 4;//存储bitmap信息的指针移动到一个像素
        }
    }


    AndroidBitmap_unlockPixels(env, bitmap);
    int resultCode = generatJPEG(tempale, width, height, filename, quality, optimize);
    if (resultCode < 0) {
        LOG_D("generatJpeg error");
        return 0;
    }
    free(tempale);
    return 1;
}


int generatJPEG(BYTE *data, jint w, jint h, const char *name, jint quality, jboolean opmized) {

    int mComponent = 3;//rgb

    struct jpeg_compress_struct jcs;

    struct myErrorMsr jem;

    jcs.err = jpeg_std_error(
            &jem.jmgr);//cinfo.err = jpeg_std_error(&jerr);  然后是将错误处理结构对象绑定在JPEG对象上。
    jem.jmgr.error_exit = my_error_exit;//自定义错误退出函数



    /**
     * goto语句看起来是处理异常的更可行方案。不幸的是，goto是本地的：它只能跳到所在函数内部的标号上，而不能将控制权转移到所在程序的任意地点（当然，除非你的所有代码都在main体中）。
　　为了解决这个限制，C函数库提供了setjmp()和longjmp()函数，它们分别承担非局部标号和goto作用。头文件<setjmp.h>申明了这些函数及同时所需的jmp_buf数据类型。
　　原理非常简单：
　　l setjmp(j)设置“jump”点，用正确的程序上下文填充jmp_buf对象j。这个上下文包括程序存放位置、栈和框架指针，其它重要的寄存器和内存数据。当初始化完jump的上下文，setjmp()返回0值。
　　l 以后调用longjmp(j,r)的效果就是一个非局部的goto或“长跳转”到由j描述的上下文处（也就是到那原来设置j的setjmp()处）。当作为长跳转的目标而被调用时，setjmp()返回r或1（如果r设为0的话）。（记住，setjmp()不能在这种情况时返回0。）
     */
    if (setjmp(jem.jumpBuffer)) {
        return 0;
    }
    jpeg_create_compress(&jcs);//创建jpeg压缩
    FILE *f = fopen(name, "wb");
    if (f == NULL) {
        return 0;
    }
    jpeg_stdio_dest(&jcs, f);//打开f的标准输出流
    jcs.image_width = w;
    jcs.image_height = h;

    if (opmized) {
        LOG_D("optimize==ture");
    } else {
        LOG_D("optimize==false");
    }
    jcs.arith_code = false;
    //   jcs.num_components = mComponent;//num_components	/* # of color components in JPEG image */ jpeg 图片中的颜色分量 不能使用这个参数
    jcs.input_components = mComponent; //input_components   of color components in input image 输入图像的颜色成分,既是 从每个像素中输入的rgb 分量
    if (mComponent == 1)
        jcs.in_color_space = JCS_GRAYSCALE;
    else
        jcs.in_color_space = JCS_RGB;

    jpeg_set_defaults(&jcs);
    jcs.optimize_coding = opmized;
    jpeg_set_quality(&jcs, quality, true);
    jpeg_start_compress(&jcs, TRUE);

    JSAMPROW row_pointer[1];//一行像素样本图像 的 指针数组
    int row_stride;
    row_stride = jcs.image_width * mComponent;//图片的宽度 乘以 每个像素的所占字节（RGB）
    while (jcs.next_scanline < jcs.image_height) {//如果下一行小于 图片的高则往下执行
        row_pointer[0] = &data[jcs.next_scanline * row_stride];

        jpeg_write_scanlines(&jcs, row_pointer, 1);//写入一行
    }

    if (jcs.optimize_coding) {
        LOG_D("optimize==ture");
    } else {
        LOG_D("optimize==false");
    }

    jpeg_finish_compress(&jcs);
    jpeg_destroy_compress(&jcs);

    fclose(f);
    return 1;
}

