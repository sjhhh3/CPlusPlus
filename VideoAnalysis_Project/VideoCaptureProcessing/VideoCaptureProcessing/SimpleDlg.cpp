// SimpleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VideoCaptureProcessing.h"
#include "SimpleDlg.h"
#include "afxdialogex.h"


// OptionDlg dialog

IMPLEMENT_DYNAMIC(OptionDlg, CDialogEx)

OptionDlg::OptionDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(OptionDlg::IDD, pParent)
	, m_Option(0)
{
	m_Title = _T("Option");
	m_Text_1 = _T("Option 1");
	m_Text_2 = _T("Option 2");
}

OptionDlg::~OptionDlg()
{
}

void OptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, m_Option);
}


BEGIN_MESSAGE_MAP(OptionDlg, CDialogEx)
END_MESSAGE_MAP()


// OptionDlg message handlers

BOOL OptionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(m_Title);

	((CEdit*)GetDlgItem(IDC_RADIO1))->SetWindowText(m_Text_1);
	((CEdit*)GetDlgItem(IDC_RADIO2))->SetWindowText(m_Text_2);

	return TRUE;
}


// InputDlg dialog

IMPLEMENT_DYNAMIC(InputDlg, CDialogEx)

InputDlg::InputDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(InputDlg::IDD, pParent)
	, m_Input(0)
{
	m_Title = _T("Input");
	m_Text_1 = _T("Input Value");
}

InputDlg::~InputDlg()
{
}

void InputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_Input);
}


BEGIN_MESSAGE_MAP(InputDlg, CDialogEx)
END_MESSAGE_MAP()


// InputDlg message handlers

BOOL InputDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(m_Title);

	((CEdit*)GetDlgItem(IDC_STATIC_TEXT))->SetWindowText(m_Text_1);

	return TRUE;
}
