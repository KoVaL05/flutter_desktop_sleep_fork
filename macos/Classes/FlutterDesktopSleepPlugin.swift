import Cocoa
import FlutterMacOS
import os

public class FlutterDesktopSleepPlugin: NSObject, FlutterPlugin {
    var methodChannel: FlutterMethodChannel?

    public static func register(with registrar: FlutterPluginRegistrar) {
        let instance = FlutterDesktopSleepPlugin()
        let channel = FlutterMethodChannel(
            name: "flutter_desktop_sleep", binaryMessenger: registrar.messenger)
        instance.methodChannel = channel
        instance.setupSleepNotifications()
        registrar.addMethodCallDelegate(instance, channel: channel)
    }

    public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
        switch call.method {
        case "getPlatformVersion":
            result("macOS " + ProcessInfo.processInfo.operatingSystemVersionString)
        case "terminateWindow":
            os_log("Going to close app", type: .info)
            DispatchQueue.main.async {
                NSApplication.shared.reply(toApplicationShouldTerminate: true)
            }
            result(nil)
        default:
            result(FlutterMethodNotImplemented)
        }
    }

    private func setupSleepNotifications() {
        let notificationCenter = NSWorkspace.shared.notificationCenter
        notificationCenter.addObserver(
            self, selector: #selector(sleepListener(_:)), name: NSWorkspace.willSleepNotification,
            object: nil)
        notificationCenter.addObserver(
            self, selector: #selector(sleepListener(_:)), name: NSWorkspace.didWakeNotification,
            object: nil)
        notificationCenter.addObserver(
            self, selector: #selector(sleepListener(_:)),
            name: NSWorkspace.willPowerOffNotification, object: nil)
        notificationCenter.addObserver(
            self, selector: #selector(sleepListener(_:)),
            name: NSWorkspace.screensDidSleepNotification, object: nil)
    }

    @objc private func sleepListener(_ notification: Notification) {
        switch notification.name {
        case NSWorkspace.willSleepNotification:
            methodChannel?.invokeMethod("onWindowsSleep", arguments: "sleep_from_menu")
            os_log("Going to sleep", type: .info)
        case NSWorkspace.didWakeNotification:
            methodChannel?.invokeMethod("onWindowsSleep", arguments: "wake_up")
            os_log("didWake", type: .info)
        case NSWorkspace.willPowerOffNotification:
            methodChannel?.invokeMethod("onWindowsSleep", arguments: "willPowerOff")
            os_log("willPowerOff or log out", type: .info)
        case NSWorkspace.screensDidSleepNotification:
            methodChannel?.invokeMethod("onWindowsSleep", arguments: "screensDidSleep")
            os_log("screensDidSleep", type: .info)
        default:
            os_log("Some other event", type: .info)
        }
    }
}
