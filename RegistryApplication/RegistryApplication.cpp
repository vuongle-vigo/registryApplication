//////////////////////////////////////////////////////////////////////////////////////////////
// Description: Registry Application
// Author: Vigo0x1
// Copyright. All rights reserved
// Additional information: N/A
// Run in windows 64 bits
//////////////////////////////////////////////////////////////////////////////////////////////

#include "framework.h"
#include "RegistryApplication.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE g_hInst;                                // current instance
WCHAR g_szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR g_szWindowClass[MAX_LOADSTRING];            // the main window class name

HWND g_hEditPathReg, g_hListView, g_hTreeView, g_hSplitter;
HKEY g_rootKeys[] = { HKEY_CLASSES_ROOT, HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE, HKEY_USERS, HKEY_CURRENT_CONFIG };
LPCTSTR g_rootKeyNames[] = {L"HKEY_CLASSES_ROOT", L"HKEY_CURRENT_USER", L"HKEY_LOCAL_MACHINE", L"HKEY_USERS", L"HKEY_CURRENT_CONFIG"};

int g_IoldWidth, g_IoldHeight;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void AddKeysToTreeView(HWND hwndTV, HKEY hRootKey, LPCTSTR lpSubKey, HTREEITEM hParent);
DWORD WINAPI AddKeysThreadFunc(LPVOID lpParam);
void AddListViewColumns(HWND g_hListView);
void AddListViewItems(HWND g_hListView, HKEY hKey);
void GetFullPath(HTREEITEM hItem, WCHAR* buffer, size_t bufferSize);
DWORD WINAPI AddListViewItemsThreadFunc(LPVOID lpParam);

//struct store infor of parametter thread
struct ThreadDataTreeview {
     HWND m_hwndTV;
     HKEY m_rootKey;
     LPCTSTR m_pszRootKeyName;
     HTREEITEM m_hTreeParrent;
};

// Entry point of the application
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow) {

     
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, g_szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_REGISTRYAPPLICATION, g_szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_REGISTRYAPPLICATION));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_REGISTRYAPPLICATION));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_REGISTRYAPPLICATION);
    wcex.lpszClassName  = g_szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//-------------------------------------------------------------------------------------------------------------
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//   In this function, we save the instance handle in a global variable and
//   create and display the main program window.
//-------------------------------------------------------------------------------------------------------------
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   g_hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(g_szWindowClass, g_szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

////-------------------------------------------------------------------------------------------------------------
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
////-------------------------------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
     static BOOL bDragging = FALSE;
    switch (message)
    {
     case WM_CREATE:
     {
          RECT rect;
          GetWindowRect(hWnd, &rect);
          int width = rect.right - rect.left;
          int heigh = rect.bottom - rect.top;

          g_hTreeView = CreateWindowEx(0, WC_TREEVIEW, L"Tree View", WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS,
               0, 20, 200, heigh - 80, hWnd, (HMENU)ID_TREEVIEWREG, g_hInst, NULL);
          g_hEditPathReg = CreateWindowA("EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_OEMCONVERT,
               0, 0, width, 20, hWnd, NULL, g_hInst, NULL);
          g_hListView = CreateWindowW(WC_LISTVIEW, L"List View", WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT,
               205, 20, width - 205, heigh - 80, hWnd, (HMENU)ID_LISTVIEWREG, g_hInst, NULL);
          g_hSplitter = CreateWindowEx(0, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_NOTIFY,
               200, 20, 5, heigh - 80, hWnd, (HMENU)ID_SPLITTER, g_hInst, NULL);
          
          HFONT hFont = CreateFont(15, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, ANSI_CHARSET,
               OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
               DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

          //Set font to treeview, listview, editbox
          SendMessage(g_hTreeView, WM_SETFONT, (WPARAM)hFont, TRUE);
          SendMessage(g_hListView, WM_SETFONT, (WPARAM)hFont, TRUE);
          SendMessage(g_hEditPathReg, WM_SETFONT, (WPARAM)hFont, TRUE);

//Comment: Get font of system and set
          /*NONCLIENTMETRICS ncm;
          ncm.cbSize = sizeof(NONCLIENTMETRICS);
          SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
          HFONT hFont = CreateFontIndirect(&ncm.lfMessageFont);*/
          
          /*NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
          if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0))
          {
               HFONT hFont = CreateFontIndirect(&ncm.lfMessageFont);
               if (hFont)
               {
                    SendMessage(g_hTreeView, WM_SETFONT, (WPARAM)hFont, TRUE);
                    SendMessage(g_hListView, WM_SETFONT, (WPARAM)hFont, TRUE);
                    SendMessage(g_hEditPathReg, WM_SETFONT, (WPARAM)hFont, TRUE);
               }
          }*/
         
          // Create ImageList
          HIMAGELIST hImageListTreeView = ImageList_Create(16, 16, ILC_COLOR16 | ILC_MASK, 2, 0);

          // Add icon to ImageList

          HICON hIconFolder = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_TREEVIEW_FOLDER), IMAGE_ICON, 16, 16, NULL);
          HICON hIconComputer = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_TREEVIEW_COMPUTER), IMAGE_ICON, 16, 16, NULL);
          
          ImageList_AddIcon(hImageListTreeView, hIconFolder);
          ImageList_AddIcon(hImageListTreeView, hIconComputer);

          //Attach image to treeview
          SendMessage(g_hTreeView, TVM_SETIMAGELIST, 0, (LPARAM)hImageListTreeView);

          HIMAGELIST hImageListListView = ImageList_Create(16, 16, ILC_COLOR16 | ILC_MASK, 2, 0);

          HICON hIconBinaryType = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_LISTVIEW_NUMBER), IMAGE_ICON, 16, 16, NULL);
          HICON hIconTextType = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_LISTVIEW_TEXT), IMAGE_ICON, 16, 16, NULL);

          ImageList_AddIcon(hImageListListView, hIconBinaryType);
          ImageList_AddIcon(hImageListListView, hIconTextType);
          
          //Attach image to listview
          SendMessage(g_hListView, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)hImageListListView);
          
          AddListViewColumns(g_hListView);                                   
          
          //treeview struct
          TVINSERTSTRUCT tvInsert = { 0 };
          tvInsert.hParent = TVI_ROOT;                     //it's is parrent
          tvInsert.hInsertAfter = TVI_LAST;
          tvInsert.item.mask = TVIF_TEXT | TVIF_STATE;
          tvInsert.item.pszText = (LPWSTR)L"Computer";
          tvInsert.item.stateMask = TVIS_STATEIMAGEMASK;
          tvInsert.item.state = INDEXTOSTATEIMAGEMASK(1);  // 1 for closed folder, 2 for open folder

          //tvInsert.item.iImage = tvInsert.item.iSelectedImage = 0;
          HTREEITEM hRootComputerItem = TreeView_InsertItem(g_hTreeView, &tvInsert);

          //treeview item struct
          TVITEM tvi;
          tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
          tvi.hItem = hRootComputerItem;
          tvi.iImage = tvi.iSelectedImage = 1;
          TreeView_SetItem(g_hTreeView, &tvi);
          //SendMessage(g_hTreeView, TVM_SETITEM, 0, (LPARAM)&tvi);

          for (int i = 0; i < 5; i++) {
               ThreadDataTreeview* data = new ThreadDataTreeview;
               data->m_hwndTV = g_hTreeView;
               data->m_rootKey = g_rootKeys[i];
               data->m_pszRootKeyName = g_rootKeyNames[i];
               data->m_hTreeParrent = hRootComputerItem;

               HANDLE hThread = CreateThread(NULL, 0, AddKeysThreadFunc, data, 0, NULL);
               if (hThread == NULL) {
                    MessageBox(NULL, L"Error start thread load RegisterKey to Treeview", NULL, NULL);
               }
               //Sleep to print out in the correct order
               Sleep(100);
          }

     }
     break;
     case WM_SETCURSOR:
     {
          if ((HWND)wParam == g_hSplitter && LOWORD(lParam) == HTCLIENT)
          {
               //Set cursor if this point to splitter
               SetCursor(LoadCursor(NULL, IDC_SIZEWE));
               return TRUE;
          }
          else {
               return DefWindowProc(hWnd, message, wParam, lParam);
          }
     }
     break;

     //case WM_RBUTTONDOWN:
     //{
     //     if ((HWND)wParam == g_hSplitter)
     //     {
     //          //Start dragging splitter
     //          SetCapture(hWnd);
     //          bDragging = TRUE;
     //     }
     //}
     //break;

     case WM_MOUSEMOVE:
     {
          if (GetCapture() == hWnd && bDragging)
          {    
               POINT pt;

               //Get the coordinates of the cursor relative to the screen
               GetCursorPos(&pt);

               //Convert to coordinates relative to the application window
               ScreenToClient(hWnd, &pt);
               
               int x = pt.x; 
               
               RECT rect;

               //Get the coordinates of the outer border of the window
               GetClientRect(hWnd, &rect);
               if (x < rect.left + 50) {
                    x = rect.left + 50;
               }
               else if (x > rect.right - 50) {
                    x = rect.right - 50;
               }
               int width = rect.right - rect.left; //width from outsize window
               int heigh = rect.bottom - rect.top;
               SetWindowPos(g_hTreeView, NULL, 0, 20, x, heigh - 20, SWP_NOZORDER | SWP_NOMOVE);
               SetWindowPos(g_hSplitter, NULL, x, 20, 5, heigh - 20, SWP_NOZORDER);
               SetWindowPos(g_hListView, NULL, x + 5, 20, width - x - 5, heigh - 20, SWP_NOZORDER);
               //Note SWP_NOZORDER, SWP_NOMOVE, SWP_NOSIZE
          }
     }
     break;

     case WM_SIZE:
     {
          int width = LOWORD(lParam); //width from insize window
          int height = HIWORD(lParam);

          RECT rectTreeview;
          GetWindowRect(g_hTreeView, &rectTreeview);
          int widthTreeview = rectTreeview.right - rectTreeview.left;
          //if width modify
          if (width != g_IoldWidth) {
               g_IoldWidth = width;
               SetWindowPos(g_hEditPathReg, NULL, 0, 0, width, 20, SWP_NOZORDER);
               SetWindowPos(g_hListView, NULL, widthTreeview + 5, 20, width - widthTreeview - 5, height - 20, SWP_NOZORDER);
          }
          //if height modify
          else if (height != g_IoldHeight) {
               SetWindowPos(g_hTreeView, NULL, 0, 20, widthTreeview, height - 20, SWP_NOZORDER);
               SetWindowPos(g_hListView, NULL, widthTreeview + 5, 20, width - widthTreeview, height - 20, SWP_NOZORDER);
               SetWindowPos(g_hSplitter, NULL, widthTreeview, 20, 5, height - 20, SWP_NOZORDER);
          }
     }
     break;

     case WM_LBUTTONUP:
     {
          if (GetCapture() == hWnd)
          {
               //set dragging is off
               bDragging = FALSE; 
               ReleaseCapture();
          }
     }
     break;

     case WM_NOTIFY:
     {
          LPNMHDR lpnmh = (LPNMHDR)lParam;
          //Click listview item handle
          if (lpnmh->hwndFrom == g_hTreeView && lpnmh->code == TVN_SELCHANGED)
          {
               LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lParam;

               WCHAR szFullPath[MAX_KEY_LENGTH] = L"";
               GetFullPath(lpnmtv->itemNew.hItem, szFullPath, MAX_KEY_LENGTH);
               szFullPath[wcslen(szFullPath) - 1] = '\0';
               SetWindowText(g_hEditPathReg, szFullPath);
               HKEY hKey = {0};
               wchar_t* wszTmpKeyPath = NULL;
               wchar_t* wszComputerKeyPath = wcstok_s(szFullPath, L"\\", &wszTmpKeyPath);
               wchar_t* wszSubKeyPath = NULL;
               wchar_t* wszRootKeyPath = wcstok_s(wszTmpKeyPath, L"\\", &wszSubKeyPath);
               for (int i = 0; i < 5; i++) {
                    //if rootKeyPath is true
                    if (wszRootKeyPath != NULL && wcscmp(wszRootKeyPath, g_rootKeyNames[i]) == 0) {
                         //Open key with subkeyPath
                         if (RegOpenKeyExW(g_rootKeys[i], wszSubKeyPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
                         {
                              CreateThread(NULL, 0, AddListViewItemsThreadFunc, hKey, 0, NULL);
                         }  
                         break;
                    }
               }
          }
          break;
     }
     // Other cases...
     case WM_COMMAND:
     {
          int wmId = LOWORD(wParam);
          // Parse the menu selections:
          switch (wmId)
          {
          //because splitter using SS_NOTIFY => can't use WM_LBUTTONDOWN
          case ID_SPLITTER:
          {
               if (HIWORD(wParam) == STN_CLICKED)
               {
                    bDragging = TRUE;
                    SetCapture(hWnd);
               }
               break;
          }
          case IDM_ABOUT:
               DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
               break;
          case IDM_EXIT:
               DestroyWindow(hWnd);
               break;
          default:
               return DefWindowProc(hWnd, message, wParam, lParam);
          }
     }
     break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
         return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void AddKeysToTreeView(HWND hwndTV, HKEY hRootKey, LPCTSTR lpSubKey, HTREEITEM hParent)
{
     HKEY hKey;
     if (RegOpenKeyEx(hRootKey, lpSubKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
     {
          TCHAR achKey[MAX_KEY_LENGTH];
          DWORD cbName;

          for (DWORD i = 0; ; i++)
          {
               cbName = MAX_KEY_LENGTH;
               //if don't have key => break;
               if (RegEnumKeyEx(hKey, i, achKey, &cbName, NULL, NULL, NULL, NULL) == ERROR_NO_MORE_ITEMS) {
                    break;
               }
               TVINSERTSTRUCT tvInsert;
               tvInsert.hParent = hParent;                      //parent of this item
               tvInsert.hInsertAfter = TVI_LAST;
               tvInsert.item.mask = TVIF_TEXT | TVIF_STATE;
               tvInsert.item.pszText = achKey;
               tvInsert.item.stateMask = TVIS_STATEIMAGEMASK;
               tvInsert.item.state = INDEXTOSTATEIMAGEMASK(1);  // 1 for closed folder, 2 for open folder
               HTREEITEM hNewItem = TreeView_InsertItem(hwndTV, &tvInsert);
               //else recursive func
               AddKeysToTreeView(hwndTV, hKey, achKey, hNewItem);
          }
          RegCloseKey(hKey);
     }
     else {

          return;
     }

     return;
}

//Func add key to treeview
DWORD WINAPI AddKeysThreadFunc(LPVOID lpParam)
{
     ThreadDataTreeview* threadDataTreeview = (ThreadDataTreeview*)lpParam;

     //Create a TVINSERTSTRUCT cho root key
     TVINSERTSTRUCT tvInsert = {0};
     tvInsert.hParent = threadDataTreeview->m_hTreeParrent;
     tvInsert.hInsertAfter = TVI_LAST;
     tvInsert.item.mask = TVIF_TEXT | TVIF_STATE;
     tvInsert.item.pszText = (LPWSTR)threadDataTreeview->m_pszRootKeyName;
     tvInsert.item.stateMask = TVIS_STATEIMAGEMASK;
     tvInsert.item.state = INDEXTOSTATEIMAGEMASK(1);  // 1 for closed folder, 2 for open folder
     HTREEITEM hNewItem = TreeView_InsertItem(threadDataTreeview->m_hwndTV, &tvInsert);

     //Add subkey vào TreeView
     AddKeysToTreeView(threadDataTreeview->m_hwndTV, threadDataTreeview->m_rootKey, NULL, hNewItem);

     delete threadDataTreeview;

     return 0;
}

//Func add column for listview
void AddListViewColumns(HWND hwndLV)
{
     LVCOLUMN lvc = { 0 };

     lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

     lvc.pszText = (LPWSTR)L"Name";
     lvc.cx = 225;
     ListView_InsertColumn(hwndLV, 0, &lvc);

     lvc.pszText = (LPWSTR)L"Type";
     lvc.cx = 125;
     ListView_InsertColumn(hwndLV, 1, &lvc);

     lvc.pszText = (LPWSTR)L"Data";
     lvc.cx = 300;
     ListView_InsertColumn(hwndLV, 2, &lvc);
}

//Func thread add item for listview
DWORD WINAPI AddListViewItemsThreadFunc(LPVOID lpParam)
{
     HKEY hKey = (HKEY)lpParam;
     AddListViewItems(g_hListView, hKey);
     return 0;
}

//-------------------------------------------------------------------------------------------------------------
// Name: void AddListViewItems(HWND hwndLV, HKEY hKey)
// Description: Read each key, get the value, process the value and add to the listview
// Parameters: HWND hwndLV: Handle of listview
//			HKEY hKey: hKey to get value
// Return: N/A
//-------------------------------------------------------------------------------------------------------------
void AddListViewItems(HWND hwndLV, HKEY hKey)
{        
     DWORD maxValueNameLen, maxValueDataLen;
     
     DWORD valueNameSize, valueDataSize, valueType;
     DWORD i = 0;
     
     ListView_DeleteAllItems(hwndLV);
     LVITEM lvi = { 0 };
     bool bDefaultFlag = FALSE;
     while (true) {
          //MessageBox(NULL, L"while", NULL, NULL);
          if (RegQueryInfoKeyW(hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &maxValueNameLen, &maxValueDataLen, NULL, NULL) != ERROR_SUCCESS) {
               break;
          }
          WCHAR* valueName = new WCHAR[maxValueNameLen + 1];  // +1 for null terminator
          BYTE* valueData = new BYTE[maxValueDataLen];
          valueNameSize = maxValueNameLen + 1;
          valueDataSize = maxValueDataLen;

          DWORD ret = RegEnumValueW(hKey, i++, valueName, &valueNameSize, NULL, &valueType, valueData, &valueDataSize);
          if (ret == ERROR_NO_MORE_ITEMS && !bDefaultFlag) {
               lvi.mask = LVIF_TEXT | LVIF_IMAGE;
               lvi.pszText = (LPWSTR)L"(Default)";
               lvi.iItem = 0;
               lvi.iImage = 1;
               //if value is default and not set
               ListView_InsertItem(hwndLV, &lvi);
               ListView_SetItemText(hwndLV, lvi.iItem, 1, (LPWSTR)L"REG_SZ");
               ListView_SetItemText(hwndLV, lvi.iItem, 2, (LPWSTR)L"(Value not set)");
               break;
          }
          if (ret == ERROR_SUCCESS) {    
               //if value is default and set
               if (wcslen(valueName) == 0 && valueDataSize != 0) {
                    wcscpy_s(valueName, sizeof("(Default)"), L"(Default)");
                    bDefaultFlag = TRUE;
               }
               lvi.mask = LVIF_TEXT | LVIF_IMAGE;
               lvi.pszText = (LPWSTR)valueName;
               lvi.iItem = i - 1;
               lvi.iImage = 0;
               //switch type of value
               switch (valueType)
               {    
                    case REG_BINARY: {
                         ListView_InsertItem(hwndLV, &lvi);
                         ListView_SetItemText(hwndLV, lvi.iItem, 1, (LPWSTR)L"REG_BINARY"); 
                         break;
                    }
                    case REG_DWORD: {
                         ListView_InsertItem(hwndLV, &lvi);
                         ListView_SetItemText(hwndLV, lvi.iItem, 1, (LPWSTR)L"REG_DWORD"); 
                         break;
                    }
                    case REG_QWORD: {
                         ListView_InsertItem(hwndLV, &lvi);
                         ListView_SetItemText(hwndLV, lvi.iItem, 1, (LPWSTR)L"REG_QWORD"); 
                         break;
                    }
                    case REG_MULTI_SZ: {
                         lvi.iImage = 1;
                         ListView_InsertItem(hwndLV, &lvi);
                         ListView_SetItemText(hwndLV, lvi.iItem, 1, (LPWSTR)L"REG_MULTI_SZ"); 
                         break;
                    }
                    case REG_SZ: {
                         lvi.iImage = 1;
                         ListView_InsertItem(hwndLV, &lvi);
                         ListView_SetItemText(hwndLV, lvi.iItem, 1, (LPWSTR)L"REG_SZ"); 
                         break;   
                    }
                    case REG_EXPAND_SZ: {
                         lvi.iImage = 1;
                         ListView_InsertItem(hwndLV, &lvi);
                         ListView_SetItemText(hwndLV, lvi.iItem, 1, (LPWSTR)L"REG_EXPAND_SZ"); 
                         break;
                    }
                    default: {
                         lvi.iImage = 1;
                         ListView_InsertItem(hwndLV, &lvi);
                         ListView_SetItemText(hwndLV, lvi.iItem, 1, (LPWSTR)L"Unknown"); 
                         break;
                    }
               }

               // If value data is Unicode string
               if (valueType == REG_SZ || valueType == REG_MULTI_SZ) {
                    ListView_SetItemText(hwndLV, lvi.iItem, 2, (LPWSTR)valueData);
               }
               // If value data is DWORD
               else if (valueType == REG_DWORD) {
                    DWORD dwValue = *(DWORD*)valueData;
                    WCHAR wszBuffer[MAX_BUFF_LENGTH];  // Buffer that holds the string representation of the DWORD
                    wsprintf(wszBuffer, L"%lu", dwValue);  // Convert the DWORD to a string
                    ListView_SetItemText(hwndLV, lvi.iItem, 2, wszBuffer);
               }
               // If value data is QWORD
               else if (valueType == REG_QWORD) {
                    ULONGLONG qwValue = *(ULONGLONG*)valueData;
                    WCHAR wszBuffer[MAX_BUFF_LENGTH];  // Buffer that holds the string representation of the QWORD
                    wsprintf(wszBuffer, L"%llu", qwValue);  // Convert the QWORD to a string
                    ListView_SetItemText(hwndLV, lvi.iItem, 2, wszBuffer);
               }
               // If value data is BINARY
               else if (valueType == REG_BINARY) {
                    WCHAR wszBuffer[MAX_BUFF_LENGTH];  // Buffer that holds the string representation of the binary data
                    for (DWORD i = 0; i < valueDataSize; i++) {
                         wsprintf(wszBuffer + 3 * i, L"%02X ", valueData[i]);  // Convert each byte to a string
                    }
                    ListView_SetItemText(hwndLV, lvi.iItem, 2, wszBuffer);
               }
               //If value data is EXPAND_SZ
               else if (valueType == REG_EXPAND_SZ) {
                    wstring wsExpandedData;
                    wsExpandedData.resize(MAX_PATH);
                    if (ExpandEnvironmentStringsW((LPWSTR)valueData, (LPWSTR)wsExpandedData.c_str(), MAX_PATH) > 0) {
                         ListView_SetItemText(hwndLV, lvi.iItem, 2, (LPWSTR)wsExpandedData.c_str());
                    }
                    else {
                         // Handle error...
                    }
               }
               // Other data types...
               else {
                    MessageBox(NULL, L"Other data types", NULL, NULL);
               }       
          }
          else {
               break;
          }
     }
     RegCloseKey(hKey);
}

void GetFullPath(HTREEITEM hItem, WCHAR* buffer, size_t bufferSize)
{
     WCHAR text[256] = L"";
     TVITEM tvItem = {0};
 
     // Get the name of the current item
     tvItem.hItem = hItem;
     tvItem.mask = TVIF_TEXT;
     tvItem.pszText = text;
     tvItem.cchTextMax = sizeof(text) / sizeof(WCHAR);
     if (TreeView_GetItem(g_hTreeView, &tvItem))
     {
          // Check if buffer has enough space for the new segment
          size_t textLen = wcslen(text);
          if (bufferSize < textLen + 1)
          {
               // Not enough space, exit
               return;
          }

          // Move the rest of the string to make space for the new segment
          memmove(buffer + textLen + 1, buffer, bufferSize - (textLen + 1) * sizeof(WCHAR));

          // Insert the new segment at the start of the string
          memcpy(buffer, text, textLen * sizeof(WCHAR));
          buffer[textLen] = L'\\';  // Append a backslash

          // Call the function recursively with the parent item
          HTREEITEM hParentItem = TreeView_GetParent(g_hTreeView, hItem);
          if (hParentItem)
          {
               GetFullPath(hParentItem, buffer, bufferSize - (textLen + 1));
          }
     }
}