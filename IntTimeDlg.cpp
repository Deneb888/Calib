// IntTimeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "IntTimeDlg.h"
#include "afxdialogex.h"


// IntTimeDlg dialog

IMPLEMENT_DYNAMIC(IntTimeDlg, CDialogEx)

IntTimeDlg::IntTimeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IntTimeDlg::IDD, pParent)
{

	m_IntTime = 10;
}

IntTimeDlg::~IntTimeDlg()
{
}

void IntTimeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_IntTime);
	DDV_MinMaxInt(pDX, (int)m_IntTime, 1, 64000);
}


BEGIN_MESSAGE_MAP(IntTimeDlg, CDialogEx)
END_MESSAGE_MAP()


// IntTimeDlg message handlers
