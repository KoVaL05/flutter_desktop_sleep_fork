#include "flutter_desktop_sleep_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>

namespace flutter_desktop_sleep {
    using flutter::EncodableMap;
    using flutter::EncodableValue;

    static LRESULT CALLBACK WindowCloseWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
    static WNDPROC oldProc;
    static std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel_;

// static
void FlutterDesktopSleepPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
      HWND handle = GetActiveWindow();
            oldProc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(handle, GWLP_WNDPROC));
            SetWindowLongPtr(handle, GWLP_WNDPROC, (LONG_PTR)WindowCloseWndProc);
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "flutter_desktop_sleep",
          &flutter::StandardMethodCodec::GetInstance());
  channel_ = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
      registrar->messenger(),
      "flutter_desktop_sleep",
      &flutter::StandardMethodCodec::GetInstance());
  ;
  auto plugin = std::make_unique<FlutterDesktopSleepPlugin>();

        plugin->SetWindow(handle);
  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

FlutterDesktopSleepPlugin::FlutterDesktopSleepPlugin() {}

FlutterDesktopSleepPlugin::~FlutterDesktopSleepPlugin() {}

void FlutterDesktopSleepPlugin::SetWindow(HWND handle)
    {
        m_windowHandle = handle;
    }

    HWND FlutterDesktopSleepPlugin::GetWindow()
    {
        return m_windowHandle;
    }

void FlutterDesktopSleepPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
        if (method_call.method_name() == "getPlatformVersion") {
        std::ostringstream version_stream;
        version_stream << "Windows ";
        version_stream << (IsWindows10OrGreater() ? "10+" : IsWindows8OrGreater() ? "8" : "7 or earlier");
        result->Success(flutter::EncodableValue(version_stream.str()));
        } else if (method_call.method_name() == "terminateWindow") {
        if (m_windowHandle) {
            DestroyWindow(m_windowHandle);
            result->Success(flutter::EncodableValue(nullptr));
        } else {
            result->Error("no_window", "The active window does not exist");
        }
        } else {
        result->NotImplemented();
        }
    }

LRESULT CALLBACK WindowCloseWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_CLOSE || message == WM_QUERYENDSESSION) {
        auto args = std::make_unique<flutter::EncodableValue>("terminate_app");
        channel_->InvokeMethod("onWindowsSleep", std::move(args));
        return 0;  // Prevent further handling
    }

    if (message == WM_POWERBROADCAST) {
        if (wParam == PBT_APMSUSPEND) {
            auto args = std::make_unique<flutter::EncodableValue>("sleep");
            channel_->InvokeMethod("onWindowsSleep", std::move(args));
        } else if (wParam == PBT_APMRESUMEAUTOMATIC) {
            auto args = std::make_unique<flutter::EncodableValue>("woke_up");
            channel_->InvokeMethod("onWindowsSleep", std::move(args));
        }
        return TRUE;
        }

        return CallWindowProc(oldProc, hWnd, message, wParam, lParam);
    }
}  // namespace flutter_desktop_sleep
