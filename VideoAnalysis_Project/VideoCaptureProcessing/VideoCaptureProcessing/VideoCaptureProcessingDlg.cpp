
// VideoCaptureProcessingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VideoCaptureProcessing.h"
#include "VideoCaptureProcessingDlg.h"
#include "afxdialogex.h"
#include "SimpleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CVideoCaptureProcessingDlg dialog



CVideoCaptureProcessingDlg::CVideoCaptureProcessingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVideoCaptureProcessingDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVideoCaptureProcessingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CVideoCaptureProcessingDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START_CAM, &CVideoCaptureProcessingDlg::OnBnClickedStartCam)
	ON_BN_CLICKED(IDC_VIDEO_TO_IMAGE, &CVideoCaptureProcessingDlg::OnBnClickedVideoToImage)
	ON_BN_CLICKED(IDC_IMAGE_TO_VIDEO, &CVideoCaptureProcessingDlg::OnBnClickedImageToVideo)
	ON_BN_CLICKED(IDC_MOTION_ANALYSIS, &CVideoCaptureProcessingDlg::OnBnClickedMotionAnalysis)
END_MESSAGE_MAP()


// CVideoCaptureProcessingDlg message handlers

BOOL CVideoCaptureProcessingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVideoCaptureProcessingDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVideoCaptureProcessingDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVideoCaptureProcessingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


int  m_Run_Process(LPCTSTR lpApplicationName, LPTSTR lpCommandLine)
{
	STARTUPINFO siStartupInfo;
	PROCESS_INFORMATION piProcessInfo;
	memset(&siStartupInfo, 0, sizeof(siStartupInfo));
	memset(&piProcessInfo, 0, sizeof(piProcessInfo));
	siStartupInfo.cb = sizeof(siStartupInfo);

	if (CreateProcess(lpApplicationName, lpCommandLine, NULL, NULL, FALSE, CREATE_NO_WINDOW | CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &siStartupInfo, &piProcessInfo) == FALSE)
		return -1;

	WaitForSingleObject(piProcessInfo.hProcess, INFINITE);

	::CloseHandle(piProcessInfo.hThread);
	::CloseHandle(piProcessInfo.hProcess);
	return 0;
}

void CVideoCaptureProcessingDlg::OnBnClickedStartCam()
{
	if (m_Run_Process(_T("CaptureEngine.exe"), NULL) != 0)
		AfxMessageBox(_T("CaptureEngine processing failed!"));
}


void CVideoCaptureProcessingDlg::OnBnClickedVideoToImage()
{
	AfxMessageBox(_T("This function needs two files: the first one is a mp4/wmv video file (input), and the second one is a bmp image file (output)."));

	TCHAR BASED_CODE szFilter_1[] = _T("MPEG-4 Files (*.mp4)|*.mp4|")_T("WMV Files (*.wmv)|*.wmv|")_T("All Files (*.*)|*.*||");
	CFileDialog dialogOpenFile_1(TRUE, _T("mp4"), NULL, OFN_FILEMUSTEXIST, szFilter_1, this);
	(dialogOpenFile_1.m_ofn).lpstrTitle = _T("Open a Video File");
	if (dialogOpenFile_1.DoModal() == IDCANCEL)
		return;
	CString strFileName_1 = dialogOpenFile_1.GetPathName();

	TCHAR BASED_CODE szFilter_2[] = _T("Bitmap Files (*.bmp)|*.bmp|All Files (*.*)|*.*||");
	CFileDialog dialogOpenFile_2(FALSE, _T("bmp"), NULL, OFN_FILEMUSTEXIST, szFilter_2, this);
	(dialogOpenFile_2.m_ofn).lpstrTitle = _T("Save Bmp Files as");
	if (dialogOpenFile_2.DoModal() == IDCANCEL)
		return;
	CString strFileName_2 = dialogOpenFile_2.GetPathName();

	CString flag_gray = _T("0");
	OptionDlg dlg;
	dlg.m_Text_1 = _T("Save as color images");
	dlg.m_Text_2 = _T("Save as grayscale images");
	dlg.m_Option = 0;
	if (dlg.DoModal()==IDOK)
		flag_gray = dlg.m_Option == 0 ? _T("0") : _T("1");

	CString lpAllCommand = _T("Video2Image.exe");
	lpAllCommand = lpAllCommand + " " + strFileName_1;
	lpAllCommand = lpAllCommand + " " + strFileName_2;
	lpAllCommand = lpAllCommand + " " + flag_gray;

	//	There are two arguments, need to let lpApplicationName be NULL, and everything be in lpCommandLine
	if (m_Run_Process(NULL, CT2W(lpAllCommand)) != 0)
		AfxMessageBox(_T("Video2Image processing failed!"));

	return;
}


void CVideoCaptureProcessingDlg::OnBnClickedImageToVideo()
{
	AfxMessageBox(_T("This function needs three files: the first one is a bmp image file with a low index number (input), and the second one is a bmp image file with a high index number (output). The third one is a mp4/wmv video file (output)."));

	TCHAR BASED_CODE szFilter_1[] = _T("Bitmap Files (*.bmp)|*.bmp|All Files (*.*)|*.*||");
	CFileDialog dialogOpenFile_1(TRUE, _T("bmp"), NULL, OFN_FILEMUSTEXIST, szFilter_1, this);
	(dialogOpenFile_1.m_ofn).lpstrTitle = _T("Open the First Bmp File");
	if (dialogOpenFile_1.DoModal() == IDCANCEL)
		return;
	CString strFileName_1 = dialogOpenFile_1.GetPathName();

	TCHAR BASED_CODE szFilter_2[] = _T("Bitmap Files (*.bmp)|*.bmp|All Files (*.*)|*.*||");
	CFileDialog dialogOpenFile_2(TRUE, _T("bmp"), NULL, OFN_FILEMUSTEXIST, szFilter_2, this);
	(dialogOpenFile_2.m_ofn).lpstrTitle = _T("Open the Last Bmp File");
	if (dialogOpenFile_2.DoModal() == IDCANCEL)
		return;
	CString strFileName_2 = dialogOpenFile_2.GetPathName();

	TCHAR BASED_CODE szFilter_3[] = _T("MPEG-4 Files (*.mp4)|*.mp4|")_T("WMV Files (*.wmv)|*.wmv|")_T("All Files (*.*)|*.*||");
	CFileDialog dialogOpenFile_3(FALSE, _T("mp4"), NULL, OFN_FILEMUSTEXIST, szFilter_3, this);
	(dialogOpenFile_3.m_ofn).lpstrTitle = _T("Save Video File as");
	if (dialogOpenFile_3.DoModal() == IDCANCEL)
		return;
	CString strFileName_3 = dialogOpenFile_3.GetPathName();

	int FPS = 30;
	InputDlg dlg;
	dlg.m_Text_1 = _T("Video FPS:");
	dlg.m_Input = FPS;
	if (dlg.DoModal() == IDOK)
		FPS = dlg.m_Input;

	int Data_Rate = 800000;
	dlg.m_Text_1 = _T("Video Data Rate:");
	dlg.m_Input = Data_Rate;
	if (dlg.DoModal() == IDOK)
		Data_Rate = dlg.m_Input;

	CString m_str;
	m_str.Format(_T("%d %d"), FPS, Data_Rate);

	CString lpAllCommand = _T("Image2Video.exe");
	lpAllCommand = lpAllCommand + " " + strFileName_1;
	lpAllCommand = lpAllCommand + " " + strFileName_2;
	lpAllCommand = lpAllCommand + " " + strFileName_3;
	lpAllCommand = lpAllCommand + " " + m_str;

	//	There are two arguments, need to let lpApplicationName be NULL, and everything be in lpCommandLine
	if (m_Run_Process(NULL, CT2W(lpAllCommand)) != 0)
		AfxMessageBox(_T("Image2Video processing failed!"));

	return;
}


void CVideoCaptureProcessingDlg::OnBnClickedMotionAnalysis()
{
	AfxMessageBox(_T("This function is to be added soon."));
}
