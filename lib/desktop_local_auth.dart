import 'dart:async';
import 'dart:io';

import 'package:flutter/services.dart';

class DesktopLocalAuth {
  static const MethodChannel _channel =
      const MethodChannel('desktop_local_auth');

  static Future<String> get platformVersion async {
    final String version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }

  static Future<bool> get isSupported async {
    var isSupported = false;
    if (Platform.isWindows) {
      isSupported = true;
    }
    return isSupported;
  }

  static Future<bool> isValidUser(String caption, String message) async {
    if (caption?.isEmpty ?? true) {
      throw ("Caption is required");
    }

    if (message?.isEmpty ?? true) {
      throw ("Message is required");
    }

    final bool isVerified =
        await _channel.invokeMethod('verifyUser', [caption, message]);
    return isVerified;
  }
}
