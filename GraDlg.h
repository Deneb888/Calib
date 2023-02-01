#pragma once

//***************************************************************
//Constant definition
//***************************************************************
#define UM_GRAPROCESS WM_USER+104	//Graphic dialog自定义消息序号

#define sendgramsg		1		// ReadRow button
#define sendpagemsg		2		// 画页命令
#define sendvideomsg	3		// video命令
#define gradatanum 200		// graphic dialog transmitted data number


// CGraDlg dialog

class CGraDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CGraDlg)

public:
	CGraDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraDlg();

// Dialog Data
	enum { IDD = IDD_GRAPHIC_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_EditReadRow;
	afx_msg void OnClickedBtnReadrow();
	void GraCalMainMsg();
	afx_msg LRESULT OnGraProcess(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedBtnDprow24();
	afx_msg void OnBnClickedBtnDppage12();
	afx_msg void OnBnClickedBtnDppage24();
	afx_msg void OnBnClickedBtnDpvedio();
	afx_msg void OnBnClickedBtnStopvideo();
	CButton m_ShowAllData;
	int m_ShowAllDataInt;
	CButton m_ReadHex;
	int m_ReadHexInt;
	CButton m_ReadDec;
	CButton m_ADCData;
	CButton m_ShowValidData;
	CString m_PixelData;
	afx_msg void OnBnClickedBtnClear();
	afx_msg void OnBnClickedBtnAdcconvert();
	CString m_ADCRecdata;
	afx_msg void OnBnClickedBtnClearadc();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedBtnDpvideo24();
	afx_msg void OnBnClickedRadioAdcdata();
	void SetGainMode(int gain);
	void DisplayHDR(void);
//	afx_msg void OnThumbposchangingSlider1(NMHDR *pNMHDR, LRESULT *pResult);
//	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
//	afx_msg void OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	void DisplayHDR24(void);
	int m_GainMode;
	afx_msg void OnClickedRadiolowgain();
//	afx_msg void OnUpdateRadiohdr(CCmdUI *pCmdUI);
//	afx_msg void OnUpdateRadiohighgain(CCmdUI *pCmdUI);
	afx_msg void OnRadiohighgain();
	afx_msg void OnRadiohdr();
	int m_FrameSize;
	afx_msg void OnClickedRadio12();
	afx_msg void OnRadio24();
	afx_msg void OnBnClickedButtonCapture();
	void CaptureFrame12(void);
	void CaptureFrame24(void);
	BOOL m_PixOut;
	afx_msg void OnClickedCheckPixout();
	BOOL m_OrigOut;
	afx_msg void OnClickedCheckOrigout();
	afx_msg void OnBnClickedBtnReadspi();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonCpycb();

	void CalcErrs();

	int Step1();
	int Step2();
	int Step3();
	int Step4();

	int Step5();
	int Step6();

	int MinHighByte(int i);
	int MaxHighByte(int i);

	void DisplayPattern(void);

	afx_msg void OnBnClickedButtonStep1();
	afx_msg void OnBnClickedButtonStep2();
	afx_msg void OnBnClickedButtonStep3();
	afx_msg void OnBnClickedButtonStep4();

	void TimerISR();
	void DebugLog(CString str);

	CString m_dLog;

	void SetMaxRepeatCount(int);

	void SaveRecData();

	void CalcFPNL();
	void CalcFPNH();
	afx_msg void OnBnClickedButtonStep5();
	BOOL m_subFPN;
	afx_msg void OnBnClickedCheckSubfpn();
	afx_msg void OnBnClickedButtonStep6();

	int Step0();
	afx_msg void OnBnClickedButtonStep0();
};
