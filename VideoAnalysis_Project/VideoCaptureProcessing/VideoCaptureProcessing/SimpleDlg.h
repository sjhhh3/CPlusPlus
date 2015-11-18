#pragma once


// OptionDlg dialog

class OptionDlg : public CDialogEx
{
	DECLARE_DYNAMIC(OptionDlg)

public:
	OptionDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~OptionDlg();

// Dialog Data
	enum { IDD = IDD_OPTION_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_Option;
	CString m_Title, m_Text_1, m_Text_2;
	virtual BOOL OnInitDialog();
};
#pragma once


// InputDlg dialog

class InputDlg : public CDialogEx
{
	DECLARE_DYNAMIC(InputDlg)

public:
	InputDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~InputDlg();

// Dialog Data
	enum { IDD = IDD_INPUT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_Input;
	CString m_Title, m_Text_1;
	virtual BOOL OnInitDialog();
};
