package com.digisensing.esp32cameracontrol

import android.os.Bundle
import android.webkit.WebSettings
import android.webkit.WebView
import android.webkit.WebViewClient
import androidx.appcompat.app.AppCompatActivity
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import java.net.HttpURLConnection
import java.net.URL
import android.widget.Button

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val webView: WebView = findViewById(R.id.webView)


        // Configure the WebView for video streaming
        val webSettings: WebSettings = webView.settings
        webSettings.javaScriptEnabled = true
        webView.webViewClient = WebViewClient()
        webView.loadUrl("http://192.168.4.1/") // Replace with your ESP32 IP

    }

    private fun sendCommand(command: String, value: String) {
        CoroutineScope(Dispatchers.IO).launch {
            try {
                val url = URL("http://192.168.4.1/command") // Replace with ESP32 IP
                val connection = url.openConnection() as HttpURLConnection
                connection.requestMethod = "POST"
                connection.doOutput = true
                val postData = "key=$command&value=$value"
                connection.outputStream.write(postData.toByteArray())
                connection.inputStream.bufferedReader().use { it.readText() }
                connection.disconnect()
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }
    }
}
