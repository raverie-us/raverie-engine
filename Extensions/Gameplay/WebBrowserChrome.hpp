///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// NOTE: This should only ever be included by one cpp (implementation generally hidden)
#include <include/cef_app.h>
#include <include/cef_browser.h>
#include <include/cef_client.h>

#include <include/base/cef_bind.h>
#include <include/cef_app.h>
#include <include/wrapper/cef_closure_task.h>
#include <include/wrapper/cef_helpers.h>
#include <include/cef_browser.h>
#include <include/cef_browser_process_handler.h>
#include <include/wrapper/cef_helpers.h>

#pragma comment(lib, "libcef.lib")

#if defined(ZeroDebug)
#pragma comment(lib, "libcef_dll_wrapper_debug.lib")
#else
#pragma comment(lib, "libcef_dll_wrapper_release.lib")
#endif

namespace Zero
{

namespace Events
{
  DeclareEvent(ChromePopup);
}

class ChromeBrowserData
{
public:
  WebBrowser* mWebBrowser;
  CefRefPtr<CefBrowser> mCefBrowser;
  bool mFocus;
  bool mVisible;
};

class ChromePopupEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CefRefPtr<CefBrowser> mBrowser;
  CefRefPtr<CefFrame> mFrame;
  String mTargetUrl;
  String mTargetFrameName;
  CefLifeSpanHandler::WindowOpenDisposition mTargetDisposition;
  bool mUserGesture;
  CefPopupFeatures mPopupFeatures;
  CefWindowInfo mWindowInfo;
  CefRefPtr<CefClient> mClient;
  CefBrowserSettings mSettings;
};

class Chrome :
  public ThreadSafeId32EventObject,
  public CefClient,
  public CefDisplayHandler,
  public CefLifeSpanHandler,
  public CefLoadHandler,
  public CefRenderHandler,
  public CefRequestHandler,
  public CefDownloadHandler
{
public:
  ZilchDeclareDerivedTypeExplicit(Chrome, ThreadSafeId32EventObject, TypeCopyMode::ReferenceType);
  Chrome();
  ~Chrome();

  // CefClient Interface
  CefRefPtr<CefDisplayHandler>  GetDisplayHandler()   override { return this; }
  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler()  override { return this; }
  CefRefPtr<CefLoadHandler>     GetLoadHandler()      override { return this; }
  CefRefPtr<CefRenderHandler>   GetRenderHandler()    override { return this; }
  CefRefPtr<CefRequestHandler>  GetRequestHandler()   override { return this; }
  CefRefPtr<CefDownloadHandler> GetDownloadHandler()  override { return this; }
  
  // CefRenderHandler Interface
  bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect & rect) override;
  void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList & dirtyRects, const void * buffer, int width, int height) override;
  bool GetScreenPoint
  (
    CefRefPtr<CefBrowser> browser,
    int viewX,
    int viewY,
    int& screenX,
    int& screenY
  ) override;
  void OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, CursorType type, const CefCursorInfo& custom_cursor_info) override;

  // CefDisplayHandler Interface
  void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;
  void OnStatusMessage(CefRefPtr<CefBrowser> browser, const CefString& value) override;
  bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line) override;

  // CefLifeSpanHandler Interface
  bool OnBeforePopup
  (
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    const CefString& target_url,
    const CefString& target_frame_name,
    CefLifeSpanHandler::WindowOpenDisposition target_disposition,
    bool user_gesture,
    const CefPopupFeatures& popupFeatures,
    CefWindowInfo& windowInfo,
    CefRefPtr<CefClient>& client,
    CefBrowserSettings& settings,
    bool* no_javascript_access
  ) override;
  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  bool DoClose(CefRefPtr<CefBrowser> browser) override;
  void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

  // CefLoadHandler Interface
  void OnLoadError(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    ErrorCode errorCode,
    const CefString& errorText,
    const CefString& failedUrl) override;

  // CefRequestHandler Interface
  bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_redirect);

  // CefDownloadHandler Interface
  void OnBeforeDownload
  (
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDownloadItem> download_item,
    const CefString& suggested_name,
    CefRefPtr<CefBeforeDownloadCallback> callback
  ) override;
  void OnDownloadUpdated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback) override;

  void OnChromePopup(ChromePopupEvent* event);

  // Maps from internal platform browsers to our WebBrowser
  ChromeBrowserData& GetWebBrowserData(CefRefPtr<CefBrowser> browser);
  ChromeBrowserData& GetWebBrowserData(WebBrowser* webBrowser);
  WebBrowser* GetWebBrowser(CefRefPtr<CefBrowser> browser);
  CefRefPtr<CefBrowser> GetCefBrowser(WebBrowser* webBrowser);
  HashMap<int, ChromeBrowserData> mIdToData;
  HashMap<int, CefRefPtr<CefDownloadItem> > mIdToDownload;

  // We are currently creating a CefBrowser for this WebBrowser
  WebBrowser* mCreatedWebBrowser;

  IMPLEMENT_REFCOUNTING(Chrome);
};

template <typename ZilchLibrary>
void WebBrowserManager::PlatformInitializeMeta()
{
  ZilchInitializeType(Chrome);
}

} // namespace Zero
