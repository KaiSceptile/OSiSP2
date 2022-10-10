#include <Windows.h>
#include <fstream>
#include <vector>
#include <regex>


using namespace std;

LPCWSTR font = L"Arial";
int fontsize = 30;
int fontweight = 300;
struct RowsTable
{
	int* rowsincell;
	int totalrows;
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT ReadFile(string filePath);

void DrawTextTable(HWND hWnd, HDC& hdc);
void DrawYBorder(HDC hdc, int cWidth, int cHeight, int wndHeight, int colNumb);
void DrawXBorder(HDC hdc, int cWidth, int cHeight, int wndHeight, RECT win);
RowsTable NumOfRowsWithWordBreak(int width, int cellWidth);
HPEN table_pen = CreatePen(PS_SOLID, 4, RGB(153, 224, 215));
static vector<vector<string>> textTable{};


INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	string filePath = "D:\\ОСиСП\\OSiSP2\\x64\\Debug\\table.txt";
	if (ReadFile(filePath)) {
		const wchar_t* const FILE_ERROR = L"Error open file";
		MessageBoxW(NULL, FILE_ERROR, NULL, MB_ICONERROR);
		return 1;
	}

	LPCWSTR const className = L"TextApp";
	LPCWSTR const windowName = L"Text";

	WNDCLASSEX wcex;
	HWND hWnd;
	MSG msg;

	// fill window class structure
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = className;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	// register window class
	if (!RegisterClassEx(&wcex)) {
		return EXIT_FAILURE;
	}
	//int totalheight= textTable.size()*fontsize*4;
	// create window
	hWnd = CreateWindowEx(
		0,
		className,
		windowName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		900, 600,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd) {
		const wchar_t* const WND_CREATE_ERROR = L"Failed to crate window!";
		MessageBoxW(NULL, WND_CREATE_ERROR, NULL, MB_OK);
		return 1;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	//  process message loop
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	HDC hdc = NULL;
	static HPEN hpen = NULL;
	
	switch (msg) {

	case WM_CREATE: {
		hpen = table_pen;
		break;
	}


	case WM_PAINT: {
		PAINTSTRUCT ps;
		HFONT oldFont, newFont;
		hdc = BeginPaint(hWnd, &ps);
		SelectObject(hdc, hpen);
		newFont = CreateFont(fontsize, fontsize/2, 0, 0, fontweight, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, (font));
		oldFont = (HFONT)SelectObject(hdc, newFont);
		DrawTextTable(hWnd, hdc);
		SelectObject(hdc, oldFont);
		DeleteObject(newFont);
		DeleteObject(oldFont);
		EndPaint(hWnd, &ps);
		break;
	}

	case WM_DESTROY: {
		DeleteObject(hpen);
		hpen = NULL;
		table_pen = NULL;
		
		PostQuitMessage(0);
		break;
	}

	default: {
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	}
	return 0;
}

INT ReadFile(string filePath) {

	ifstream file(filePath);

	if (!file) {
		return 1;
	}

	string currentLine;
	while (getline(file, currentLine)) {
		regex regex{ R"([,][\s]+)" };	// remove commons and whitespaces
		sregex_token_iterator iterator{ currentLine.begin(), currentLine.end(), regex, -1 };
		textTable.emplace_back(vector<string>{iterator, {}});
	}

	file.close();
	return 0;
}

void DrawTextTable(HWND hWnd, HDC& hdc) {

	int colNumb = textTable[0].size();
	int rowNumb = textTable.size();

	// get basic font info
	TEXTMETRIC textMetric;
	GetTextMetrics(hdc, &textMetric);

	// get program window
	RECT windowRect;
	GetClientRect(hWnd, &windowRect);

	// count table params
	int cellWidth = (windowRect.right - windowRect.left) / colNumb;
	RowsTable rows = NumOfRowsWithWordBreak(textMetric.tmAveCharWidth, cellWidth);
	int cellHeight = (windowRect.bottom - windowRect.top) / rows.totalrows;
	int totalHeight = cellHeight/8;
	int cellmove = 3 * cellHeight / 8;
	DrawXBorder(hdc, cellWidth, cellHeight, totalHeight+ cellmove, windowRect);
	// fill table 
	for (int i = 0; i < rowNumb; i++) {
		int maxRowHeight = cellHeight;

		// draw text
		for (int j = 0; j < colNumb; ++j) {
			RECT cell;
			cell.left = j * cellWidth;
			cell.right = cell.left + cellWidth;
			cell.top = totalHeight;
			cell.bottom = cell.top+ cellHeight*rows.rowsincell[i];
			wstring currentLine = wstring(textTable[i][j].begin(), textTable[i][j].end());
			int curCellHeight = DrawText(hdc, currentLine.c_str(), -1, &cell, DT_WORDBREAK | DT_CENTER | DT_END_ELLIPSIS);
			}
		totalHeight += cellHeight * rows.rowsincell[i];
		DrawXBorder(hdc, cellWidth, cellHeight * rows.rowsincell[i], totalHeight+ cellmove, windowRect);
		
	}

	// draw final table borders
	DrawXBorder(hdc, cellWidth, cellHeight * rows.rowsincell[rowNumb-1], totalHeight+ cellmove, windowRect);
	DrawYBorder(hdc, cellWidth, cellHeight * rows.rowsincell[rowNumb - 1], totalHeight+ cellmove, colNumb);
}

void DrawYBorder(HDC hdc, int cWidth, int cHeight, int wndHeight, int colNumb) {
	for (int i = 0; i <= colNumb; i++) {
		MoveToEx(hdc, cWidth * i, 0, nullptr);
		LineTo(hdc, cWidth * i, wndHeight - cHeight / 2);
	}
}

void DrawXBorder(HDC hdc, int cWidth, int cHeight, int wndHeight, RECT win) {
	MoveToEx(hdc, 0, wndHeight - cHeight/2 , nullptr);
	LineTo(hdc, win.right, wndHeight - cHeight/2);
}

RowsTable NumOfRowsWithWordBreak(int width, int cellWidth) {
	int colNumb = textTable[0].size();
	int rowNumb = textTable.size();
	int* Array=new int[rowNumb];
	int total = 0;
	for (int i = 0; i < rowNumb; i++) {
		int maxinrow = 1;
		for (int j = 0; j < colNumb; ++j) {
			wstring currentLine = wstring(textTable[i][j].begin(), textTable[i][j].end());
			int cellRows = currentLine.length() * width / cellWidth + ((currentLine.length() * width % cellWidth) ? 1 : 0);
			maxinrow = (maxinrow < cellRows) ? cellRows : maxinrow;
		}
		Array[i] = maxinrow;
		total += maxinrow;
	}
	RowsTable result = { Array , total };
	return result;
}
