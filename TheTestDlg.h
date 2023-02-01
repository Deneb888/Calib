#pragma once


// CTheTestDlg dialog

class CTheTestDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTheTestDlg)

public:
	CTheTestDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTheTestDlg();

// Dialog Data
	enum { IDD = IDD_MYTESTDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_ChipID;
	// current ambient temperature
	float m_curTemp;
};
