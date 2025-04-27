#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")

#include <stdbool.h>
#include <stdio.h>

#include <windows.h>

#include "GetMsgName.h"

#define LOG(fmt, ...) do { \
    fprintf(stderr, fmt "\n", ##__VA_ARGS__); \
    fflush(stderr); \
} while (0)

#define UNREACHABLE() do { \
    LOG("line %d in file %s should be unreachable", __LINE__, __FILE__); \
    abort(); \
} while (0)

#define ENFORCE(expr) do { \
    if (!(expr)) { \
        LOG("ENFORCE failed: %s, file %s, line %d", #expr, __FILE__, __LINE__); \
        abort(); \
    } \
} while (0)
#define ENFORCE_EQ(value_prefix, spec, expected, actual) do { \
    if ((expected) != (actual)) { \
        LOG("%s:%d: %s != %s (" value_prefix spec " != " value_prefix spec ")", __FILE__, __LINE__, #expected, #actual, expected, actual); \
        abort(); \
    } \
} while (0)
#define FATAL_WIN32(what, code) do { \
    LOG("%s failed, error=%u", what, code); \
    fflush(stderr); \
    abort(); \
} while (0)


static void append_str(char* s, size_t* offset, const char sep, const char* append_str)
{
    const size_t append_len = strlen(append_str);
    if (*offset > 0) {
        s[*offset] = sep;
        *offset += 1;
    }
    memcpy(s + *offset, append_str, append_len);
    *offset += append_len;
}
static bool consume_flag(DWORD* flags, DWORD flag)
{
    if (*flags & flag) {
        *flags &= ~flag;
        return true;
    }
    return false;
}

static const DWORD WND_STYLE_ALL = (
    WS_TABSTOP
    | WS_MINIMIZEBOX
    | WS_SIZEBOX
    | WS_SYSMENU
    | WS_HSCROLL
    | WS_VSCROLL
    | WS_DLGFRAME
    | WS_BORDER
    | WS_MAXIMIZE
    | WS_CLIPCHILDREN
    | WS_CLIPSIBLINGS
    | WS_DISABLED
    | WS_VISIBLE
    | WS_MINIMIZE
    | WS_CHILD
    | WS_POPUP
);

// This is calcualted by calling format_wnd_style with 0xffffffff and adding 1
#define FORMAT_WND_STYLE_BUF_LEN ((size_t)147)

static size_t format_wnd_style(char* out, DWORD style)
{
    DWORD remaining = style;
    size_t offset = 0;
    if (consume_flag(&remaining, WS_TABSTOP)) append_str(out, &offset, ',', "TABSTOP");
    if (consume_flag(&remaining, WS_MINIMIZEBOX)) append_str(out, &offset, ',', "MINBOX");
    if (consume_flag(&remaining, WS_SIZEBOX)) append_str(out, &offset, ',', "SIZEBOX");
    if (consume_flag(&remaining, WS_SYSMENU)) append_str(out, &offset, ',', "SYSMENU");
    if (consume_flag(&remaining, WS_HSCROLL)) append_str(out, &offset, ',', "HSCROLL");
    if (consume_flag(&remaining, WS_VSCROLL)) append_str(out, &offset, ',', "VSCROLL");
    if (consume_flag(&remaining, WS_DLGFRAME)) append_str(out, &offset, ',', "DLGFRAME");
    if (consume_flag(&remaining, WS_BORDER)) append_str(out, &offset, ',', "BORDER");
    if (consume_flag(&remaining, WS_MAXIMIZE)) append_str(out, &offset, ',', "MAXIMIZE");
    if (consume_flag(&remaining, WS_CLIPCHILDREN)) append_str(out, &offset, ',', "CLIPCHILDREN");
    if (consume_flag(&remaining, WS_CLIPSIBLINGS)) append_str(out, &offset, ',', "CLIPSIBLINGS");
    if (consume_flag(&remaining, WS_DISABLED)) append_str(out, &offset, ',', "DISABLED");
    if (consume_flag(&remaining, WS_VISIBLE)) append_str(out, &offset, ',', "VISIBLE");
    if (consume_flag(&remaining, WS_MINIMIZE)) append_str(out, &offset, ',', "MINIMIZE");
    if (consume_flag(&remaining, WS_CHILD)) append_str(out, &offset, ',', "CHILD");
    if (consume_flag(&remaining, WS_POPUP)) append_str(out, &offset, ',', "POPUP");
    if (remaining) {
        if (offset > 0) {
            out[offset] = ',';
            offset += 1;
        }
        offset += sprintf(out + offset, "0x%08x", remaining);
    }
    out[offset] = 0;

    return offset;
}

static const DWORD WND_EX_STYLE_ALL = (
    WS_EX_DLGMODALFRAME
    | WS_EX_NOPARENTNOTIFY
    | WS_EX_TOPMOST
    | WS_EX_ACCEPTFILES
    | WS_EX_TRANSPARENT
    | WS_EX_MDICHILD
    | WS_EX_TOOLWINDOW
    | WS_EX_WINDOWEDGE
    | WS_EX_CLIENTEDGE
    | WS_EX_CONTEXTHELP
    | WS_EX_RIGHT
    | WS_EX_RTLREADING
    | WS_EX_LEFTSCROLLBAR
    | WS_EX_RIGHTSCROLLBAR
    | WS_EX_CONTROLPARENT
    | WS_EX_STATICEDGE
    | WS_EX_APPWINDOW
    | WS_EX_PALETTEWINDOW
    | WS_EX_LAYERED
    | WS_EX_NOINHERITLAYOUT
    | WS_EX_NOREDIRECTIONBITMAP
    | WS_EX_LAYOUTRTL
    | WS_EX_COMPOSITED
    | WS_EX_NOACTIVATE
);

// This is calcualted by calling format_wnd_ex_style with 0xffffffff and adding 1
#define FORMAT_WND_EX_STYLE_BUF_LEN ((size_t)268)

static size_t format_wnd_ex_style(char* out, DWORD ex_style)
{
    DWORD remaining = ex_style;
    size_t offset = 0;
    if (consume_flag(&remaining, WS_EX_DLGMODALFRAME)) append_str(out, &offset, ',', "DLGMODALFRAME");
    // No 0x2 flag
    if (consume_flag(&remaining, WS_EX_NOPARENTNOTIFY)) append_str(out, &offset, ',', "NOPARENTNOTIFY");
    if (consume_flag(&remaining, WS_EX_TOPMOST)) append_str(out, &offset, ',', "TOPMOST");
    if (consume_flag(&remaining, WS_EX_ACCEPTFILES)) append_str(out, &offset, ',', "ACCEPTFILES");
    if (consume_flag(&remaining, WS_EX_TRANSPARENT)) append_str(out, &offset, ',', "TRANSPARENT");
    if (consume_flag(&remaining, WS_EX_MDICHILD)) append_str(out, &offset, ',', "MDICHILD");
    if (consume_flag(&remaining, WS_EX_TOOLWINDOW)) append_str(out, &offset, ',', "TOOLWINDOW");
    if (consume_flag(&remaining, WS_EX_WINDOWEDGE)) append_str(out, &offset, ',', "WINDOWEDGE");
    if (consume_flag(&remaining, WS_EX_CLIENTEDGE)) append_str(out, &offset, ',', "CLIENTEDGE");
    if (consume_flag(&remaining, WS_EX_CONTEXTHELP)) append_str(out, &offset, ',', "CONTEXTHELP");
    // no 0x800 flag
    if (consume_flag(&remaining, WS_EX_RIGHT)) append_str(out, &offset, ',', "RIGHT");
    if (consume_flag(&remaining, WS_EX_RTLREADING)) append_str(out, &offset, ',', "RTLREADING");
    if (consume_flag(&remaining, WS_EX_LEFTSCROLLBAR)) append_str(out, &offset, ',', "LEFTSCROLLBAR");
    // no 0x8000 flag
    if (consume_flag(&remaining, WS_EX_CONTROLPARENT)) append_str(out, &offset, ',', "CONTROLPARENT");
    if (consume_flag(&remaining, WS_EX_STATICEDGE)) append_str(out, &offset, ',', "STATICEDGE");
    if (consume_flag(&remaining, WS_EX_APPWINDOW)) append_str(out, &offset, ',', "APPWINDOW");
    if (consume_flag(&remaining, WS_EX_LAYERED)) append_str(out, &offset, ',', "LAYERED");
    if (consume_flag(&remaining, WS_EX_NOINHERITLAYOUT)) append_str(out, &offset, ',', "NOINHERITLAYOUT");
    if (consume_flag(&remaining, WS_EX_NOREDIRECTIONBITMAP)) append_str(out, &offset, ',', "NOREDIRECTIONBITMAP");
    if (consume_flag(&remaining, WS_EX_LAYOUTRTL)) append_str(out, &offset, ',', "LAYOUTRTL");
    // no 0x0080000 flag
    // no 0x0100000 flag
    if (consume_flag(&remaining, WS_EX_COMPOSITED)) append_str(out, &offset, ',', "COMPOSITED");
    // no 0x0400000 flag
    if (consume_flag(&remaining, WS_EX_NOACTIVATE)) append_str(out, &offset, ',', "NOACTIVATE");
    // no 0x1000000 flag
    // no 0x2000000 flag
    // no 0x4000000 flag
    // no 0x8000000 flag

    if (remaining) {
        if (offset > 0) {
            out[offset] = ',';
            offset += 1;
        }
        offset += sprintf(out + offset, "0x%08x", remaining);
    }
    out[offset] = 0;
    return offset;
}

static const char *showwindow_status_str(LPARAM status)
{
    switch (status) {
    case 0: return "ShowWindow";
    case SW_PARENTCLOSING: return "PARENTCLOSING";
    case SW_OTHERZOOM: return "OTHERZOOM";
    case SW_OTHERUNZOOM: return "OTHERUNZOOM";
    case SW_PARENTOPENING: return "PARENTOPENING";
    default: return "?";
    }
}

// This is calcualted by calling format_swp_flags with 0xffffffff and adding 1
#define FORMAT_SWP_FLAGS_BUF_LEN ((size_t)155)

static size_t format_swp_flags(char* out, UINT flags)
{
    DWORD remaining = flags;
    size_t offset = 0;
    if (consume_flag(&remaining, SWP_NOSIZE)) append_str(out, &offset, ',', "NOSIZE");
    if (consume_flag(&remaining, SWP_NOMOVE)) append_str(out, &offset, ',', "NOMOVE");
    if (consume_flag(&remaining, SWP_NOZORDER)) append_str(out, &offset, ',', "NOZORDER");
    if (consume_flag(&remaining, SWP_NOREDRAW)) append_str(out, &offset, ',', "NOREDRAW");
    if (consume_flag(&remaining, SWP_NOACTIVATE)) append_str(out, &offset, ',', "NOACTIVATE");
    if (consume_flag(&remaining, SWP_FRAMECHANGED)) append_str(out, &offset, ',', "FRAMECHANGED");
    if (consume_flag(&remaining, SWP_SHOWWINDOW)) append_str(out, &offset, ',', "SHOWWINDOW");
    if (consume_flag(&remaining, SWP_HIDEWINDOW)) append_str(out, &offset, ',', "HIDEWINDOW");
    if (consume_flag(&remaining, SWP_NOCOPYBITS)) append_str(out, &offset, ',', "NOCOPYBITS");
    if (consume_flag(&remaining, SWP_NOOWNERZORDER)) append_str(out, &offset, ',', "NOOWNERZORDER");
    if (consume_flag(&remaining, SWP_NOSENDCHANGING)) append_str(out, &offset, ',', "NOSENDCHANGING");
    if (consume_flag(&remaining, SWP_DEFERERASE)) append_str(out, &offset, ',', "DEFERERASE");
    if (consume_flag(&remaining, SWP_ASYNCWINDOWPOS)) append_str(out, &offset, ',', "ASYNCWINDOWPOS");
    if (remaining) {
        if (offset > 0) {
            out[offset] = ',';
            offset += 1;
        }
        offset += sprintf(out + offset, "0x%08x", remaining);
    }
    out[offset] = 0;
    return offset;
}

static const char* ime_notify_code_str(WPARAM code)
{
    switch (code) {
    case IMN_CLOSESTATUSWINDOW: return "CLOSESTATUSWINDOW";
    case IMN_OPENSTATUSWINDOW: return "OPENSTATUSWINDOW";
    case IMN_CHANGECANDIDATE: return "CHANGECANDIDATE";
    case IMN_CLOSECANDIDATE: return "CLOSECANDIDATE";
    case IMN_OPENCANDIDATE: return "OPENCANDIDATE";
    case IMN_SETCONVERSIONMODE: return "SETCONVERSIONMODE";
    case IMN_SETSENTENCEMODE: return "SETSENTENCEMODE";
    case IMN_SETOPENSTATUS: return "SETOPENSTATUS";
    case IMN_SETCANDIDATEPOS: return "SETCANDIDATEPOS";
    case IMN_SETCOMPOSITIONFONT: return "SETCOMPOSITIONFONT";
    case IMN_SETCOMPOSITIONWINDOW: return "SETCOMPOSITIONWINDOW";
    case IMN_GUIDELINE: return "GUIDELINE";
    case IMN_PRIVATE: return "PRIVATE";
    default: return "?";
    }
}

static const char* get_hit_str(WPARAM hit_test_area)
{
    switch (hit_test_area) {
    case HTBORDER: return "BORDER";
    case HTBOTTOM: return "BOTTOM";
    case HTBOTTOMLEFT: return "BOTTOMLEFT";
    case HTBOTTOMRIGHT: return "BOTTOMRIGHT";
    case HTCAPTION: return "CAPTION";
    case HTCLIENT: return "CLIENT";
    case HTCLOSE: return "CLOSE";
    case HTERROR: return "ERROR";
    case HTGROWBOX: return "GROWBOX";
    case HTHELP: return "HELP";
    case HTHSCROLL: return "HSCROLL";
    case HTLEFT: return "LEFT";
    case HTMENU: return "MENU";
    case HTMAXBUTTON: return "MAXBUTTON";
    case HTMINBUTTON: return "MINBUTTON";
    case HTNOWHERE: return "NOWHERE";
    case HTRIGHT: return "RIGHT";
    case HTSYSMENU: return "SYSMENU";
    case HTTOP: return "TOP";
    case HTTOPLEFT: return "TOPLEFT";
    case HTTOPRIGHT: return "TOPRIGHT";
    case HTVSCROLL: return "VSCROLL";
    default: return "?";
    }
}

// --------------------------------------------------------------------------------
// This application
// --------------------------------------------------------------------------------


static const WCHAR* WND_CLASS = L"BasicsWindow";
static const WCHAR* WND_NAME = L"Basics";
static const DWORD WND_STYLE = (
    WS_BORDER
    | WS_DLGFRAME
    | WS_SYSMENU
    | WS_SIZEBOX
    | WS_MINIMIZEBOX
    | WS_MAXIMIZEBOX
);
static const DWORD WND_EX_STYLE = WS_EX_WINDOWEDGE;
#define CREATE_PARAMS_MAGIC ((void*)0x017e3919)

static void CheckHwnd(HWND hwnd)
{
    // TODO: check everything we can about the hwnd, is the style correct?
    //       the size? the Name/Class, etc
}


HWND global_hwnd = NULL;
unsigned global_msg_count = 0;
unsigned global_wnd_pos_changing = 0;
unsigned global_wnd_pos_changed = 0;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    global_msg_count++;

    if (global_hwnd) ENFORCE_EQ("", "%p", global_hwnd, hwnd);
    global_hwnd = hwnd;

    CheckHwnd(hwnd);

    //LOG("WndProc msg=%s(%u)", GetMsgName(msg), msg);
    switch (msg) {
    case WM_NULL: return 0; // WM_NULL == 0
    case WM_CREATE: { // WM_CREATE == 1
        ENFORCE_EQ("", "%u", 4, global_msg_count);
        CREATESTRUCT* create = (CREATESTRUCT*)lparam;
        ENFORCE_EQ("", "%p", CREATE_PARAMS_MAGIC, create->lpCreateParams);
        ENFORCE_EQ("", "%p", GetModuleHandleW(NULL), create->hInstance);
        ENFORCE_EQ("", "%p", NULL, create->hMenu);
        ENFORCE_EQ("", "%p", NULL, create->hwndParent);
        LOG("WM_NCCREATE %d,%d %dx%d", create->x, create->y, create->cx, create->cy);
        {
            char buf[FORMAT_WND_STYLE_BUF_LEN];
            format_wnd_style(buf, create->style);
            LOG("  style=0x%x %s", create->style, buf);
        }
        ENFORCE_EQ("0x", "%x", WND_STYLE, create->style);
        ENFORCE(!wcscmp(WND_NAME, create->lpszName));
        ENFORCE(!wcscmp(WND_CLASS, create->lpszClass));
        {
            char buf[FORMAT_WND_EX_STYLE_BUF_LEN];
            format_wnd_ex_style(buf, create->dwExStyle);
            LOG("  exstyle=0x%x %s", create->dwExStyle, buf);
        }
        ENFORCE_EQ("0x", "%x", WND_EX_STYLE, create->dwExStyle);
        return 0;
    }
    case WM_DESTROY: // WM_DESTROY == 2
        UNREACHABLE();
        return 0;
    case WM_MOVE: { // WM_MOVE == 3
        POINT p = {(short)LOWORD(lparam), (short)HIWORD(lparam)};
        LOG("WM_MOVE %d,%d", p.x, p.y);
        // TODO: paint the window position, so, call invalidate here
        return 0;
    }
    case WM_SIZE: { // WM_SIZE == 5
        WORD width = LOWORD(lparam);
        WORD height = HIWORD(lparam);

        // TODO: verify width/height match size that GetClientRect returns

        // Get the resize type
        WPARAM resize_type = wparam;
        const char* type_str = "UNKNOWN";
        switch (resize_type) {
        case SIZE_MAXIMIZED: type_str = "MAXIMIZED"; break;
        case SIZE_MINIMIZED: type_str = "MINIMIZED"; break;
        case SIZE_RESTORED: type_str = "RESTORED"; break;
        case SIZE_MAXHIDE: type_str = "MAXHIDE"; break;
        case SIZE_MAXSHOW: type_str = "MAXSHOW"; break;
        default: break;
        }
        LOG("WM_SIZE: type=%s (%llu), width=%u, height=%u",
            type_str, resize_type, width, height);
        return 0;
    }
    case WM_ACTIVATE: { // WM_ACTIVATE == 6
        WORD activate_state = LOWORD(wparam);
        WORD minimized = HIWORD(wparam);
        HWND other_window = (HWND)lparam;
        const char* state_str = "UNKNOWN";
        switch (activate_state) {
        case WA_INACTIVE: state_str = "INACTIVE"; break;
        case WA_ACTIVE: state_str = "ACTIVE"; break;
        case WA_CLICKACTIVE: state_str = "CLICKACTIVE"; break;
        default: UNREACHABLE();
        }
        LOG("WM_ACTIVATE: state=%s (%u) minimized=%d otherWindow=%p",
            state_str, activate_state, minimized, other_window);
        // This is where you would handle window activation state changes
        // For example:
        if (activate_state == WA_INACTIVE) {
            // Window is being deactivated
            // You might pause animations or background operations
        } else {
            // Window is being activated (either by mouse click or other means)
            // You might resume animations or background operations
        }
        return 0;
    }
    case WM_SETFOCUS: { // WM_SETFOCUS == 7
        HWND hwnd_prev_focus = (HWND)wparam;
        LOG("WM_SETFOCUS: previous focus=%p", hwnd_prev_focus);

        // This is where you would handle receiving keyboard focus
        // For example:
        // - Creating or showing a caret (text cursor)
        // - Refreshing or highlighting input areas
        // - Starting keyboard-related operations

        // If you need a caret, create it here:
        // CreateCaret(hwnd, NULL, 2, 20); // creates a 2x20 pixel solid caret
        // SetCaretPos(x, y); // position the caret
        // ShowCaret(hwnd); // make the caret visible

        return 0;
    }
    case WM_CLOSE: // WM_CLOSE == 16
        PostQuitMessage(0);
        return 0;
    case WM_ERASEBKGND: { // WM_ERASEBKGND == 14
        HDC hdc = (HDC)wparam;
        ENFORCE(hdc);

        RECT rect;
        if (!GetClientRect(hwnd, &rect)) {
            FATAL_WIN32("GetClientRect", GetLastError());
        }
        ENFORCE_EQ("", "%d", 0, rect.left);
        ENFORCE_EQ("", "%d", 0, rect.top);
        LOG("WM_ERASEBKGND: %dx%d", rect.right, rect.bottom);

        // examples
        // HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
        // FillRect(hdc, &rect, brush);

        // Return TRUE to indicate that the background has been erased
        // This prevents the default handling from also erasing the background
        return TRUE;
    }
    case WM_PAINT: { // WM_PAINT == 15
        PAINTSTRUCT paint;
        HDC hdc = BeginPaint(hwnd, &paint);
        if (!hdc) FATAL_WIN32("BeginPaint", GetLastError());

        if (!EndPaint(hwnd, &paint)) FATAL_WIN32("EndPaint", GetLastError());
        return 0;
    }
    case WM_SHOWWINDOW: { // WM_SHOWWINDOW == 24
        WPARAM show = wparam;
        LPARAM status = (LPARAM)lparam;
        LOG("WM_SHOWWINDOW show=%lld, status=%s (%llu)", show, showwindow_status_str(status), status);
        ENFORCE((show == 0) || (show == 1));
        return 0;
    }
    case WM_ACTIVATEAPP: // WM_ACTIVATEAPP == 28
        WPARAM activate = wparam;
        LPARAM thread_id = lparam;
        ENFORCE((activate == 0) || (activate == 1));
        if (activate) {
            LOG("WM_ACTIVATEAPP: activate (thread %llu)", thread_id);
            // This is where you would handle window activation
            // For example, resuming animations, sounds, or other processing
        } else {
            LOG("WM_ACTIVATEAPP: deactivate (thread %llu)", thread_id);
            // This is where you would handle window deactivation
            // For example, pausing animations, sounds, or other processing
        }
        return 0;
    case WM_SETCURSOR: { // WM_SETCURSOR == 32
        HWND hwnd_cursor = (HWND)wparam;
        WORD hit_test = LOWORD(lparam);
        WORD trigger_msg = HIWORD(lparam);
        LOG("WM_SETCURSOR: hwnd=%p, hitTest=%u, triggerMsg=%u",
            hwnd_cursor, hit_test, trigger_msg);
        if (hit_test == HTCLIENT) {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return TRUE; // Return TRUE to prevent default handling
        }
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    case WM_GETMINMAXINFO: { // WM_GETMINMAXINFO == 36
        if (global_msg_count <= 4) {
            ENFORCE_EQ("", "%u", 1, global_msg_count);
        }
        MINMAXINFO* info = (MINMAXINFO*)lparam;
        LOG(
            "maxsize=%dx%d maxpos=%d,%d mintrack=%dx%d maxtrack=%dx%d",
            info->ptMaxSize.x, info->ptMaxSize.y,
            info->ptMaxPosition.x, info->ptMaxPosition.y,
            info->ptMinTrackSize.x, info->ptMinTrackSize.y,
            info->ptMaxTrackSize.x, info->ptMaxTrackSize.y
        );
        return 0;
    }
    case WM_WINDOWPOSCHANGING: { // WM_WINDOWPOSCHANGING == 70
        global_wnd_pos_changing++;
        WINDOWPOS* winpos = (WINDOWPOS*)lparam;
        LOG(
            "WM_WINDOWPOSCHANGING %d,%d %dx%d hwndInsertAfter=0x%p count=%u",
            winpos->x, winpos->y, winpos->cx, winpos->cy,
            winpos->hwndInsertAfter, global_wnd_pos_changing
        );
        {
            char buf[FORMAT_SWP_FLAGS_BUF_LEN];
            format_swp_flags(buf, winpos->flags);
            LOG("  flags=0x%x %s", winpos->flags, buf);
        }

        // You can modify winpos fields here to influence the window position change
        // For example:
        // if (winpos->cx < 200) winpos->cx = 200; // Enforce minimum width
        // if (winpos->cy < 150) winpos->cy = 150; // Enforce minimum height

        return 0;
    }
    case WM_WINDOWPOSCHANGED: { // WM_WINDOWPOSCHANGED == 71
        global_wnd_pos_changed++;

        WINDOWPOS* winpos = (WINDOWPOS*)lparam;
        LOG(
            "WM_WINDOWPOSCHANGED %d,%d %dx%d hwndInsertAfter=0x%p count=%u",
            winpos->x, winpos->y, winpos->cx, winpos->cy,
            winpos->hwndInsertAfter, global_wnd_pos_changed
        );
        {
            char buf[FORMAT_SWP_FLAGS_BUF_LEN];
            format_swp_flags(buf, winpos->flags);
            LOG("  flags=0x%x %s", winpos->flags, buf);
        }

        // This is where you would handle the finalized window position change
        // For example, you might:
        // - Update your internal state
        // - Recalculate layout if needed
        // - Invalidate areas that need repainting
        // - Notify child controls about size/position changes

        // If the size changed and SWP_NOSIZE was not set
        // if (!(winpos->flags & SWP_NOSIZE)) {
        //     // Invalidate the client area to trigger redrawing
        //     InvalidateRect(hwnd, NULL, TRUE);
        // }

        return 0;
    }
    case WM_GETICON: { // WM_GETICON == 127
        WPARAM icon_type = wparam;
        const char *type_str = NULL;
        switch (icon_type) {
        case ICON_SMALL: type_str = "SMALL"; break;
        case ICON_BIG: type_str = "BIG"; break;
        case ICON_SMALL2: type_str = "SMALL2"; break;
        }
        LOG("WM_GETICON: %s(%lld)", type_str ? type_str : "?", icon_type);
        if (!type_str) UNREACHABLE();
        // verify that DefWindowProc just returns NULL
        LRESULT result = DefWindowProc(hwnd, msg, wparam, lparam);
        ENFORCE_EQ("", "%p", NULL, (HANDLE)result);
        return 0;
    }
    case WM_NCCREATE: { // WM_NCCREATE == 129
        ENFORCE_EQ("", "%u", 2, global_msg_count);
        CREATESTRUCT* create = (CREATESTRUCT*)lparam;
        ENFORCE_EQ("", "%p", CREATE_PARAMS_MAGIC, create->lpCreateParams);
        ENFORCE_EQ("", "%p", GetModuleHandleW(NULL), create->hInstance);
        ENFORCE_EQ("", "%p", NULL, create->hMenu);
        ENFORCE_EQ("", "%p", NULL, create->hwndParent);
        LOG("WM_NCCREATE %d,%d %dx%d", create->x, create->y, create->cx, create->cy);
        {
            char buf[FORMAT_WND_STYLE_BUF_LEN];
            format_wnd_style(buf, create->style);
            LOG("  style=0x%x %s", create->style, buf);
        }
        ENFORCE_EQ("0x", "%x", WND_STYLE, create->style);
        ENFORCE(!wcscmp(WND_NAME, create->lpszName));
        ENFORCE(!wcscmp(WND_CLASS, create->lpszClass));
        {
            char buf[FORMAT_WND_EX_STYLE_BUF_LEN];
            format_wnd_ex_style(buf, create->dwExStyle);
            LOG("  exstyle=0x%x %s", create->dwExStyle, buf);
        }
        ENFORCE_EQ("0x", "%x", WND_EX_STYLE, create->dwExStyle);
        return TRUE; // continue creating the window
    }
    case WM_NCCALCSIZE: // WM_NCCALCSIZE == 131
        if (global_msg_count <= 4) {
            ENFORCE_EQ("", "%u", 3, global_msg_count);
        }

        // If wParam is TRUE, lparam points to NCCALCSIZE_PARAMS structure
        if (wparam) {
            NCCALCSIZE_PARAMS* params = (NCCALCSIZE_PARAMS*)lparam;
            LOG("WM_NCCALCSIZE(TRUE) (%d,%d)-(%d,%d) %dx%d",
                params->rgrc[0].left, params->rgrc[0].top,
                params->rgrc[0].right, params->rgrc[0].bottom,
                params->rgrc[0].right - params->rgrc[0].left,
                params->rgrc[0].bottom - params->rgrc[0].top);
            if (params->lppos) {
                LOG("Window position flags=0x%x", params->lppos->flags);
                LOG("Window position: (%d,%d) %dx%d",
                    params->lppos->x, params->lppos->y,
                    params->lppos->cx, params->lppos->cy);
            }
            // You can modify params->rgrc[0] here to change the client area
            // For example, to create a custom-drawn title bar:
            // params->rgrc[0].top += 30; // Add a 30-pixel custom title bar
            // By default, return 0 to let Windows handle non-client area calculations
            return DefWindowProc(hwnd, msg, wparam, lparam);
        }

        // If wParam is FALSE, lparam points to a RECT structure
        RECT* rect = (RECT*)lparam;
        LOG("WM_NCCALCSIZE(FALSE) (%d,%d)-(%d,%d) %dx%d",
            rect->left, rect->top, rect->right, rect->bottom,
            rect->right - rect->left, rect->bottom - rect->top);
        // You can modify the rectangle to change the client area
        // Return 0 to let Windows handle the default calculations
        return DefWindowProc(hwnd, msg, wparam, lparam);
    case WM_NCHITTEST: { // WM_NCHITTEST == 132
        POINT p = {(short)LOWORD(lparam), (short)HIWORD(lparam)};
        LRESULT result = DefWindowProc(hwnd, msg, wparam, lparam);
        LOG("WM_NCHITTEST: %d,%d => %lld", p.x, p.y, result);
        return result;
    }
    case WM_NCPAINT: { // WM_NCPAINT == 133
        HRGN update_region = (HRGN)wparam;
        // wparam is a region handle (HRGN) that contains the update region
        // If wparam is 1, the entire non-client area needs to be repainted
        // If wparam is a valid region handle, only that region needs to be repainted

        if (wparam == 1) {
            LOG("WM_NCPAINT: entire area");
        } else if (wparam != 0) {
            RECT region_rect;
            if (!GetRgnBox(update_region, &region_rect)) FATAL_WIN32("GetRgnBox", GetLastError());
            LOG(
                "WM_NCPAINT: region: (%d,%d)-(%d,%d) %dx%d",
                region_rect.left, region_rect.top,
                region_rect.right, region_rect.bottom,
                region_rect.right - region_rect.left,
                region_rect.bottom - region_rect.top
            );
        }

        // To perform custom drawing of the non-client area:
        // 1. Get the device context for the non-client area
        // HDC hdc = GetDCEx(hwnd, update_region, DCX_WINDOW | DCX_INTERSECTRGN);

        // 2. Perform your custom drawing
        // ...

        // 3. Release the device context
        // ReleaseDC(hwnd, hdc);

        // Let Windows handle the default non-client painting
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    case WM_NCACTIVATE: {// WM_NCACTIVATE = 134
        WPARAM active = wparam;
        ENFORCE((active == FALSE) || (active == TRUE));

        // The lparam is usually a handle to the window being deactivated when active is TRUE,
        // or NULL when active is FALSE. Can be -1 for special cases.
        HWND other_window = (lparam == -1) ? NULL : (HWND)lparam;

        LOG("WM_NCACTIVATE: active=%llu otherWindow=%p", active, other_window);

        // You can customize non-client area drawing here for active/inactive states
        // Returning TRUE tells Windows to use the default processing for this message,
        // which will update the window border and caption to show active/inactive state

        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    case WM_NCMOUSEMOVE: { // WM_NCMOUSEMOVE == 160
        POINT p = { (short)LOWORD(lparam), (short)HIWORD(lparam) };
        WPARAM hit_test_area = wparam;

        LOG("WM_NCMOUSEMOVE: point=%d,%d area=%s(%llu)",
            p.x, p.y, get_hit_str(hit_test_area), hit_test_area);

        // You can perform actions based on mouse movement in non-client areas.
        // For example, you might want to:
        // - Track mouse hovering over custom caption buttons
        // - Highlight parts of a custom title bar
        // - Implement custom tooltips for non-client elements

        // If you want to track when the mouse leaves the non-client area,
        // you can use TrackMouseEvent to receive WM_NCMOUSELEAVE messages:
        /*
          TRACKMOUSEEVENT tme;
          tme.cbSize = sizeof(TRACKMOUSEEVENT);
          tme.dwFlags = TME_LEAVE | TME_NONCLIENT;
          tme.hwndTrack = hwnd;
          tme.dwHoverTime = HOVER_DEFAULT;
          TrackMouseEvent(&tme);
        */

        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    case WM_NCLBUTTONDOWN: { // WM_NCLBUTTONDOWN == 161
        POINT p = { (short)LOWORD(lparam), (short)HIWORD(lparam) };
        WPARAM hit_test_area = wparam;
        LOG("WM_NCLBUTTONDOWN: %d,%d area=%s(%llu)",
            p.x, p.y, get_hit_str(hit_test_area), hit_test_area);
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    case WM_MOUSEMOVE: { // WM_MOUSEMOVE == 512
        POINT p = {(short)LOWORD(lparam), (short)HIWORD(lparam)};
        WPARAM key_flags = wparam;

        bool left_button = (key_flags & MK_LBUTTON) != 0;
        bool right_button = (key_flags & MK_RBUTTON) != 0;
        bool shift_key = (key_flags & MK_SHIFT) != 0;
        bool control_key = (key_flags & MK_CONTROL) != 0;
        bool middle_button = (key_flags & MK_MBUTTON) != 0;
        bool x_button1 = (key_flags & MK_XBUTTON1) != 0;
        bool x_button2 = (key_flags & MK_XBUTTON2) != 0;

        LOG("WM_MOUSEMOVE: %d,%d keys=0x%llx (L=%d,R=%d,M=%d,X1=%d,X2=%d,shift=%d,ctrl=%d)",
            p.x, p.y, (LONG_PTR)key_flags,
            left_button, right_button, middle_button,
            x_button1, x_button2, shift_key, control_key);

        // This is useful for UI elements that need to know when mouse leaves
        /*
          TRACKMOUSEEVENT tme;
          tme.cbSize = sizeof(TRACKMOUSEEVENT);
          tme.dwFlags = TME_LEAVE;
          tme.hwndTrack = hwnd;
          tme.dwHoverTime = HOVER_DEFAULT;
          TrackMouseEvent(&tme);
        */
        return 0;
    }
    case WM_IME_SETCONTEXT: { // WM_IME_SETCONTEXT == 641
        WPARAM is_active = wparam;
        LPARAM flags = lparam;

        LOG("WM_IME_SETCONTEXT: is_active=%llu flags=0x%llx", is_active, flags);

        // The flags parameter controls which parts of the IME window are drawn
        // You can modify the flags to customize IME window appearance
        // Common flags include:
        // ISC_SHOWUICOMPOSITIONWINDOW (0x80000000)
        // ISC_SHOWUICANDIDATEWINDOW (0x00000001)
        // ISC_SHOWUICANDIDATEWINDOW << 1 through ISC_SHOWUICANDIDATEWINDOW << 3

        // You can modify the flags to hide certain UI elements
        // For example, to hide the composition window:
        // flags &= ~ISC_SHOWUICOMPOSITIONWINDOW;

        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    case WM_IME_NOTIFY: { // WM_IME_NOTIFY == 0x0282 (642)
        WPARAM code = wparam;
        const char *code_str = ime_notify_code_str(code);
        LOG("WM_IME_NOTIFY: code=%s (0x%x) param=0x%llx", code_str, (unsigned)code, lparam);
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    case WM_IME_REQUEST: // WM_IME_REQUEST == 648
        return DefWindowProc(hwnd, msg, wparam, lparam);
    case WM_NCMOUSELEAVE: // WM_NCMOUSELEAVE == 674
        LOG("WM_NCMOUSELEAVE: mouse left non-client area");
        return 0;
    case WM_DWMNCRENDERINGCHANGED: // WM_DWMNCRENDERINGCHANGED == 799
        return DefWindowProc(hwnd, msg, wparam, lparam);
    default:
        if (msg < WM_USER) {
            LOG("TODO: implement window message %s (%u)", GetMsgName(msg), msg);
            /* return DefWindowProc(hwnd, msg, wparam, lparam); */
            abort();
        } else if (msg < WM_APP) {
            LOG("WM_USER+%u", msg - WM_USER);
            UNREACHABLE();
        } else if (msg < 0xc000) {
            LOG("App Window Message %u", msg);
            return DefWindowProc(hwnd, msg, wparam, lparam);
        } else if (msg < 0x10000) {
            LRESULT result = DefWindowProc(hwnd, msg, wparam, lparam);
            LOG("String Message %u (0x%x) => %lld (0x%llx)", msg, msg, result, (LONG_PTR)result);
            return result;
        } else {
            LOG("Reserved System Message %u?", msg);
            UNREACHABLE();
        }
    }
    UNREACHABLE();
}

int CALLBACK wWinMain(
    HINSTANCE hinstance,
    HINSTANCE hprev_instance,
    LPWSTR cmdline,
    int cmd_show
) {
    ENFORCE_EQ("", "%lu", 0xffff0000, WND_STYLE_ALL);
    {
        char buf[FORMAT_WND_STYLE_BUF_LEN + 100];
        ENFORCE_EQ("", "%zu", FORMAT_WND_STYLE_BUF_LEN, 1 + format_wnd_style(buf, 0xffffffff));
    }

    ENFORCE_EQ("", "%lu", 0xa7f77fd, WND_EX_STYLE_ALL);
    {
        char buf[FORMAT_WND_EX_STYLE_BUF_LEN + 100];
        ENFORCE_EQ("", "%zu", FORMAT_WND_EX_STYLE_BUF_LEN, 1 + format_wnd_ex_style(buf, 0xffffffff));
    }

    {
        char buf[FORMAT_SWP_FLAGS_BUF_LEN + 100];
        ENFORCE_EQ("", "%zu", FORMAT_SWP_FLAGS_BUF_LEN, 1 + format_swp_flags(buf, 0xffffffff));
    }


    ENFORCE_EQ("", "%p", hinstance, GetModuleHandleW(NULL));
    ENFORCE_EQ("", "%p", NULL, hprev_instance);

    {
        WNDCLASSEXW c;
        c.cbSize = sizeof(c);
        c.style = 0;
        c.lpfnWndProc = WndProc;
        c.cbClsExtra = 0;
        c.cbWndExtra = 0;
        c.hInstance = GetModuleHandleW(NULL);
        c.hIcon = NULL;
        c.hCursor = NULL;
        c.hbrBackground = NULL;
        c.lpszMenuName = NULL;
        c.lpszClassName = WND_CLASS;
        c.hIconSm = NULL;
        if (!RegisterClassExW(&c)) FATAL_WIN32("RegisterClass", GetLastError());
    }

    HWND hwnd = CreateWindowExW(
        WND_EX_STYLE,
        WND_CLASS,
        WND_NAME,
        WND_STYLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        GetModuleHandleW(NULL),
        CREATE_PARAMS_MAGIC
    );
    if (!hwnd) FATAL_WIN32("CreateWindow", GetLastError());
    ShowWindow(hwnd, SW_SHOWNORMAL);

    while (true) {
        MSG msg;
        BOOL result = GetMessage(&msg, NULL, 0, 0);
        if (result < 0) FATAL_WIN32("GetMessage", GetLastError());
        if (result == 0) {
            LOG("WM_QUIT %llu", msg.wParam);
            return msg.wParam;
        }
        DispatchMessage(&msg);
    }
}
