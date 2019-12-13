package com.james.jpegcompress

import android.Manifest
import android.content.pm.PackageManager
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Canvas
import android.graphics.Rect
import android.os.Build
import android.os.Bundle
import androidx.annotation.RequiresApi
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.PermissionChecker
import kotlinx.android.synthetic.main.activity_main.*
import java.io.File


class MainActivity : AppCompatActivity() {

    private var path = ""
    private val DEFAULT_QUALITY = 40
    private val READ_EXTERNAL_STORAGE = 0
    private val WRITE_EXTERNAL_STORAGE = 1

    @RequiresApi(Build.VERSION_CODES.KITKAT)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        path = getExternalFilesDir("")?.canonicalPath + "/compress.jpg"
        if (PermissionChecker.checkSelfPermission(
                this@MainActivity,
                Manifest.permission.READ_EXTERNAL_STORAGE
            ) < 0
        ) {
            ActivityCompat.requestPermissions(
                this@MainActivity,
                arrayOf<String>(Manifest.permission.READ_EXTERNAL_STORAGE),
                READ_EXTERNAL_STORAGE
            )
        }


        if (PermissionChecker.checkSelfPermission(
                this@MainActivity,
                Manifest.permission.WRITE_EXTERNAL_STORAGE
            )
            < 0
        ) {
            ActivityCompat.requestPermissions(
                this@MainActivity,
                arrayOf<String>(Manifest.permission.WRITE_EXTERNAL_STORAGE),
                WRITE_EXTERNAL_STORAGE
            )
        } else {
            val file = File(path)
            file.exists().apply {
                file.delete()
                file.createNewFile()
            }
        }

        val bitmap = BitmapFactory.decodeResource(resources, R.mipmap.timg)

        var ret = 0
        iv_normal.setImageBitmap(bitmap)
        btn_compress.setOnClickListener {
            ret = compress(
                bitmap, path
            )
            if (ret == 1) {
                val comress = BitmapFactory.decodeFile(path)
                iv_compress.setImageBitmap(comress)
            }

        }
    }


    private fun sizeOf(value: Bitmap): Int {
        return value.rowBytes * value.height
    }


    private fun compress(bitmap: Bitmap, fileName: String): Int {
        return compressBitmap(bitmap, DEFAULT_QUALITY, fileName)
    }

    private fun compressBitmap(bit: Bitmap, quality: Int, fileName: String): Int {
        var result: Bitmap? = null
        try {
            result = Bitmap.createBitmap(bit.width, bit.height, Bitmap.Config.ARGB_8888)
            val canvas = Canvas(result)
            val rect = Rect(0, 0, bit.width, bit.height)// original
            canvas.drawBitmap(bit, null, rect, null)
            return saveBitmap(result, quality, fileName)
        } finally {
            result?.recycle()
        }
    }


    fun saveBitmap(bit: Bitmap, quality: Int, fileName: String): Int {
        return compressJpeg(bit, bit.width, bit.height, quality, fileName.toByteArray(), true)
    }


    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String


    /**
     * JNIEnv *env, jobject type, jobject bit, jint w, jint h, jint quality,
    jbyteArray fileNameBytes_, jboolean optimize
     */
    external fun compressJpeg(
        bit: Bitmap, w: Int, h: Int, quality: Int, fileNameBytes: ByteArray,
        optimize: Boolean
    ): Int


    companion object {

        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }


    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        if (grantResults.size != 1 || grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            val file = File(path)
            file.exists().apply {
                file.delete()
                file.createNewFile()
            }
        } else {

        }
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)


    }
}
