package com.digisensing.esp32cameracontrol

import android.Manifest
import android.annotation.SuppressLint
import android.app.Activity
import android.app.Service
import android.bluetooth.*
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.bluetooth.le.ScanFilter
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Binder
import android.os.Build
import android.os.Handler
import android.os.IBinder
import android.os.Looper
import android.os.ParcelUuid
import android.util.Log
import androidx.activity.result.ActivityResultLauncher
import androidx.activity.result.contract.ActivityResultContracts
import androidx.annotation.RequiresApi
import androidx.core.app.ActivityCompat
import java.util.*

class BLEService : Service() {
    private val binder = LocalBinder()
    private var bluetoothManager: BluetoothManager? = null
    private var bluetoothAdapter: BluetoothAdapter? = null
    private var bluetoothGatt: BluetoothGatt? = null
    private var deviceAddress: String? = null
    private var mContext: Context? = null
    private var mConnected: Boolean = false
    private val characteristicQueue: Queue<BluetoothGattCharacteristic> = LinkedList()
    private var mSSID: String = null.toString()
    private var mPassword: String = null.toString()

    companion object {
        const val SERVICE_UUID = "12345678-1234-5678-1234-56789ABCDEF0"
        const val SSID_CHARACTERISTIC_UUID = "ABCDEF01-1234-5678-1234-56789ABCDEF0"
        const val PASSWORD_CHARACTERISTIC_UUID = "ABCDEF02-1234-5678-1234-56789ABCDEF0"
        const val ACK_CHARACTERISTIC_UUID = "abcdef03-1234-5678-1234-56789abcdef0"
    }

    inner class LocalBinder : Binder() {
        fun getService(): BLEService = this@BLEService
    }

    override fun onBind(intent: Intent?): IBinder {
        return binder
    }

    private var callback: BLEServiceCallback? = null

    fun setCallback(callback: BLEServiceCallback) {
        this.callback = callback
    }

    private fun handleCredentials(gatt: BluetoothGatt, ssid: String, password: String) {
        // Send credentials to the callback
        callback?.onCredentialsReceived(ssid, password)
        sendAck(gatt)
        disconnect()
    }

    fun initialize(context: Context): Boolean {
        bluetoothManager = getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothAdapter = bluetoothManager?.adapter
        mContext = context
        Log.d("BLEService", "BLE initialized")
        return bluetoothAdapter != null
    }


    fun startScanning() {
        if (!mConnected) {
            Log.d("BLEService", "Starting BLE scanning...")
            val bluetoothAdapter = BluetoothAdapter.getDefaultAdapter()
            val bluetoothLeScanner = bluetoothAdapter.bluetoothLeScanner

            // Optionally filter for devices advertising your service UUID
            val filters = listOf(
                ScanFilter.Builder()
                    .setServiceUuid(ParcelUuid.fromString(SERVICE_UUID)) // Replace with your service UUID
                    .build()
            )

            val settings = ScanSettings.Builder()
                .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
                .build()


            if (ActivityCompat.checkSelfPermission(
                    this,
                    Manifest.permission.BLUETOOTH_SCAN
                ) != PackageManager.PERMISSION_GRANTED
            ) {
                // TODO: Consider calling
                //    ActivityCompat#requestPermissions
                // here to request the missing permissions, and then overriding
                //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
                //                                          int[] grantResults)
                // to handle the case where the user grants the permission. See the documentation
                // for ActivityCompat#requestPermissions for more details.
                Log.d("BLEService", "BLE scanning permission is not granted")
                return
            }
            bluetoothLeScanner.startScan(filters, settings, object : ScanCallback() {
                override fun onScanResult(callbackType: Int, result: ScanResult) {
                    super.onScanResult(callbackType, result)
                    val device = result.device
                    Log.d("BLEScan", "Found device: ${device.name} - ${device.address}")
                    if (device.name == "ESP32_Camera_Config") { // Match the ESP32's advertised name
                        if (mContext?.let {
                                ActivityCompat.checkSelfPermission(
                                    it,
                                    Manifest.permission.BLUETOOTH_SCAN
                                )
                            } != PackageManager.PERMISSION_GRANTED
                        ) {
                            // TODO: Consider calling
                            //    ActivityCompat#requestPermissions
                            // here to request the missing permissions, and then overriding
                            //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
                            //                                          int[] grantResults)
                            // to handle the case where the user grants the permission. See the documentation
                            // for ActivityCompat#requestPermissions for more details.
                            return
                        }
                        bluetoothLeScanner.stopScan(this)
                        connectToDevice(device)
                    }
                }

                override fun onBatchScanResults(results: MutableList<ScanResult>?) {
                    super.onBatchScanResults(results)
                    results?.forEach { result ->
                        if (mContext?.let {
                                ActivityCompat.checkSelfPermission(
                                    it,
                                    Manifest.permission.BLUETOOTH_CONNECT
                                )
                            } != PackageManager.PERMISSION_GRANTED
                        ) {
                            // TODO: Consider calling
                            //    ActivityCompat#requestPermissions
                            // here to request the missing permissions, and then overriding
                            //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
                            //                                          int[] grantResults)
                            // to handle the case where the user grants the permission. See the documentation
                            // for ActivityCompat#requestPermissions for more details.
                            return
                        }
                        Log.d("BLEScan", "Batch scan found device: ${result.device.name}")
                    }
                }

                override fun onScanFailed(errorCode: Int) {
                    super.onScanFailed(errorCode)
                    Log.e("BLEScan", "Scan failed with error code: $errorCode")
                }
            })
        }
    }

    private fun connectToDevice(device: BluetoothDevice) {
        if (mConnected) {
            return
        }
        if (ActivityCompat.checkSelfPermission(
                this,
                Manifest.permission.BLUETOOTH_CONNECT
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            // TODO: Consider calling
            //    ActivityCompat#requestPermissions
            // here to request the missing permissions, and then overriding
            //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
            //                                          int[] grantResults)
            // to handle the case where the user grants the permission. See the documentation
            // for ActivityCompat#requestPermissions for more details.
            return
        }
        device.connectGatt(this, false, object : BluetoothGattCallback() {
            override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
                if (newState == BluetoothProfile.STATE_CONNECTED) {
                    Log.d("BLEConnect", "Connected to GATT server")
                    if (mContext?.let {
                            ActivityCompat.checkSelfPermission(
                                it,
                                Manifest.permission.BLUETOOTH_CONNECT
                            )
                        } != PackageManager.PERMISSION_GRANTED
                    ) {
                        // TODO: Consider calling
                        //    ActivityCompat#requestPermissions
                        // here to request the missing permissions, and then overriding
                        //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
                        //                                          int[] grantResults)
                        // to handle the case where the user grants the permission. See the documentation
                        // for ActivityCompat#requestPermissions for more details.
                        return
                    }
                    gatt.discoverServices()
                    mConnected = true
                } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                    Log.d("BLEConnect", "Disconnected from GATT server")
                }
            }

            override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
                if (status == BluetoothGatt.GATT_SUCCESS) {
                    val service = gatt.getService(UUID.fromString(SERVICE_UUID))
                    for (service in gatt.services) {
                        Log.d("BLEService", "Service UUID: ${service.uuid}")
                        for (characteristic in service.characteristics) {
                            Log.d("BLEService", "  Characteristic UUID: ${characteristic.uuid}")
                        }
                    }

                    if (service != null) {
                        val ssidCharacteristic = service.getCharacteristic(UUID.fromString(SSID_CHARACTERISTIC_UUID))
                        val passwordCharacteristic = service.getCharacteristic(UUID.fromString(PASSWORD_CHARACTERISTIC_UUID))

                        if (ssidCharacteristic != null) characteristicQueue.add(ssidCharacteristic)
                        if (passwordCharacteristic != null) characteristicQueue.add(passwordCharacteristic)

                        readNextCharacteristic(gatt)
                    } else {
                        Log.e("BLEService", "Service not found")
                    }
                } else {
                    Log.e("BLEService", "Service discovery failed with status: $status")
                }
            }

            override fun onCharacteristicRead(gatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic, status: Int) {
                if (status == BluetoothGatt.GATT_SUCCESS) {
                    when (characteristic.uuid) {
                        UUID.fromString(SSID_CHARACTERISTIC_UUID) -> {
                            mSSID = characteristic.getStringValue(0)
                            Log.d("BLEService", "SSID: ${characteristic.getStringValue(0)}")
                        }
                        UUID.fromString(PASSWORD_CHARACTERISTIC_UUID) -> {
                            mPassword = characteristic.getStringValue(0)
                            Log.d("BLEService", "Password: ${characteristic.getStringValue(0)}")
                        }
                        else -> Log.e("BLEService", "Unexpected UUID: ${characteristic.uuid}")
                    }
                } else {
                    Log.e("BLEService", "Characteristic read failed with status: $status")
                }

                // Read the next characteristic in the queue
                readNextCharacteristic(gatt)
            }

            override fun onCharacteristicWrite(gatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic, status: Int) {
                if (status == BluetoothGatt.GATT_SUCCESS) {
                    Log.d("BLEService", "ACK successfully written to ESP32")
                } else {
                    Log.e("BLEService", "Failed to write ACK: Status $status")
                }
            }
        })
    }

    fun connect(address: String): Any {
        if (mConnected) {
            return true
        }
        if (bluetoothAdapter == null || address.isEmpty()) {
            Log.e("BLEService", "BluetoothAdapter not initialized or unspecified address.")
            return false
        }

        // Check if we're already connected
        if (deviceAddress == address && bluetoothGatt != null) {
            Log.d("BLEService", "Reusing existing BluetoothGatt for connection.")
            return if (ActivityCompat.checkSelfPermission(
                    this,
                    Manifest.permission.BLUETOOTH_CONNECT
                ) != PackageManager.PERMISSION_GRANTED
            ) {
                // TODO: Consider calling
                //    ActivityCompat#requestPermissions
                // here to request the missing permissions, and then overriding
                //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
                //                                          int[] grantResults)
                // to handle the case where the user grants the permission. See the documentation
                // for ActivityCompat#requestPermissions for more details.
                return false
            } else {
                return true
            }
            bluetoothGatt?.connect() == true
            mConnected = true
        }

        // Connect to the device
        val device = bluetoothAdapter?.getRemoteDevice(address) ?: return false
        if (ActivityCompat.checkSelfPermission(
                this,
                Manifest.permission.BLUETOOTH_CONNECT
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            // TODO: Consider calling
            //    ActivityCompat#requestPermissions
            // here to request the missing permissions, and then overriding
            //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
            //                                          int[] grantResults)
            // to handle the case where the user grants the permission. See the documentation
            // for ActivityCompat#requestPermissions for more details.
            return false
        }
        bluetoothGatt = device.connectGatt(this, false, gattCallback)
        deviceAddress = address
        Log.d("BLEService", "Connecting to $address")
        return true
    }

    // Method to disconnect from the BLE device
    fun disconnect() {
        bluetoothGatt.let {
            Log.d("BLEService", "Disconnecting from device")
            if (ActivityCompat.checkSelfPermission(
                    this,
                    Manifest.permission.BLUETOOTH_CONNECT
                ) != PackageManager.PERMISSION_GRANTED
            ) {
                // TODO: Consider calling
                //    ActivityCompat#requestPermissions
                // here to request the missing permissions, and then overriding
                //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
                //                                          int[] grantResults)
                // to handle the case where the user grants the permission. See the documentation
                // for ActivityCompat#requestPermissions for more details.
                return
            }
            if (it != null) {
                it.disconnect()
            }
            if (it != null) {
                it.close()
            } // Optional: Clean up resources
            bluetoothGatt = null
        } ?: Log.e("BLEService", "BluetoothGatt is null. Nothing to disconnect.")
    }

    fun readCharacteristic(gatt: BluetoothGatt) {
        val service = gatt.getService(UUID.fromString(SERVICE_UUID))
        val ssidCharacteristic = service?.getCharacteristic(UUID.fromString(SSID_CHARACTERISTIC_UUID))
        val passwordCharacteristic = service?.getCharacteristic(UUID.fromString(PASSWORD_CHARACTERISTIC_UUID))
        // Read the SSID
        ssidCharacteristic?.let {
            if (mContext?.let { it1 ->
                    ActivityCompat.checkSelfPermission(
                        it1,
                        Manifest.permission.BLUETOOTH_CONNECT
                    )
                } != PackageManager.PERMISSION_GRANTED
            ) {
                // TODO: Consider calling
                //    ActivityCompat#requestPermissions
                // here to request the missing permissions, and then overriding
                //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
                //                                          int[] grantResults)
                // to handle the case where the user grants the permission. See the documentation
                // for ActivityCompat#requestPermissions for more details.
                Log.d("BLEService", "No permission to read Characteristic")
                return
            }
            Log.d("BLEService", "To read SSID Characteristic")
            gatt.readCharacteristic(it)
        }

        // Read the Password
        passwordCharacteristic?.let {
            Log.d("BLEService", "To read PWD Characteristic")
            gatt.readCharacteristic(it)
        }
    }

    private fun readNextCharacteristic(gatt: BluetoothGatt) {
        if (characteristicQueue.isNotEmpty()) {
            val characteristic = characteristicQueue.poll()
            if (ActivityCompat.checkSelfPermission(
                    this,
                    Manifest.permission.BLUETOOTH_CONNECT
                ) != PackageManager.PERMISSION_GRANTED
            ) {
                // TODO: Consider calling
                //    ActivityCompat#requestPermissions
                // here to request the missing permissions, and then overriding
                //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
                //                                          int[] grantResults)
                // to handle the case where the user grants the permission. See the documentation
                // for ActivityCompat#requestPermissions for more details.
                return
            }
            gatt.readCharacteristic(characteristic)
        } else {
            handleCredentials(gatt, mSSID, mPassword)
        }
    }

    fun sendAck(gatt: BluetoothGatt) {
        val device = gatt.device
        if (device == null) {
            Log.e("BLEService", "BluetoothDevice is null, cannot send ACK")
            return
        }

        val connectionState = bluetoothManager?.getConnectionState(device, BluetoothProfile.GATT)
        Log.d("BLEService", "Connection state: $connectionState")
        if (connectionState != BluetoothProfile.STATE_CONNECTED) {
            Log.e("BLEService", "BLE device is not connected")
            return
        }

        val service = gatt.getService(UUID.fromString(SERVICE_UUID))
        if (service != null) {
            Log.d("BLEService", "Service found: ${service.uuid}")

            val ackCharacteristic =
                service?.getCharacteristic(UUID.fromString(ACK_CHARACTERISTIC_UUID))
            if (ackCharacteristic != null) {
                val properties = ackCharacteristic.properties
                Log.d("BLEService", "ACK characteristic properties: $properties")
                if (properties and BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE != 0) {
                    Log.d("BLEService", "Characteristic uses WRITE_NO_RESPONSE. No callback will be triggered.")
                }

                if (ackCharacteristic != null) {
                    ackCharacteristic.value = "ACK".toByteArray(Charsets.UTF_8)
                    if (ActivityCompat.checkSelfPermission(
                            this,
                            Manifest.permission.BLUETOOTH_CONNECT
                        ) != PackageManager.PERMISSION_GRANTED
                    ) {
                        // TODO: Consider calling
                        //    ActivityCompat#requestPermissions
                        // here to request the missing permissions, and then overriding
                        //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
                        //                                          int[] grantResults)
                        // to handle the case where the user grants the permission. See the documentation
                        // for ActivityCompat#requestPermissions for more details.
                        return
                    }
                    ackCharacteristic.writeType = BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE
                    ackCharacteristic.value = "ACK".toByteArray(Charsets.UTF_8)
                    val result = gatt.writeCharacteristic(ackCharacteristic)
                    Log.d("BLEService", "Write ACK result: $result")
                } else {
                    Log.e("BLEService", "ACK characteristic not found")
                }
            }
        }
    }

    private val gattCallback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.d("BLEService", "Connected to GATT server.")
                bluetoothGatt = gatt
                if (mContext?.let {
                        ActivityCompat.checkSelfPermission(
                            it,
                            Manifest.permission.BLUETOOTH_CONNECT
                        )
                    } != PackageManager.PERMISSION_GRANTED
                ) {
                    // TODO: Consider calling
                    //    ActivityCompat#requestPermissions
                    // here to request the missing permissions, and then overriding
                    //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
                    //                                          int[] grantResults)
                    // to handle the case where the user grants the permission. See the documentation
                    // for ActivityCompat#requestPermissions for more details.
                    return
                }
                bluetoothGatt?.discoverServices()
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.d("BLEService", "Disconnected from GATT server.")
                bluetoothGatt = null
            }
        }

        override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                readCharacteristic(gatt)
            } else {
                Log.w("BLEService", "onServicesDiscovered received: $status")
            }
        }

        override fun onCharacteristicRead(gatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic, status: Int) {
            Log.d("BLEService", "Reading successful.")
            if (status == BluetoothGatt.GATT_SUCCESS) {
                when (characteristic.uuid.toString()) {
                    SSID_CHARACTERISTIC_UUID -> Log.d("BLEService", "SSID: ${characteristic.getStringValue(0)}")
                    PASSWORD_CHARACTERISTIC_UUID -> Log.d("BLEService", "Password: ${characteristic.getStringValue(0)}")
                }
            }
        }
    }
}
