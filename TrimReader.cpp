
#include "stdafx.h"
#include "TrimReader.h"

#define DARK_LEVEL 100
#define DARK_MANAGE

#define	CALIB_PROGRAM

#ifdef CALIB_PROGRAM
extern CString g_ChipID;				// Useful for writing Trim dat file
#else
CString g_ChipID;						// Just so I can compile
#endif


// Node

CTrimNode::CTrimNode()
{
	Initialize();
}

void 
CTrimNode::Initialize()
{
	int i;

	for(i=0; i<TRIM_IMAGER_SIZE; i++) {
		kb[i][0] = 1;		// k
		kb[i][1] = 0;		// b
		kb[i][2] = 0;		// k2
		kb[i][3] = 0;		// b2
		kb[i][4] = 0;		// c
		kb[i][5] = 0;		// h

		fpn[0][i] = 0;
		fpn[1][i] = 0;

		if(!i) tempcal[i] = 1;
		else tempcal[i] = 0;
	}

	rampgen = 0x88;
	range = 0xf;
	
	auto_v20[0] = 0x8;
	auto_v20[1] = 0xa;

	auto_v15 = 0x8;

#ifdef CALIB_PROGRAM
	version = 2;
#else
	version = 0x0;
#endif

	tbuff_size = 0;
	tbuff_rptr = 0;
}


// Reader

CTrimReader::CTrimReader() 
{ 
//	InFile = 0; 
	curNode = NULL;
	NumNode = 0;

	WordIndex = 0;

	calib2 = 0;
//	version = 2;			// new auto cal method with floating point kb and fpn values
}

CTrimReader::~CTrimReader() 
{
	CloseFile();
}

void CTrimReader::CloseFile() 
{
	if(fileLoaded) {
		InFile.Close();
		fileLoaded = false;
	}
}

int CTrimReader::Load(TCHAR* fn) 
{
	int e;
	CString FileBuf;

	e = InFile.Open(fn, CFile::modeRead);

	fileLoaded = e;

	if(!e) return e;

	DWORD fl = (DWORD)InFile.GetLength();

	char *buf = new char[fl];

	InFile.Read(buf, fl);

	FileBuf = buf;

	delete buf;

	int ep;

	CString delimit = CString(", \t\r\n");

	int i = 0;

	FileBuf.TrimLeft(delimit);

	while (((ep = FileBuf.FindOneOf(delimit)) != -1) && i < TRIM_MAX_WORD) 
	{
		WordBuf[i] = FileBuf.Mid(0, ep);
		int l = FileBuf.GetLength();
		FileBuf = FileBuf.Mid(ep, (l - ep));
		FileBuf.TrimLeft(delimit);
		i++;
	}
	
	MaxWord = i;
	FileBuf.Empty();

	return e;
}

int CTrimReader::GetWord()
{
	CurWord = WordBuf[WordIndex];
	WordIndex++;

	return WordIndex;
}

int CTrimReader::Match(CString s)
{
	return (int)(CurWord.Compare(s) == 0);
}

void CTrimReader::Parse()
{
	CString Name;
	int i = 0;

	if(!InFile) 
		return;

	for(;;) {		
		if(GetWord() > MaxWord) 
			break;		
		
		if(Match(CString("DEF"))) {
			GetWord();
			Name = CurWord;
			
			GetWord();
			if(Match(CString("{"))) {
				curNode = Node + i;
				i++;
				curNode->name = Name;
				ParseNode();
			}
			else break;
		}
		else break;
	}

	NumNode = i;
}


void CTrimReader::ParseNode()
{
	if(!InFile) 
		return;

	for(;;) {		
		if(GetWord() > MaxWord) 
			break;		
		
		if(Match(CString("Kb"))) {
			GetWord();
			if(Match(CString("{"))) {
				if (curNode->version == 2) 
					ParseMatrix();
				else 
					ParseMatrixOld();

				GetWord();
				if(!Match(CString("}"))) 
					return;
			}
			else return;
		}
		else if(Match(CString("Fpn_lg"))) {
			GetWord();
			if(Match(CString("{"))) {
				ParseArray(0);

				GetWord();
				if(!Match(CString("}"))) 
					return;
			}
			else return;
		}		
		else if(Match(CString("Fpn_hg"))) {
			GetWord();
			if(Match(CString("{"))) {
				ParseArray(1);

				GetWord();
				if(!Match(CString("}"))) 
					return;
			}
			else return;
		}
		else if(Match(CString("Temp_calib"))) {
			GetWord();
			if(Match(CString("{"))) {
				ParseArray(2);

				GetWord();
				if(!Match(CString("}"))) 
					return;
			}
			else return;
		}		
		else if(Match(CString("Rampgen"))) {
			GetWord();
			if(Match(CString("{"))) {
				ParseValue(2);

				GetWord();
				if(!Match(CString("}"))) 
					return;
			}
			else return;
		}
		else if(Match(CString("AutoV20_lg"))) {
			GetWord();
			if(Match(CString("{"))) {
				ParseValue(0);

				GetWord();
				if(!Match(CString("}"))) 
					return;
			}
			else return;
		}
		else if(Match(CString("AutoV20_hg"))) {
			GetWord();
			if(Match(CString("{"))) {
				ParseValue(1);

				GetWord();
				if(!Match(CString("}"))) 
					return;
			}
			else return;
		}
		else if(Match(CString("AutoV15"))) {
			GetWord();
			if(Match(CString("{"))) {
				ParseValue(3);

				GetWord();
				if(!Match(CString("}"))) 
					return;
			}
			else return;
		}
		else if (Match(CString("Range"))) {
			GetWord();
			if (Match(CString("{"))) {
				ParseValue(4);

				GetWord();
				if (!Match(CString("}")))
					return;
			}
			else return;
		}
		else if (Match(CString("Version"))) {
			GetWord();
			if (Match(CString("{"))) {
				ParseValue(5);

				GetWord();
				if (!Match(CString("}")))
					return;
			}
			else return;
		}
		else if(Match(CString("}"))) {
			return;
		}
		else 
			return;
	}
}

void CTrimReader::ParseMatrix()
{
	for(int i=0; i<TRIM_IMAGER_SIZE; i++) {
		for(int j=0; j<6; j++) {				// New calib method with hump factor
			if(GetWord() > MaxWord) 
				break;
			curNode->kb[i][j] = _tstof((LPCTSTR)CurWord); // atof(CurWord);
		}
	}
}

void CTrimReader::JustParseMatrix()
{
	curNode = Node;

	ParseMatrix();

/*	for (int i = 0; i<TRIM_IMAGER_SIZE; i++) {
		for (int j = 0; j<6; j++) {				// New calib method with hump factor
			if (GetWord() == MaxWord)
				break;
			curNode->kb[i][j] = _tstof((LPCTSTR)CurWord); // atof(CurWord);
		}
	}
*/
}


void CTrimReader::ParseArray(int gain) 
{
	for(int i=0; i<12; i++) {
		GetWord();	
		if(gain == 2) 
			curNode->tempcal[i] = _tstof((LPCTSTR)CurWord);
		else 
			curNode->fpn[gain][i] = _tstof((LPCTSTR)CurWord);
	}
}

// gain: 0, 1 - auto_v20[0, 1]; 2: rampgen; 3: auto_v15; 4: range; 5

 void CTrimReader::ParseValue(int gain) 
{
	GetWord();

	CString word = CurWord;
	word.MakeLower();
	int p = word.Find(CString("0x"));
	int l = word.GetLength();
	word = word.Mid(p+2, l-p-2);
	unsigned int val = _tstoi((LPCTSTR)word);

	val =  _tcstoul((LPCTSTR)word, 0, 16);

	if(gain == 2)
		curNode->rampgen = val;
	else if(gain == 3) 
		curNode->auto_v15 = val;
	else if (gain == 4)
		curNode->range = val;
	else if (gain == 5)
		curNode->version = val;
	else 
		curNode->auto_v20[gain] = val;
}
 
 // NumData =  "Column Number", this is 0 based. For 12 mode, it is 0 to 11.
 // pixelNum = "Frame Size", 12 or 24
 // PCRNum = channel ID, this is 1 based.
 // gain_mode = 1: low gain, 0: high gain.

 int CTrimReader::ADCCorrection(int NumData, BYTE HighByte, BYTE LowByte, int pixelNum, int PCRNum, int gain_mode, int* flag)
 {
	 unsigned int ver = Node[PCRNum - 1].version;

	 if (ver == 2) {
		 return ADCCorrectionf(NumData, HighByte, LowByte, pixelNum, PCRNum, gain_mode, flag);
	 }
	 else {
		 return ADCCorrectionOld(NumData, HighByte, LowByte, pixelNum, PCRNum, gain_mode, flag);
	 }
 }

int CTrimReader::ADCCorrectionf(int NumData, BYTE HighByte, BYTE LowByte, int pixelNum, int PCRNum, int gain_mode, int* flag)
{
	int hb,lb, lbc, hbi;
	int hbln,lbp,hbhn;
	bool oflow = false, uflow = false; //  qerr_big=false;

//	CString strbuf;
	double ioffset = 0; 
	int result;

	hb = (int)HighByte;
	hbln = hb % 16;		//
	hbhn = hb / 16;		//

	int nd = 0;
	if(pixelNum == 12) nd = NumData;
	else nd = NumData >> 1;

	double k, b, c, h;

	c = Node[PCRNum - 1].kb[nd][4];
	h = Node[PCRNum - 1].kb[nd][5];
	double shrink = c * 0.0033;

	if (hb < 16) {
		k = Node[PCRNum - 1].kb[nd][0];
		b = Node[PCRNum - 1].kb[nd][1] + h * 0.5;			// 15 is just an empirical value, the first bump is raised higher. To do: what about reverse bump
		c = c + 0.1 * h;
	}
	else if (hb < 128) {
		k = Node[PCRNum - 1].kb[nd][0];
		b = Node[PCRNum - 1].kb[nd][1];					// 
	}
	else {
		k = Node[PCRNum - 1].kb[nd][2];
		b = Node[PCRNum - 1].kb[nd][3];					// 
	}

	ioffset =k * (double)hb + b;

	lb = (int)LowByte;
	lbc = lb + (int)ioffset;

	if (hb > 128) {
		hbi = 128 + (hb - 128) / 2;
	}
	else {
		hbi = hb;
	}

	// Use lbc, not hbln to calculate sawtooth correction, as hbln tends to be a little jittery	
	ioffset += ((double)lbc - 128) * (c - shrink * (double)hbi) / 12;		// 12/19/2016 modification, shrinking sawtooth.
	lbc = lb + (int)ioffset;					// re-calc lbc, 2 pass algorithm
				
	if(lbc > 255) lbc = 255;
	else if(lbc < 0) lbc = 0;

	lbp = hbln * 16 + 7;
	
	int lbpc = lbp - (int)ioffset;				// lpb - ioffset: low byte predicted from the high byte low nibble BEFORE correction
	int qerr = lbp - lbc;					// if the lbc is correct, this would be the quantization error. If it is too large, maybe lb was the saturated "stuck" version
		
	if (lbpc > 255 + 20) {					// We allow some correction error, because hbln may have randomly flipped.
		oflow = true; *flag = 1;
	}
	else if (lbpc > 255 && qerr > 28) {		// Again we allow some tolerance because hbln may have drifted, leading to fake error
		oflow = true; *flag = 2;
	}
	else if(lbpc > 191 && qerr > 52) {
		oflow = true; *flag = 3;
	}
	else if(qerr > 96) {
		oflow = true; *flag = 4;
	}
	else if(lbpc < -20) {
		uflow = true; *flag = 5;
	}
	else if(lbpc < 0 && qerr < -28) {
		uflow = true; *flag = 6;
	}
	else if(lbpc < 64 && qerr < -52) {
		uflow = true; *flag = 7;
	}
	else if(qerr < -96) {
		uflow = true; *flag = 8;
	}
	else {
		*flag = 0;
	}
		
	if (oflow || uflow) {
		result = hb * 16 + 7;
	}
	else {
		result = hbhn * 256 + lbc;
	}		

	if(calib2) return result;
		
#ifdef DARK_MANAGE

	if(!gain_mode)
		result += -(int) (Node[PCRNum-1].fpn[1][nd]) + DARK_LEVEL;		// high gain
	else 
		result += -(int) (Node[PCRNum-1].fpn[0][nd]) + DARK_LEVEL;		// low gain

	if(result < 0) result = 0;

#endif

	return result;
}

#ifdef CALIB_PROGRAM

extern float g_curTemp;
extern float g_juncTemp;

#endif

void CTrimReader::WriteTrimData()
{
	CString str;
	int i;
	int new_version = 2;

	curNode = Node;

	TrimDotData.Empty();
	TrimDotData.Format("DEF %s {\r\n", g_ChipID);

	TrimDotData += "Version {\r\n";
	str.Format("0x%x\r\n", new_version);
	TrimDotData += str;
	TrimDotData += "}\r\n";

	TrimDotData += "Kb {\r\n";

	for (i = 0; i<12; i++) {
		str.Format("%0.5f, %0.5f, %0.5f, %0.5f, %0.5f, %0.5f", 
			curNode->kb[i][0], curNode->kb[i][1],
			curNode->kb[i][2], curNode->kb[i][3],
			curNode->kb[i][4], curNode->kb[i][5]);

		TrimDotData += str;
		if (i != 11)
			TrimDotData += ",\r\n";
		else
			TrimDotData += "\r\n";
	}

	TrimDotData += "}\r\n";
	TrimDotData += "Fpn_lg {\r\n";

	for (i = 0; i<12; i++) {
		str.Format("%0.5f", curNode->fpn[0][i]);
		TrimDotData += str;
		if (i != 11)
			TrimDotData += ", ";
		else
			TrimDotData += "\r\n";
	}

	TrimDotData += "}\r\n";
	TrimDotData += "Fpn_hg {\r\n";

	for (i = 0; i<12; i++) {
		str.Format("%0.5f", curNode->fpn[1][i]);
		TrimDotData += str;
		if (i != 11)
			TrimDotData += ", ";
		else
			TrimDotData += "\r\n";
	}

	TrimDotData += "}\r\n";

	TrimDotData += "AutoV20_lg {\r\n";
	str.Format("0x%x\r\n", (int)curNode->auto_v20[0]);
	TrimDotData += str;
	TrimDotData += "}\r\n";

	TrimDotData += "AutoV20_hg {\r\n";
	str.Format("0x%x\r\n", (int)curNode->auto_v20[1]);
	TrimDotData += str;
	TrimDotData += "}\r\n";

	TrimDotData += "Rampgen {\r\n";
	str.Format("0x%x\r\n", (int)curNode->rampgen);
	TrimDotData += str;
	TrimDotData += "}\r\n";

	TrimDotData += "Range {\r\n";
	str.Format("0x%x\r\n", (int)curNode->range);
	TrimDotData += str;
	TrimDotData += "}\r\n";

	TrimDotData += "AutoV15 {\r\n";
	str.Format("0x%x\r\n", (int)curNode->auto_v15);
	TrimDotData += str;
	TrimDotData += "}\r\n";

	curNode->tempcal[0] = 29.5;				// tempcal1

#ifdef CALIB_PROGRAM
	curNode->tempcal[1] = (double)(g_curTemp - 29.5 * g_juncTemp);
#endif

	TrimDotData += "Temp_calib {\r\n";

	for (i = 0; i<12; i++) {
		str.Format("%0.5f", curNode->tempcal[i]);
		TrimDotData += str;
		if (i != 11)
			TrimDotData += ", ";
		else
			TrimDotData += "\r\n";
	}

	TrimDotData += "}\r\n";
	TrimDotData += "}\r\n";
}

void CTrimReader::WriteTrimDatai()
{
	int intmax = 32767;
	int intmax256 = 128;
	CString str;
	int i;
	int new_version = 3;

	curNode = Node;

	TrimDotData.Empty();
	TrimDotData.Format("DEF %s {\r\n", g_ChipID);

	TrimDotData += "Version {\r\n";
	str.Format("0x%x\r\n", new_version);
	TrimDotData += str;
	TrimDotData += "}\r\n";

	TrimDotData += "Kb {\r\n";

	for(i=0; i<12; i++) {
		str.Format("%d, %d, %d, %d, %d, %d", (int)(curNode->kb[i][0] * (double)intmax), (int)(curNode->kb[i][1] * (double)intmax256), 
			(int)(curNode->kb[i][2] * (double)intmax), (int)(curNode->kb[i][3] * (double)intmax256), 
			(int)(curNode->kb[i][4] * (double)intmax256), (int)(curNode->kb[i][5] * (double)intmax256));

		TrimDotData += str;
		if(i != 11) 
			TrimDotData += ",\r\n";
		else 
			TrimDotData += "\r\n";
	}

	TrimDotData += "}\r\n";
	TrimDotData += "Fpn_lg {\r\n";

	for(i=0; i<12; i++) {
		str.Format("%d", (int)curNode->fpn[0][i]);
		TrimDotData += str;
		if(i != 11) 
			TrimDotData += ", ";
		else 
			TrimDotData += "\r\n";
	}

	TrimDotData += "}\r\n";
	TrimDotData += "Fpn_hg {\r\n";

	for(i=0; i<12; i++) {
		str.Format("%d", (int)curNode->fpn[1][i]);
		TrimDotData += str;
		if(i != 11) 
			TrimDotData += ", ";
		else 
			TrimDotData += "\r\n";
	}

	TrimDotData += "}\r\n";
	
	TrimDotData += "AutoV20_lg {\r\n";
	str.Format("0x%x\r\n", (int)curNode->auto_v20[0]);
	TrimDotData += str;
	TrimDotData += "}\r\n";

	TrimDotData += "AutoV20_hg {\r\n";
	str.Format("0x%x\r\n", (int)curNode->auto_v20[1]);
	TrimDotData += str;
	TrimDotData += "}\r\n";

	TrimDotData += "Rampgen {\r\n";
	str.Format("0x%x\r\n", (int)curNode->rampgen);
	TrimDotData += str;
	TrimDotData += "}\r\n";

	TrimDotData += "Range {\r\n";
	str.Format("0x%x\r\n", (int)curNode->range);
	TrimDotData += str;
	TrimDotData += "}\r\n";

	TrimDotData += "AutoV15 {\r\n";
	str.Format("0x%x\r\n", (int)curNode->auto_v15);
	TrimDotData += str;
	TrimDotData += "}\r\n";

	curNode->tempcal[0] = 29.5;				// tempcal1

#ifdef CALIB_PROGRAM
	curNode->tempcal[1] = (double)(g_curTemp - 29.5 * g_juncTemp);
#endif

	TrimDotData += "Temp_calib {\r\n";

	for (i = 0; i<12; i++) {
		str.Format("%d", (int)round(curNode->tempcal[i] * 128));
		TrimDotData += str;
		if (i != 11)
			TrimDotData += ", ";
		else
			TrimDotData += "\r\n";
	}

	TrimDotData += "}\r\n";
	TrimDotData += "}\r\n";
}

// NumData =  "Column Number", this is 0 based. For 12 mode, it is 0 to 11.
// pixelNum = "Frame Size", 12 or 24
// PCRNum = channel ID, this is 1 based.
// gain_mode = 1: low gain, 0: high gain.

// With the Sawtooth method, we need to gather denser data and perform a better data fitting.

// This is the integer version of the auto correct function

int CTrimReader::ADCCorrectioni(int NumData, BYTE HighByte, BYTE LowByte, int pixelNum, int PCRNum, int gain_mode, int* flag)
{
	int hb, lb, lbc, hbi;
	int hbln, lbp, hbhn;
	bool oflow = false, uflow = false; 
									  
	int ioffset = 0;
	int result;

	int intmax = 32767;
	int intmax256 = 128;

	hb = (int)HighByte;
	hbln = hb % 16;		//
	hbhn = hb / 16;		//

	int nd = 0;
	if (pixelNum == 12) nd = NumData;
	else nd = NumData >> 1;

	int k, b, c, h;

//	double shrink = 0.022;

	c = (int)(Node[PCRNum - 1].kbi[nd][4]);
	h = (int)(Node[PCRNum - 1].kbi[nd][5]);

	if (hb < 16) {
		k = (int)(Node[PCRNum - 1].kbi[nd][0]);
		b = (int)(Node[PCRNum - 1].kbi[nd][1]) + h / 2;			// 15 is just an empirical value, the first bump is raised higher. To do: what about reverse bump
		c = c + h / 10;
	}
	else if (hb < 128) {
		k = (int)(Node[PCRNum - 1].kbi[nd][0]);
		b = (int)(Node[PCRNum - 1].kbi[nd][1]);					// 
	}
	else {
		k = (int)(Node[PCRNum - 1].kbi[nd][2]);
		b = (int)(Node[PCRNum - 1].kbi[nd][3]);					// 
	}

	ioffset = k * hb / intmax + b / intmax256;

	lb = (int)LowByte;
	lbc = lb + ioffset;

	if (hb > 128) {
		hbi = 128 + (hb - 128) / 2;
	}
	else {
		hbi = hb;
	}

	// Use lbc, not hbln to calculate sawtooth correction, as hbln tends to be a little jittery	
	ioffset += (lbc - 128) * c * (300 - hbi) / (12 * 300 * intmax256);		// 12/19/2016 modification, shrinking sawtooth.
	lbc = lb + ioffset;												// re-calc lbc, 2 pass algorithm

	if (lbc > 255) lbc = 255;
	else if (lbc < 0) lbc = 0;

	lbp = hbln * 16 + 7;

	int lbpc = lbp - ioffset;				// lpb - ioffset: low byte predicted from the high byte low nibble BEFORE correction
	int qerr = lbp - lbc;					// if the lbc is correct, this would be the quantization error. If it is too large, maybe lb was the saturated "stuck" version

	if (lbpc > 255 + 20) {					// We allow some correction error, because hbln may have randomly flipped.
		oflow = true; *flag = 1;
	}
	else if (lbpc > 255 && qerr > 28) {		// Again we allow some tolerance because hbln may have drifted, leading to fake error
		oflow = true; *flag = 2;
	}
	else if (lbpc > 191 && qerr > 52) {
		oflow = true; *flag = 3;
	}
	else if (qerr > 96) {
		oflow = true; *flag = 4;
	}
	else if (lbpc < -20) {
		uflow = true; *flag = 5;
	}
	else if (lbpc < 0 && qerr < -28) {
		uflow = true; *flag = 6;
	}
	else if (lbpc < 64 && qerr < -52) {
		uflow = true; *flag = 7;
	}
	else if (qerr < -96) {
		uflow = true; *flag = 8;
	}
	else {
		*flag = 0;
	}

	if (oflow || uflow) {
		result = hb * 16 + 7;
	}
	else {
		result = hbhn * 256 + lbc;
	}

	if (calib2) return result;

#ifdef DARK_MANAGE

	if (!gain_mode)
		result += -(int)(Node[PCRNum - 1].fpni[1][nd]) + DARK_LEVEL;		// high gain
	else
		result += -(int)(Node[PCRNum - 1].fpni[0][nd]) + DARK_LEVEL;		// low gain

	if (result < 0) result = 0;

#endif

	return result;
}

// EEProm buffer related stuff

void CTrimReader::Convert2Int()
{
	int intmax = 32767;
	int intmax256 = 128;

	int i;
	curNode = Node;

	for (i = 0; i<12; i++) {
		curNode->kbi[i][0] = (int)round(curNode->kb[i][0] * (double)intmax);
		curNode->kbi[i][1] = (int)round(curNode->kb[i][1] * (double)intmax256);
		curNode->kbi[i][2] = (int)round(curNode->kb[i][2] * (double)intmax);
		curNode->kbi[i][3] = (int)round(curNode->kb[i][3] * (double)intmax256);
		curNode->kbi[i][4] = (int)round(curNode->kb[i][4] * (double)intmax256);
		curNode->kbi[i][5] = (int)round(curNode->kb[i][5] * (double)intmax256);
	}

	for (i = 0; i<12; i++) {
		curNode->fpni[0][i] = (int)round(curNode->fpn[0][i]);
		curNode->fpni[1][i] = (int)round(curNode->fpn[1][i]);
	}
}

int  CTrimReader::Add2TrimBuff(int i, int val)
{
	int k = Node[i].tbuff_size;
	if (k >= MAX_TRIMBUFF - 1) return -1;

	Node[i].trim_buff[k] = val >> 8;					//
	Node[i].trim_buff[k + 1] = val;				//
	Node[i].tbuff_size = k + 2;

	return k + 2;
}

int  CTrimReader::Add2TrimBuff(int i, BYTE val)
{
	int k = Node[i].tbuff_size;
	if (k >= MAX_TRIMBUFF) return -1;

	Node[i].trim_buff[k] = val;							//
	Node[i].tbuff_size = k + 1;

	return k + 1;
}

int  CTrimReader::WriteTrimBuff(int k)
{
	int i, j;
	Node[k].tbuff_size = 0;									// initialize write pointer

	int sn = _tstoi(g_ChipID);

	BYTE b0 = (BYTE)sn;
	BYTE b1 = sn >> 8;

	Add2TrimBuff(k, (BYTE)b0);
	Add2TrimBuff(k, (BYTE)b1);
	Add2TrimBuff(k, (BYTE)('O'));							// O for ONT, Oxford Nanopore Tech

	for (i = 0; i < TRIM_IMAGER_SIZE; i++) {
		for (j = 0; j < 6; j++) {
			Add2TrimBuff(k, (int)Node[k].kbi[i][j]);		// kb
		}
	}

	for (i = 0; i < TRIM_IMAGER_SIZE; i++) {
		Add2TrimBuff(k, (int)(Node[k].fpni[0][i]));
		Add2TrimBuff(k, (int)(Node[k].fpni[1][i]));
	}

	Add2TrimBuff(k, (BYTE)(Node[k].rampgen));
	Add2TrimBuff(k, (BYTE)(Node[k].range));
	Add2TrimBuff(k, (BYTE)(Node[k].auto_v20[0]));
	Add2TrimBuff(k, (BYTE)(Node[k].auto_v20[1]));
	Add2TrimBuff(k, (BYTE)(Node[k].auto_v15));

	Add2TrimBuff(k, (int)round(29.5 * 128));				// tempcal1

	float tcal2 = 0;

#ifdef CALIB_PROGRAM

	tcal2 = (float)(g_curTemp - 29.5 * g_juncTemp);

#endif

	int r = Add2TrimBuff(k, (int)round(tcal2 * 128));


	return r;
}

int CTrimReader::TrimBuff2Int(int i)
{
	_int16 r;		// this is necessary to get negative value correctly
	int k = Node[i].tbuff_rptr;

	r = (Node[i].trim_buff[k] << 8) | (Node[i].trim_buff[k + 1]);
	Node[i].tbuff_rptr += 2;

	return (int)r;
}

BYTE CTrimReader::TrimBuff2Byte(int i)
{
	BYTE r;

	r = Node[i].trim_buff[Node[i].tbuff_rptr];
	Node[i].tbuff_rptr++;

	return r;
}

void  CTrimReader::RestoreTrimBuff(int k)
{
	int i, j;
	Node[k].tbuff_rptr = 0;				// initialize read pointer

	BYTE b0, b1, b2;
	CString cid;

	b0 = TrimBuff2Byte(k);
	b1 = TrimBuff2Byte(k);
	b2 = TrimBuff2Byte(k);

	int sn = b1 << 8 | b0;

	cid.Format("%c%d", b2, sn);

	Node[k].name = cid;

	for (i = 0; i < TRIM_IMAGER_SIZE; i++) {
		for (j = 0; j < 6; j++) {
			Node[k].kbi[i][j] = TrimBuff2Int(k);
		}
	}

	for (i = 0; i < TRIM_IMAGER_SIZE; i++) {
		Node[k].fpni[0][i] = TrimBuff2Int(k);
		Node[k].fpni[1][i] = TrimBuff2Int(k);
	}

	Node[k].rampgen = TrimBuff2Byte(k);
	Node[k].range = TrimBuff2Byte(k);
	Node[k].auto_v20[0] = TrimBuff2Byte(k);
	Node[k].auto_v20[1] = TrimBuff2Byte(k);
	Node[k].auto_v15 = TrimBuff2Byte(k);

	// tempcal restore
	Node[k].tempcal[0] = (double)TrimBuff2Int(k) / (double)128;
	Node[k].tempcal[1] = (double)TrimBuff2Int(k) / (double)128;
}

#ifdef CALIB_PROGRAM
extern BYTE EepromBuff[NUM_EPKT][EPKT_SZ + 1];		// +1 for parity
#endif

void  CTrimReader::CopyEepromBuff(int k)
{
#ifdef CALIB_PROGRAM
	for (int i = 0; i < NUM_EPKT; i++) {
		for (int j = 0; j < EPKT_SZ; j++) {			// parity not copied
			Node[k].trim_buff[i * EPKT_SZ + j] = EepromBuff[i][j];
		}
	}
#endif
}


//============Backward compatible modes====================

#define SAW_TOOTH2						// Newer Sawtooth algorithm. USe 2 pass low byte correction
#define NON_CONTIGUOUS

void CTrimReader::ParseMatrixOld()
{
	for (int i = 0; i<TRIM_IMAGER_SIZE; i++) {
		for (int j = 0; j<4; j++) {
			if (GetWord() > MaxWord)
				break;
			curNode->kb[i][j] = _tstof((LPCTSTR)CurWord); // atof(CurWord);
		}
	}
}

// NumData =  "Column Number"
// pixekNum = "Frame Size"

// With the Sawtooth method, we need to gather denser data and perform a better data fitting.

int CTrimReader::ADCCorrectionOld(int NumData, BYTE HighByte, BYTE LowByte, int pixelNum, int PCRNum, int gain_mode, int* flag)
{
	int hb, lb, lbc;
	int hbln, lbp, hbhn;
	bool oflow = false, uflow = false; //  qerr_big=false;

	CString strbuf;
	double ioffset = 0;
	int result;

	hb = (int)HighByte;

	int nd = 0;
	if (pixelNum == 12) nd = NumData;
	else nd = NumData >> 1;

	ioffset = Node[PCRNum - 1].kb[nd][0] * (double)hb + Node[PCRNum - 1].kb[nd][1];

#ifdef NON_CONTIGUOUS

	if (hb >= 128) {
		ioffset += Node[PCRNum - 1].kb[nd][3];
	}

#endif

	hbln = hb % 16;		//

	hbhn = hb / 16;		//

#ifdef SAW_TOOTH		

	ioffset += (int)(Node[PCRNum - 1].kb[nd][2] * (hbln - 7));

#endif

	//		ioffset = (int)(Node[PCRNum-1].kb[nd][0]*hb + Node[PCRNum-1].kb[nd][1] + Node[PCRNum-1].kb[nd][2] *(hbln - 7));

	lb = (int)LowByte;

	lbc = lb + (int)ioffset;

#ifdef SAW_TOOTH2							// Use lbc, not hbln to calculate sawtooth correction, as hbln tends to be a little unstable	
	ioffset += Node[PCRNum - 1].kb[nd][2] * ((double)lbc - 127) * (1 - (double)hb / 400) / 16;		// 12/19/2016 modification, shrinking sawtooth.
	lbc = lb + (int)ioffset;					// re-calc lbc, 2 pass algorithm
#endif

	lbp = hbln * 16 + 7;

	if (lbc > 255) lbc = 255;
	else if (lbc < 0) lbc = 0;

	int lbpc = lbp - (int)ioffset;				// lpb - ioffset: low byte predicted from the high byte low nibble BEFORE correction
	int qerr = lbp - lbc;					// if the lbc is correct, this would be the quantization error. If it is too large, maybe lb was the saturated "stuck" version

	if (lbpc > 255 + 20) {					// We allow some correction error, because hbln may have randomly flipped.
		oflow = true; *flag = 1;
	}
	else if (lbpc > 255 && qerr > 28) {		// Again we allow some tolerance because hbln may have drifted, leading to fake error
		oflow = true; *flag = 2;
	}
	else if (lbpc > 191 && qerr > 52) {
		oflow = true; *flag = 3;
	}
	else if (qerr > 96) {
		oflow = true; *flag = 4;
	}
	else if (lbpc < -20) {
		uflow = true; *flag = 5;
	}
	else if (lbpc < 0 && qerr < -28) {
		uflow = true; *flag = 6;
	}
	else if (lbpc < 64 && qerr < -52) {
		uflow = true; *flag = 7;
	}
	else if (qerr < -96) {
		uflow = true; *flag = 8;
	}
	else {
		*flag = 0;
	}

	//	if(abs(qerr) > 84) qerr_big = true;

	if (oflow || uflow) {
		result = hb * 16 + 7;
	}
	else {
		result = hbhn * 256 + lbc;
	}

	if (calib2) return result;

#ifdef DARK_MANAGE

	if (!gain_mode)
		result += -(int)(Node[PCRNum - 1].fpn[1][nd]) + DARK_LEVEL;		// high gain
	else
		result += -(int)(Node[PCRNum - 1].fpn[0][nd]) + DARK_LEVEL;		// low gain

	if (result < 0) result = 0;

#endif

	return result;
}







 
