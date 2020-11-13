# desktop_local_auth

A new flutter plugin project.

## Check if device supports

```dart
// Currently only support windows
bool isSupported = await DesktopLocalAuth.isSupported;
```

## Request authorization

```dart
// caption and message is required
String caption = "Caption Windows Security";
String message = "The Application is trying to show your password. Please enter your Windows password to allow this";
bool isValidUser = await DesktopLocalAuth.isValidUser(caption, message);
```