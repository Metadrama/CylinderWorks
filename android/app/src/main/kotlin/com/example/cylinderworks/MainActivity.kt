package com.example.cylinderworks

import android.os.Build
import com.example.cylinderworks.engine.EngineDiagnosticsRegistry
import com.example.cylinderworks.engine.EngineRendererViewFactory
import io.flutter.embedding.android.FlutterActivity
import io.flutter.embedding.engine.FlutterEngine
import io.flutter.plugin.common.MethodChannel

class MainActivity : FlutterActivity() {
	override fun configureFlutterEngine(flutterEngine: FlutterEngine) {
		super.configureFlutterEngine(flutterEngine)
		flutterEngine
			.platformViewsController
			.registry
			.registerViewFactory("engine/renderer", EngineRendererViewFactory())

		MethodChannel(flutterEngine.dartExecutor.binaryMessenger, "engine/diagnostics")
			.setMethodCallHandler { call, result ->
				when (call.method) {
					"getSnapshot" -> {
						val snapshot = EngineDiagnosticsRegistry.snapshot() ?: mutableMapOf()
						val enriched = snapshot.toMutableMap()
						enriched["deviceManufacturer"] = Build.MANUFACTURER
						enriched["deviceModel"] = Build.MODEL
						enriched["cpuHardware"] = Build.HARDWARE
						if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
							enriched["socModel"] = Build.SOC_MODEL ?: ""
							enriched["socManufacturer"] = Build.SOC_MANUFACTURER ?: ""
						}
						result.success(enriched)
					}
					else -> result.notImplemented()
				}
			}
	}
}
