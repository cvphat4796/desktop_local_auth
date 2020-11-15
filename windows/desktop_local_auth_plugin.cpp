#include "include/desktop_local_auth/desktop_local_auth_plugin.h"

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
#include <wincred.h>

namespace {

bool verifyUser(std::string strCaption, std::string strMessage) {
  std::wstring caption = std::wstring(strCaption.begin(), strCaption.end());
  std::wstring message = std::wstring(strMessage.begin(), strMessage.end());
  bool isValid = FALSE;
  HWND handle = GetActiveWindow();
  DWORD dwResult;
  DWORD err = 0;
  PVOID pvInAuthBlob = NULL;
  ULONG cbInAuthBlob = 0;
  PVOID pvAuthBlob = NULL;
  ULONG cbAuthBlob = 0;
  CREDUI_INFOW ui;
  ULONG ulAuthPackage = 0;
  BOOL fSave = FALSE;
  ui.cbSize = sizeof(ui);
  ui.hwndParent = handle;
  ui.pszMessageText = message.c_str(); //messageText;
  ui.pszCaptionText = caption.c_str();//captionText.c_str();
  ui.hbmBanner = NULL;
  BOOL continueAsk = TRUE;
  do
  {
    dwResult = CredUIPromptForWindowsCredentialsW(
        &ui,                               // Customizing information
        err,                               // Error code to display
        &ulAuthPackage,                    // Authorization package
        pvInAuthBlob,                      // Credential byte array
        cbInAuthBlob,                      // Size of credential input buffer
        &pvAuthBlob,                       // Output credential byte array
        &cbAuthBlob,                       // Size of credential byte array
        &fSave,                            // Select the save check box.
        CREDUIWIN_ENUMERATE_CURRENT_USER); //CREDUIWIN_AUTHPACKAGE_ONLY

    if (dwResult == ERROR_SUCCESS)
    {
      DWORD dwUserLength = 0;
      DWORD dwDomainLength = 0;
      DWORD dwPasswordLength = 0;

      if (!::CredUnPackAuthenticationBufferW(CRED_PACK_PROTECTED_CREDENTIALS, pvAuthBlob, cbAuthBlob, nullptr, &dwUserLength, nullptr, &dwDomainLength, nullptr, &dwPasswordLength) && ERROR_INSUFFICIENT_BUFFER == ::GetLastError())
      {
        std::vector<wchar_t> bufferUser(dwUserLength);
        std::vector<wchar_t> bufferDomain(dwDomainLength);
        std::vector<wchar_t> bufferPassword(dwPasswordLength);
        if (::CredUnPackAuthenticationBufferW(CRED_PACK_PROTECTED_CREDENTIALS, pvAuthBlob, cbAuthBlob, bufferUser.data(), &dwUserLength, bufferDomain.data(), &dwDomainLength, bufferPassword.data(), &dwPasswordLength))
        {
          HANDLE hToken;
          std::wstring strUsername = bufferUser.data();
          std::wstring strDomain;
          if (bufferDomain.size() == 0)
          {
            std::wstring::size_type pos = strUsername.find(L'\\');
            if (pos != std::wstring::npos)
            {
              strDomain = strUsername.substr(0, pos);
              strUsername = strUsername.substr(pos + 1, strUsername.size() - pos - 1);
            }
          }
          else
          {
            strDomain = bufferDomain.data();
          }

          if (::LogonUserW(strUsername.c_str(), strDomain.c_str(), bufferPassword.data(), LOGON32_LOGON_UNLOCK, LOGON32_PROVIDER_DEFAULT, &hToken))
          {
            isValid = TRUE;
            continueAsk = FALSE;
          }
          else
          {
            err = ERROR_LOGON_FAILURE;
          }
        }
      }
    }
    else if (dwResult == ERROR_CANCELLED)
    {
      continueAsk = FALSE;
    }
    else
    {
      continueAsk = FALSE;
    }
  } while (continueAsk);

  return isValid;
}

class DesktopLocalAuthPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  DesktopLocalAuthPlugin();

  virtual ~DesktopLocalAuthPlugin();

 private:
  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

// static
void DesktopLocalAuthPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "desktop_local_auth",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<DesktopLocalAuthPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

DesktopLocalAuthPlugin::DesktopLocalAuthPlugin() {}

DesktopLocalAuthPlugin::~DesktopLocalAuthPlugin() {}

void DesktopLocalAuthPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  // Replace "getPlatformVersion" check with your plugin's method.
  // See:
  // https://github.com/flutter/engine/tree/master/shell/platform/common/cpp/client_wrapper/include/flutter
  // and
  // https://github.com/flutter/engine/tree/master/shell/platform/glfw/client_wrapper/include/flutter
  // for the relevant Flutter APIs.
  std::string methodName = method_call.method_name();
  if (methodName.compare("getPlatformVersion") == 0) {
    std::ostringstream version_stream;
    version_stream << "Windows ";
    if (IsWindows10OrGreater()) {
      version_stream << "10+";
    } else if (IsWindows8OrGreater()) {
      version_stream << "8";
    } else if (IsWindows7OrGreater()) {
      version_stream << "7";
    }
    result->Success(flutter::EncodableValue(version_stream.str()));
  }
  else if(methodName.compare("verifyUser") == 0){
    const auto *frame_list = std::get_if<flutter::EncodableList>(method_call.arguments());
    if (!frame_list || frame_list->size() != 2) {
      result->Error("Bad arguments", "Expected 2-element list");
      return;
    }
    
    std::string strCaption = std::get<std::string>(frame_list->at(0));
    std::string strMessage = std::get<std::string>(frame_list->at(1));

    bool isValid = verifyUser(strCaption, strMessage);
    result->Success(flutter::EncodableValue(isValid));
  } else {
    result->NotImplemented();
  }
}

}  // namespace

void DesktopLocalAuthPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  DesktopLocalAuthPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
