package com.digisensing.esp32cameracontrol

import android.annotation.SuppressLint
import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.content.ServiceConnection
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.os.IBinder
import android.util.Log
import android.webkit.WebSettings
import android.webkit.WebView
import android.webkit.WebViewClient
import androidx.activity.result.ActivityResultLauncher
import androidx.activity.result.contract.ActivityResultContracts
import androidx.annotation.RequiresApi
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import java.net.HttpURLConnection
import java.net.URL
import android.net.Uri
import android.provider.Settings

interface BLEServiceCallback {
    fun onCredentialsReceived(ssid: String, password: String)
}

class MainActivity : AppCompatActivity(), BLEServiceCallback {
    private var bleService: BLEService? = null
    private var isBound = false
    private var mContext: Context? = null
    @RequiresApi(Build.VERSION_CODES.S)
    val permissions = arrayOf(
        android.Manifest.permission.BLUETOOTH,
        android.Manifest.permission.BLUETOOTH_ADMIN,
        android.Manifest.permission.BLUETOOTH_SCAN,
        android.Manifest.permission.BLUETOOTH_CONNECT,
        android.Manifest.permission.ACCESS_FINE_LOCATION,
        android.Manifest.permission.ACCESS_COARSE_LOCATION
    )
    private lateinit var requestPermissionsLauncher: ActivityResultLauncher<Array<String>>
    private val PERMISSION_REQUEST_CODE = 100

    @SuppressLint("NewApi")
    private fun checkAndRequestPermissions(): Boolean {
        val missingPermissions = permissions.filter {
            ActivityCompat.checkSelfPermission(this, it) != PackageManager.PERMISSION_GRANTED
        }

        return if (missingPermissions.isNotEmpty()) {
            requestPermissionsLauncher.launch(missingPermissions.toTypedArray())
            false
        } else {
            true
        }
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)

        if (requestCode == PERMISSION_REQUEST_CODE) {
            val allGranted = grantResults.all { it == PackageManager.PERMISSION_GRANTED }
            if (allGranted) {
                Log.d("Permissions", "All permissions granted")
                // Call the function that requires permissions here
                startBLEService()
            } else {
                Log.e("Permissions", "Some permissions were denied")
                // Show a message or take appropriate action for denied permissions
            }
        }
    }

    private val serviceConnection = object : ServiceConnection {
        override fun onServiceConnected(name: ComponentName?, service: IBinder?) {
            val binder = service as BLEService.LocalBinder
            bleService = binder.getService()
            isBound = true
            Log.d("MainActivity", "BLE Service is bound")
            if (mContext?.let { bleService?.initialize(it) } == true) {
                bleService?.startScanning()
            }
            bleService?.setCallback(this@MainActivity)
        }

        override fun onServiceDisconnected(name: ComponentName?) {
            bleService?.disconnect()
            bleService = null
            isBound = false
        }
    }

    fun startBLEService() {
        Log.d("MainActivity", "Start to bind BLE Service")
        val intent = Intent(this, BLEService::class.java)
        bindService(intent, serviceConnection, BIND_AUTO_CREATE)
    }

    private fun showPermissionDeniedDialog() {
        AlertDialog.Builder(this)
            .setTitle("Permissions Required")
            .setMessage("The app needs Bluetooth and Location permissions to function. Please grant them in settings.")
            .setPositiveButton("OK") { _, _ ->
                // Open app settings if needed
                val intent = Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS).apply {
                    data = Uri.fromParts("package", packageName, null)
                }
                startActivity(intent)
            }
            .setNegativeButton("Cancel", null)
            .show()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        mContext = this
        requestPermissionsLauncher = registerForActivityResult(
            ActivityResultContracts.RequestMultiplePermissions()
        ) { permissions ->
            val allGranted = permissions.entries.all { it.value }
            if (allGranted) {
                Log.d("Permissions", "All permissions granted")
                startBLEService() // Call your BLE service start function here
            } else {
                Log.e("Permissions", "Some permissions were denied")
                // Handle denied permissions
                showPermissionDeniedDialog()
            }
        }

        // Request permissions
        if (checkAndRequestPermissions()) {
            startBLEService() // Start BLE service only if permissions are already granted
        }
        val webView: WebView = findViewById(R.id.webView)

        // Configure the WebView for video streaming
        val webSettings: WebSettings = webView.settings
        webSettings.javaScriptEnabled = true
        webView.webViewClient = WebViewClient()
        webView.loadUrl("http://192.168.4.1/") // Replace with your ESP32 IP

    }

    override fun onCredentialsReceived(ssid: String, password: String) {
        Log.d("MainActivity", "SSID: $ssid, Password: $password")

        // Use the received SSID and password
        // Optionally update the UI or start WiFi configuration
    }

    override fun onDestroy() {
        super.onDestroy()
        if (isBound) {
            unbindService(serviceConnection)
            isBound = false
        }
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
