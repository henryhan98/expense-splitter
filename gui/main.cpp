#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>
#include "expense.h"
#include "storage.h"
#include "operations.h"
#include "report.h"

namespace {
enum { People = 101, AddPerson, Search, Table, Day, Description, Payer, Amount,
       NewExpense, Apply, Remove, Save, Load, Status, Sharing, Daily, Balances, Settlement, CategoryChoice, CategoryTotals, CategoryAnalytics, CategoryRanking };
const char* categoryNames[] = {"Food", "Transport", "Accommodation", "Entertainment", "Other"};
Group group;
HWND window;
HFONT font;
bool dirty = false;
int editing = -1;

HWND control(int id) { return GetDlgItem(window, id); }
std::string text(int id) {
    HWND item = control(id);
    std::vector<char> buffer(GetWindowTextLengthA(item) + 1);
    GetWindowTextA(item, buffer.data(), static_cast<int>(buffer.size()));
    return buffer.data();
}
void set(int id, const std::string& value) { SetWindowTextA(control(id), value.c_str()); }
void message(const std::string& value) {
    MessageBoxA(window, value.c_str(), "Expense Splitter", MB_OK | MB_ICONINFORMATION);
}
HWND make(const char* type, const char* label, DWORD style, int id,
          int x, int y, int width, int height) {
    HWND item = CreateWindowExA(0, type, label, WS_CHILD | WS_VISIBLE | style,
        x, y, width, height, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
        GetModuleHandle(nullptr), nullptr);
    SendMessage(item, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    return item;
}
void label(const char* value, int x, int y, int width) {
    make("STATIC", value, 0, 0, x, y, width, 22);
}
void resetForm() {
    editing = -1;
    set(Day, "1");
    set(Description, "");
    set(Amount, "");
    SendMessage(control(CategoryChoice), CB_SETCURSEL, OTHER, 0);
    EnableWindow(control(CategoryChoice), TRUE);
    SendMessage(control(Payer), CB_SETCURSEL, group.people.empty() ? -1 : 0, 0);
    EnableWindow(control(Day), TRUE);
    set(Apply, "Add expense");
    EnableWindow(control(Sharing), TRUE);
    SendMessage(control(Sharing), LB_SETSEL, TRUE, -1);
    ListView_SetItemState(control(Table), -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
}
void refreshPeople() {
    SendMessage(control(Payer), CB_RESETCONTENT, 0, 0);
    SendMessage(control(Sharing), LB_RESETCONTENT, 0, 0);
    for (const auto& person : group.people) {
        SendMessageA(control(Payer), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(person.c_str()));
        SendMessageA(control(Sharing), LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(person.c_str()));
    }
}
void refresh() {
    // Store the original expense index on each row so filtering cannot edit the wrong bill.
    ListView_DeleteAllItems(control(Table));
    std::string query = text(Search);
    long long total = 0;
    int visible = 0;
    for (int i = 0; i < static_cast<int>(group.expenses.size()); ++i) {
        const Expense& e = group.expenses[i];
        total += e.amount;
        if (!matchesExpense(group, e, ExpenseFilter::Any, query)) continue;
        std::string values[] = {std::to_string(i + 1), std::to_string(e.day),
                               e.description, group.people[e.payer], money(e.amount), categoryNames[e.category]};
        LVITEMA row = {};
        row.mask = LVIF_TEXT | LVIF_PARAM;
        row.iItem = visible++;
        row.pszText = &values[0][0];
        row.lParam = i;
        int index = ListView_InsertItem(control(Table), &row);
        for (int column = 1; column < 6; ++column)
            ListView_SetItemText(control(Table), index, column, &values[column][0]);
    }
    std::ostringstream summary;
    summary << group.people.size() << " people | " << visible << " of "
            << group.expenses.size() << " expenses | Total: " << total / 100 << '.';
    if (total % 100 < 10) summary << '0';
    summary << total % 100 << (dirty ? " | Unsaved changes" : "");
    set(Status, summary.str());
}
void selectExpense() {
    int rowIndex = ListView_GetNextItem(control(Table), -1, LVNI_SELECTED);
    if (rowIndex < 0) return;
    LVITEMA row = {};
    row.mask = LVIF_PARAM;
    row.iItem = rowIndex;
    ListView_GetItem(control(Table), &row);
    editing = static_cast<int>(row.lParam);
    const Expense& e = group.expenses[editing];
    set(Day, std::to_string(e.day));
    set(Description, e.description);
    set(Amount, money(e.amount));
    SendMessage(control(CategoryChoice), CB_SETCURSEL, e.category, 0);
    EnableWindow(control(CategoryChoice), FALSE);
    SendMessage(control(Payer), CB_SETCURSEL, e.payer, 0);
    EnableWindow(control(Day), FALSE);
    set(Apply, "Confirm edit...");
    SendMessage(control(Sharing), LB_SETSEL, FALSE, -1);
    for (int person : e.sharedBy) SendMessage(control(Sharing), LB_SETSEL, TRUE, person);
    EnableWindow(control(Sharing), FALSE);
}
void applyExpense() {
    if (group.people.empty()) { message("Add at least one person first."); return; }
    Expense candidate = {};
    if (editing >= 0) candidate = group.expenses[editing];
    std::istringstream dayInput(text(Day)), amountInput(text(Amount));
    std::string extra;
    double amount;
    if (!(dayInput >> candidate.day) || (dayInput >> extra) || candidate.day < 1 || candidate.day > 365) {
        message("Enter a day from 1 to 365."); return;
    }
    if (!(amountInput >> amount) || (amountInput >> extra) || !std::isfinite(amount)
        || amount <= 0 || amount >= 10000000 || std::round(amount * 100) < 1) {
        message("Enter a positive amount of at least 0.01 and less than 10000000."); return;
    }
    candidate.amount = static_cast<int>(std::round(amount * 100));
    candidate.description = text(Description);
    if (candidate.description.empty()) candidate.description = "(no description)";
    candidate.payer = static_cast<int>(SendMessage(control(Payer), CB_GETCURSEL, 0, 0));
    candidate.category = static_cast<Category>(SendMessage(control(CategoryChoice), CB_GETCURSEL, 0, 0));
    if (candidate.payer < 0) { message("Select a payer."); return; }
    if (editing >= 0) {
        const Expense& old = group.expenses[editing];
        std::string preview = "Save changes to expense #" + std::to_string(editing + 1) + "?\n\n"
            + "Description: " + old.description + " -> " + candidate.description + "\n"
            + "Amount: " + money(old.amount) + " -> " + money(candidate.amount) + "\n"
            + "Payer: " + group.people[old.payer] + " -> " + group.people[candidate.payer];
        if (MessageBoxA(window, preview.c_str(), "Confirm edit", MB_YESNO | MB_ICONQUESTION) != IDYES) return;
        if (!replaceExpense(group, editing, candidate)) { message("Invalid expense."); return; }
    } else {
        for (int i = 0; i < static_cast<int>(group.people.size()); ++i)
            if (SendMessage(control(Sharing), LB_GETSEL, i, 0) > 0) candidate.sharedBy.push_back(i);
        if (!appendExpense(group, candidate)) { message("Select at least one person to share this bill."); return; }
    }
    dirty = true;
    resetForm();
    refresh();
}
bool chooseFile(bool save) {
    char path[MAX_PATH] = "";
    OPENFILENAMEA dialog = {};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = window;
    dialog.lpstrFilter = "Expense files (*.txt)\0*.txt\0All files\0*.*\0";
    dialog.lpstrFile = path;
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrDefExt = "txt";
    dialog.Flags = OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);
    if (!(save ? GetSaveFileNameA(&dialog) : GetOpenFileNameA(&dialog))) return false;
    if (!(save ? saveToFile(group, path) : loadFromFile(group, path))) {
        message(save ? "Could not save the file." : "Could not load this expense file.");
        return false;
    }
    dirty = false;
    if (!save) { refreshPeople(); resetForm(); }
    refresh();
    return true;
}
bool canDiscard() {
    if (!dirty) return true;
    int answer = MessageBoxA(window, "Save your changes first?", "Unsaved changes", MB_YESNOCANCEL | MB_ICONQUESTION);
    if (answer == IDCANCEL) return false;
    return answer == IDNO || chooseFile(true);
}

LRESULT CALLBACK reportHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_SIZE) {
        MoveWindow(GetDlgItem(hwnd, 1), 0, 0, LOWORD(lp), HIWORD(lp), TRUE);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}
void showReport(const char* title, const std::string& report) {
    WNDCLASSA type = {};
    type.lpfnWndProc = reportHandler;
    type.hInstance = GetModuleHandle(nullptr);
    type.lpszClassName = "ExpenseReportWindow";
    type.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassA(&type);
    HWND reportWindow = CreateWindowA(type.lpszClassName, title, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 680, 480, window, nullptr, type.hInstance, nullptr);
    std::string lines;
    for (char c : report) { if (c == '\n') lines += '\r'; lines += c; }
    RECT bounds;
    GetClientRect(reportWindow, &bounds);
    HWND body = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", lines.c_str(),
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
        0, 0, bounds.right, bounds.bottom, reportWindow, reinterpret_cast<HMENU>(1), type.hInstance, nullptr);
    SendMessage(body, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(ANSI_FIXED_FONT)), TRUE);
    ShowWindow(reportWindow, SW_SHOW);
}

LRESULT CALLBACK handle(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE: {
            window = hwnd;
            font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
            label("EXPENSE SPLITTER", 20, 16, 300);
            make("BUTTON", "Category totals", WS_TABSTOP, CategoryTotals, 350, 12, 135, 28);
            make("BUTTON", "Analytics", WS_TABSTOP, CategoryAnalytics, 495, 12, 135, 28);
            make("BUTTON", "Ranking", WS_TABSTOP, CategoryRanking, 640, 12, 140, 28);
            label("Add people, record bills, then select a row to edit.", 20, 42, 650);
            label("Person name", 20, 78, 100);
            make("EDIT", "", WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL, People, 125, 74, 220, 26);
            make("BUTTON", "Add person", WS_TABSTOP, AddPerson, 355, 73, 110, 28);
            make("BUTTON", "Save...", WS_TABSTOP, Save, 590, 73, 90, 28);
            make("BUTTON", "Load...", WS_TABSTOP, Load, 690, 73, 90, 28);
            label("Search", 20, 119, 100);
            make("EDIT", "", WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL, Search, 125, 115, 655, 26);
            label("Search description, payer, or day. Clear the box to show all expenses.", 125, 145, 655);
            HWND table = make(WC_LISTVIEWA, "", WS_BORDER | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                              Table, 20, 174, 760, 195);
            ListView_SetExtendedListViewStyle(table, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
            const char* columns[] = {"#", "Day", "Description", "Payer", "Amount", "Category"};
            int widths[] = {40, 45, 240, 130, 95, 180};
            for (int i = 0; i < 6; ++i) {
                LVCOLUMNA column = {};
                column.mask = LVCF_TEXT | LVCF_WIDTH;
                column.pszText = const_cast<char*>(columns[i]);
                column.cx = widths[i];
                ListView_InsertColumn(table, i, &column);
            }
            label("Day", 20, 386, 60);
            label("Description", 100, 386, 300);
            label("Payer", 425, 386, 190);
            label("Amount", 635, 386, 145);
            make("EDIT", "1", WS_BORDER | WS_TABSTOP | ES_NUMBER, Day, 20, 412, 60, 26);
            make("EDIT", "", WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL, Description, 100, 412, 305, 26);
            make("COMBOBOX", "", WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL, Payer, 425, 412, 190, 200);
            make("EDIT", "", WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL, Amount, 635, 412, 145, 26);
            label("Shared by (click to select). Fixed when editing.", 20, 450, 760);
            make("LISTBOX", "", WS_BORDER | WS_TABSTOP | WS_VSCROLL | LBS_MULTIPLESEL | LBS_NOINTEGRALHEIGHT, Sharing, 20, 475, 305, 80);
            make("BUTTON", "New / cancel edit", WS_TABSTOP, NewExpense, 350, 480, 135, 32);
            make("BUTTON", "Add expense", WS_TABSTOP, Apply, 495, 480, 135, 32);
            make("BUTTON", "Remove selected", WS_TABSTOP, Remove, 640, 480, 140, 32);
            make("BUTTON", "By day", WS_TABSTOP, Daily, 350, 525, 135, 32);
            make("BUTTON", "Balances", WS_TABSTOP, Balances, 495, 525, 135, 32);
            make("BUTTON", "Settle up", WS_TABSTOP, Settlement, 640, 525, 140, 32);
            label("Category", 350, 574, 85);
            make("COMBOBOX", "", WS_TABSTOP | CBS_DROPDOWNLIST, CategoryChoice, 440, 570, 190, 200);
            for (const char* name : categoryNames) SendMessageA(control(CategoryChoice), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(name));
            SendMessage(control(CategoryChoice), CB_SETCURSEL, OTHER, 0);
            make("STATIC", "", 0, Status, 20, 611, 760, 24);
            refresh();
            return 0;
        }
        case WM_NOTIFY:
            if (reinterpret_cast<NMHDR*>(lp)->idFrom == Table && reinterpret_cast<NMHDR*>(lp)->code == LVN_ITEMCHANGED)
                selectExpense();
            return 0;
        case WM_COMMAND:
            if (LOWORD(wp) == Search && HIWORD(wp) == EN_CHANGE) { resetForm(); refresh(); return 0; }
            if (HIWORD(wp) != BN_CLICKED) break;
            switch (LOWORD(wp)) {
                case AddPerson: {
                    std::string name = text(People);
                    if (name.find_first_not_of(" \t") == std::string::npos) { message("Enter a person's name."); break; }
                    if (group.people.size() >= 50) { message("A group can have up to 50 people."); break; }
                    group.people.push_back(name);
                    dirty = true;
                    set(People, "");
                    refreshPeople(); resetForm(); refresh();
                    break;
                }
                case CategoryTotals: showReport("Category totals", categoryTotalsReport(group)); break;
                case CategoryAnalytics: showReport("Category analytics", categoryAnalyticsReport(group)); break;
                case CategoryRanking: showReport("Category ranking", categoryRankingReport(group)); break;
                case Daily: showReport("Summary by day", dailyReport(group)); break;
                case Balances: showReport("Balances", balanceReport(group)); break;
                case Settlement: showReport("Settle up", settlementReport(group)); break;
                case NewExpense: resetForm(); break;
                case Apply: applyExpense(); break;
                case Remove:
                    if (editing < 0) { message("Select an expense first."); break; }
                    if (MessageBoxA(window, "Remove the selected expense?", "Confirm removal", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                        eraseExpense(group, editing);
                        dirty = true;
                        resetForm(); refresh();
                    }
                    break;
                case Save:
                    if (group.people.empty()) message("Add at least one person before saving.");
                    else chooseFile(true);
                    break;
                case Load: if (canDiscard()) chooseFile(false); break;
            }
            return 0;
        case WM_CLOSE: if (canDiscard()) DestroyWindow(hwnd); return 0;
        case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}
} // namespace

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int show) {
    INITCOMMONCONTROLSEX common = {sizeof(common), ICC_LISTVIEW_CLASSES};
    InitCommonControlsEx(&common);
    WNDCLASSA type = {};
    type.lpfnWndProc = handle;
    type.hInstance = instance;
    type.lpszClassName = "ExpenseSplitterWindow";
    type.hCursor = LoadCursor(nullptr, IDC_ARROW);
    type.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassA(&type)) return 1;
    RECT bounds = {0, 0, 800, 655};
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRect(&bounds, style, FALSE);
    HWND app = CreateWindowA(type.lpszClassName, "Expense Splitter", style,
        CW_USEDEFAULT, CW_USEDEFAULT, bounds.right - bounds.left, bounds.bottom - bounds.top,
        nullptr, nullptr, instance, nullptr);
    if (!app) return 1;
    ShowWindow(app, show);
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        if (!IsDialogMessage(app, &msg)) { TranslateMessage(&msg); DispatchMessage(&msg); }
    }
    return static_cast<int>(msg.wParam);
}
