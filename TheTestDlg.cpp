// TheTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "TheTestDlg.h"
#include "afxdialogex.h"


// CTheTestDlg dialog

IMPLEMENT_DYNAMIC(CTheTestDlg, CDialogEx)

CTheTestDlg::CTheTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTheTestDlg::IDD, pParent)
	, m_ChipID(_T(""))
	, m_curTemp(0)
{

}

CTheTestDlg::~CTheTestDlg()
{
}

void CTheTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_ChipID);
	DDX_Text(pDX, IDC_EDIT2, m_curTemp);
	DDV_MinMaxFloat(pDX, m_curTemp, -20, 100);
}


BEGIN_MESSAGE_MAP(CTheTestDlg, CDialogEx)
END_MESSAGE_MAP()


// CTheTestDlg message handlers
