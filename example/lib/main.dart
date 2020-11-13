import 'package:flutter/material.dart';
import 'dart:async';

import 'package:flutter/services.dart';
import 'package:desktop_local_auth/desktop_local_auth.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String _message = 'Click Float button to request user auth';
  bool _isSupported = false;

  @override
  void initState() {
    super.initState();
    checkSupported();
  }

  Future<void> checkSupported() async {
    try {
      var isSupported = await DesktopLocalAuth.isSupported;
      if (!isSupported) {
        setState(() {
          _message = "Platform is not supported";
        });
      }
      setState(() {
        _isSupported = isSupported;
      });
    } on PlatformException {
      print('PlatformException');
    }
  }

  Future<void> askPermission() async {
    String message = '';
    try {
      if (_isSupported) {
        var caption = "Caption Windows Security";
        var messageText =
            "The Application is trying to show your password. Please enter your Windows password to allow this";
        var isValidUser =
            await DesktopLocalAuth.isValidUser(caption, messageText);
        if (isValidUser) {
          message = 'User logon success';
        } else {
          message = 'User logon fail';
        }
      } else {
        message = "Platform is not supported";
      }
    } on PlatformException {
      print('PlatformException');
    }

    // If the widget was removed from the tree while the asynchronous platform
    // message was in flight, we want to discard the reply rather than calling
    // setState to update our non-existent appearance.
    if (!mounted) return;

    setState(() {
      _message = message;
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        body: Center(
          child: Text(_message),
        ),
        floatingActionButton: FloatingActionButton(
          onPressed: askPermission,
          tooltip: 'ask permission',
          child: Icon(Icons.add),
        ),
      ),
    );
  }
}
