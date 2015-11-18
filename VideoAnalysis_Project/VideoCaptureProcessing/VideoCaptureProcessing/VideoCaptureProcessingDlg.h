
// VideoCaptureProcessingDlg.h : header file
//

#pragma once


// CVideoCaptureProcessingDlg dialog
class CVideoCaptureProcessingDlg : public CDialogEx
{
// Construction
public:
	CVideoCaptureProcessingDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_VIDEOCAPTUREPROCESSING_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedStartCam();
	afx_msg void OnBnClickedVideoToImage();
	afx_msg void OnBnClickedImageToVideo();
	afx_msg void OnBnClickedMotionAnalysis();
};
