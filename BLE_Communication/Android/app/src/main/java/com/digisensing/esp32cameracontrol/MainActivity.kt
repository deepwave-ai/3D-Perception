package com.digisensing.esp32cameracontrol

import SplashScreen
import android.Manifest
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
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.runtime.Composable
import androidx.compose.material3.Text
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.sp

import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import androidx.compose.runtime.getValue
import androidx.compose.runtime.setValue

interface BLEServiceCallback {
    fun onCredentialsReceived(ssid: String, password: String)
}

class MainActivity : AppCompatActivity(), BLEServiceCallback {
    private var bleService: BLEService? = null
    private var isBound = false
    private var mContext: Context? = null
    @RequiresApi(Build.VERSION_CODES.S)
    val permissions = arrayOf(
        Manifest.permission.BLUETOOTH,
        Manifest.permission.BLUETOOTH_ADMIN,
        Manifest.permission.BLUETOOTH_SCAN,
        Manifest.permission.BLUETOOTH_CONNECT,
        Manifest.permission.ACCESS_FINE_LOCATION,
        Manifest.permission.ACCESS_COARSE_LOCATION
    )
    private lateinit var requestPermissionsLauncher: ActivityResultLauncher<Array<String>>
    private val PERMISSION_REQUEST_CODE = 100

    private var mCredentials by mutableStateOf(Pair("", "")) // Use MutableState for Compose

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

        setContent {
            MyApp()
        }

        //setContentView(R.layout.activity_main)
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

/*
        val webView: WebView = findViewById(R.id.webView)

        // Configure the WebView for video streaming
        val webSettings: WebSettings = webView.settings
        webSettings.javaScriptEnabled = true
        webView.webViewClient = WebViewClient()
        webView.loadUrl("http://192.168.4.1/") // Replace with your ESP32 IP
        */
    }

    @Composable
    fun MyApp() {
        val navController = rememberNavController()

        NavHost(navController, startDestination = "splash") {
            composable("splash") {
                SplashScreen(onSplashTimeout = {
                    navController.navigate("main") {
                        popUpTo("splash") { inclusive = true }
                    }
                })
            }
            composable("main") {
                MainScreen()
            }
        }
    }

    @Composable
    fun MainScreen() {
        val credentials = mCredentials // Observe state

        Surface(
            modifier = Modifier.fillMaxSize(),
            color = MaterialTheme.colorScheme.background
        ) {
            Column(
                modifier = Modifier.fillMaxSize(),
                verticalArrangement = Arrangement.Center,
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Text(
                    text = "SSID: ${credentials.first}\nPassword: ${credentials.second}",
                    fontSize = 20.sp
                )
            }
        }
    }

    override fun onCredentialsReceived(ssid: String, password: String){
        Log.d("MainActivity", "SSID: $ssid, Password: $password")
        mCredentials = ssid to password

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
