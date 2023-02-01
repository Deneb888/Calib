// GraDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "PCRProjectDlg.h"
#include "GraDlg.h"
#include "afxdialogex.h"
#include <fstream>
using namespace std;

//*****************************************************************
//Constant definition
//*****************************************************************
#define TxNum 50		// the number of the buffer for sent data to COMX
#define RxNum 200		// the number of the buffer for received data from COMX

#define RowNum24 24		// display row number
#define ColNum24 24		// display column number
#define pixelsize24 10	// display size for one pixel
#define RowNum12 12		// display row number
#define ColNum12 12		// display column number
#define pixelsize12 20	// display size for one pixel

#define dprow12 0x01		// display one line with 12 pixel
#define dppage12 0x02		// display one page with 12 pixel
#define dpvideo12 0x03		// display video with 12 pixel
#define spiread	0x04		// only SPI read
#define adccvt	0x05		// only ADC conversion and SPI read
#define ndread	0x06		// non-destructive read
#define dprow24	0x07		// display one line with 24 pixel
#define dppage24 0x08		// display one page with 24 pixel
#define heatstr 0x09		// start auto-heating
#define heatstop 0x0a		// stop auto-heating
#define dpvideo24 0x0b		// display video with 24 pixel


//***************************************************************
//Global variable definition
//***************************************************************
int GraFlag = 0;			// graphic dialog flag
bool Gra_pageFlag = false;		// graphic dialog »­Ò³Ñ­»·±êÖ¾
bool Gra_videoFlag = false;		// graphic dialog videoÑ­»·±êÖ¾
BYTE GraBuf[gradatanum];	// graphic dialog transmitted data buffer


//*****************************************************************
//External variable definition
//*****************************************************************
extern BYTE TxData[TxNum];		// the buffer of sent data to COMX
extern BYTE RxData[RxNum];		// the buffer of received data from COMX

extern CString RegRecStr;				//½ÓÊÕÊý¾Ý×Ö·û´®buffer Ê®Áù½øÖÆ
extern CString Dec_RegRecStr;			//½ÓÊÕÊý¾Ý×Ö·û´®buffer Ê®½øÖÆ
extern CString Valid_RegRecStr;			//ÓÐÐ§½ÓÊÕÊý¾Ý×Ö·û´®buffer Ê®Áù½øÖÆ
extern CString Valid_Dec_RegRecStr;		//ÓÐÐ§½ÓÊÕÊý¾Ý×Ö·û´®buffer Ê®½øÖÆ


//*****************************************************************
//External function
//*****************************************************************
extern unsigned char AsicConvert (unsigned char i, unsigned char j);		//ASIC convert to HEX
extern int ChangeNum (CString str, int length);								//×Ö·û´®×ª10½øÖÆ

extern float int_time; // zd add, in ms
extern int gain_mode;  // zd add, 0 high gain, 1 low gain
int adc_result[24];    // zd add

extern BOOL g_DeviceDetected;


//=====HDR Support==========

int pdata_0[24][24];	// first pass
int pdata_1[24][24];	// second pass
int hdr_phase = 0;

int contrast = 100;

//==========================

extern CTrimDlg *g_pTrimDlg;
extern char g_TxBin;

#define C08

#if defined(A11)

	const double kb[] = {
		-0.025641026	,	0.128205128	,
		-0.451282051	,	10.25641026	,
		-0.076923077	,	30.38461538	,
		-0.451282051	,	50.25641026	,
		0.076923077		,	-30.38461538,
		-0.58974359		,	57.94871795	,
		-0.143589744	,	5.717948718	,
		0.21025641		,	-39.05128205 ,
		-0.292307692	,	10.46153846	,
		-0.233333333	,	-23.33333333 ,
		-0.076923077	,	-24.61538462 ,
		-0.564102564	,	54.82051282
	};


#elif defined(A12)

	const double kb[] = {
	0.205128205	,	-1.025641026	,
	0.358974359	,	-141.7948718	,
	0	,			50	,
	0.076923077	,	-55.38461538	,
	-0.179487179	,	-24.1025641	,
	-0.169230769	,	33.84615385	,
	0.158974359	,	-40.79487179	,
	0.302564103	,	9.487179487	,
	-0.102564103	,	-39.48717949	,
	-0.030769231	,	-43.84615385	,
	0.358974359	,	-91.79487179	,
	0.317948718	,	-91.58974359	
	};

#elif defined(A10)

const double kb[] = {
0.051282051	,	-0.256410256	,
0.318367347	,	-39.59183673	,
-0.128205128	,	75.64102564	,
0.338461538	,	7.307692308	,
-0.051282051	,	50.25641026	,
-0.128205128	,	70.64102564	,
0.302564103	,	-46.51282051	,
-0.112820513	,	70.56410256	,
0.41025641	,	52.94871795	,
0.333333333	,	8.333333333	,
0.076923077	,	59.61538462	,
0.205128205	,	38.97435897	
};

#elif defined(A14)

const double kb[] = {
-0.179487179	,	30.8974359	,
0.040816327	,	-75.20408163	,
-0.205128205	,	36.02564103	,
0.025641026	,	-5.128205128	,
-0.230769231	,	-13.84615385	,
-0.230769231	,	-53.84615385	,
-0.102564103	,	-44.48717949	,
0.128205128	,	-5.641025641	,
0	,	-55	,
0.046666667	,	-57.33333333	,
0.451282051	,	-67.25641026	,
-0.169230769	,	20.84615385	
};

#elif defined(Chip4)

const double kb[] = {
-0.230769231	,	79.15384615	,
0.244897959	,	-31.2244898	,
0	,	-45	,
0.18974359	,	-12.94871795	,
0	,	35	,
0.297435897	,	-41.48717949	,
-0.215384615	,	51.07692308	,
-0.282051282	,	51.41025641	,
0.025641026	,	19.87179487	,
-0.066666667	,	103.3333333	,
0.230769231	,	-71.15384615	,
0	,	22	
};

#elif defined(B10)

const double kb[] = {
-0.102564103,	20.51282051,
-0.163265306	,	20.81632653	,
-0.256410256	,	76.28205128	,
0.282051282	,	-36.41025641	,
0.025641026	,	74.87179487	,
0.102564103	,	-20.51282051	,
0.025641026	,	-0.128205128	,
-0.102564103	,	25.51282051	,
0.128205128	,	-75.64102564	,
0.256410256	,	-1.282051282	,
0.205128205	,	-21.02564103	,
0.020512821	,	-100.1025641
};

#elif defined(B11)

const double kb[] = {
-0.358974359	,	76.79487179	,
-0.591836735	,	62.95918367	,
-0.102564103	,	-4.487179487	,
-0.256410256	,	26.28205128	,
-0.230769231	,	-8.846153846	,
-0.153846154	,	-34.23076923	,
-0.230769231	,	96.15384615	,
-0.282051282	,	16.41025641	,
-0.051282051	,	0.256410256	,
-0.076923077	,	70.38461538	,
-0.2	,			56	,
-0.102564103	,	5.512820513	
};

#elif defined(B12)

const double kb[] = {
-0.41025641	,	12.05128205	,
-0.306122449	,	51.53061224	,
-0.143589744	,	10.71794872	,
-0.282051282	,	41.41025641	,
-0.076923077	,	20.38461538	,
0.025641026	,	4.871794872	,
-0.102564103	,	25.51282051	,
-0.128205128	,	30.64102564	,
-0.128205128	,	20.64102564	,
0.005128205	,	14.97435897	,
0.143589744	,	-40.71794872	,
0.082051282	,	-20.41025641	

};

#elif defined(B13)

const double kb[] = {
-0.379487179	,	31.8974359	,
-0.102040816	,	25.51020408	,
-0.338461538	,	-2.307692308	,
-0.025641026	,	-54.87179487	,
-0.112820513	,	40.56410256	,
0.18974359	,	-20.94871795	,
-0.128205128	,	-4.358974359	,
-0.261538462	,	34.30769231	,
-0.256410256	,	61.28205128	,
-0.061538462	,	15.30769231	,
-0.076923077	,	45.38461538	,
-0.317948718	,	33.58974359	
};

#elif defined(B14)

const double kb[] = {
-0.333333333	,	-13.33333333	,
-0.195918367	,	55.97959184	,
-0.282051282	,	46.41025641	,
-0.333333333	,	46.66666667	,
-0.087179487	,	55.43589744	,
-0.230769231	,	-3.846153846	,
-0.358974359	,	-3.205128205	,
0	,	-10	,
-0.143589744	,	40.71794872	,
-0.087179487	,	-27.56410256	,
-0.384615385	,	46.92307692	,
-0.271794872	,	19.35897436	
};

#elif defined(B15)

const double kb[] = {
-0.358974359	,	51.79487179	,
0	,	50	,
-0.230769231	,	31.15384615	,
0	,	6	,
-0.117948718	,	25.58974359	,
-0.220512821	,	-3.897435897	,
0	,	70	,
-0.097435897	,	4.487179487	,
0.164102564	,	-35.82051282	,
-0.102564103	,	0.512820513	,
-0.153846154	,	-14.23076923	,
-0.38974359	,	49.94871795	
};

#elif defined(C01)

const double kb[] = {
-0.487179487,	107.4358974,
-0.195918367,	3.979591837,
-0.194871795,	0.974358974,
-0.246153846,	23.23076923,
-0.21025641,	-7.948717949,
-0.138461538,	12.69230769,
-0.215384615,	19.07692308,
-0.128205128,	25.64102564,
0.051282051,	44.74358974,
-0.179487179,	105.8974359,
-0.461538462,	2.307692308,
0.01025641,	17.94871795
};

#elif defined(C02)

const double kb[] = {
-0.066666667,	38.33333333,
0.102040816,	-20.51020408,
0.169230769,	-38.84615385,
-0.061538462,	42.30769231,
0.112820513,	9.435897436,
0.046153846,	7.769230769,
0.241025641,	11.79487179,
0.271794872,	-14.35897436,
0.076923077,	-40.38461538,
-0.020512821,	-17.8974359,
0.487179487,	-64.43589744,
0.292307692,	-46.46153846
};

#elif defined(C03)

const double kb[] = {
0.18974359,	-5.948717949,
0.020408163,	-5.102040816,
0,				20,
0.143589744,	-28.71794872,
0.138461538,	22.30769231,
-0.215384615,	76.07692308,
0.071794872,	-10.35897436,
-0.066666667,	-11.66666667,
0.2,			5,
0.102564103,	31.48717949,
0.015384615,	-8.076923077,
0.282051282,	13.58974359
};

#elif defined(C04)

const double kb[] = {
-0.435897436,	47.17948718,
-0.06122449,	55.30612245,
0.056410256,	-9.282051282,
-0.148717949,	57.74358974,
0.256410256,	18.71794872,
0.251282051,	13.74358974,
0.051282051,	-50.25641026,
-0.066666667,	-4.666666667,
0.261538462,	-28.30769231,
0.082051282,	59.58974359,
-0.107692308,	28.53846154,
-0.138461538,	19.69230769

};

#elif defined(C05)

const double kb[] = {
-0.051282051,	25.25641026,
0.110204082,	-50.55102041,
-0.102564103,	0.512820513,
0.158974359,	2.205128205,
-0.025641026,	5.128205128,
0.184615385,	27.07692308,
0.005128205,	9.974358974,
-0.061538462,	20.30769231,
0.220512821,	-28.1025641,
-0.102564103,	-4.487179487,
-0.241025641,	-11.79487179,
0.184615385,	1.076923077
};

#elif defined(C06)

const double kb[] = {
-0.215384615	,	31.07692308	,
0.13877551	,	-15.69387755	,
0.2	,	-4	,
0.066666667	,	31.66666667	,
0.246153846	,	-16.23076923	,
0.041025641	,	33.79487179	,
-0.041025641	,	33.20512821	,
-0.01025641	,	20.05128205	,
0.169230769	,	21.15384615	,
0.215384615	,	-21.07692308	,
0.025641026	,	7.871794872	,
-0.046153846	,	7.230769231	
};

#elif defined(C07)

const double kb[] = {
0.153846154	,	-55.76923077	,
0.044897959	,	1.775510204	,
-0.169230769	,	0.846153846	,
-0.015384615	,	28.07692308	,
0.18974359	,	37.05128205	,
0.241025641	,	-74.20512821	,
-0.235897436	,	7.179487179	,
0.117948718	,	-48.58974359	,
0.138461538	,	-35.69230769	,
-0.138461538	,	59.69230769	,
0.097435897	,	-42.48717949	,
-0.205128205	,	6.025641026	
};

#elif defined(C08)

const double kb[] = {
-0.056410256	,	8.282051282	,
-0.244897959	,	16.2244898	,
-0.112820513	,	0.564102564	,
0.061538462,	37.69230769,
-0.117948718	,	51.58974359	,
-0.164102564	,	23.82051282	,
-0.138461538	,	27.69230769	,
-0.194871795	,	0.974358974	,
-0.153846154	,	-4.230769231	,
0.153846154	,	17.23076923	,
0.256410256	,	18.71794872	,
-0.282051282	,	46.41025641	
};

#endif

	// Dark management/////////////// zd add

#if defined(A11)

	const double FPN[] = {165.9833333, 185.8666667, 275.2666667, 135.8833333, 227.5666667, 159.8666667, 150.6333333, 130.8666667, 162, 244.0666667, 179.8833333, 170.6333333};
	const double FPN_hg[] = {381.35, 388.6666667, 438.0166667, 343.2833333,	404, 368.3333333, 358.4666667, 343.4833333,	374.2666667, 420.0833333, 391.65, 375.3166667};

	const char auto_v20_lg = 0x08;
	const char auto_v20_hg = 0x08;

#elif defined(A12)
		
	const double FPN[] = {157.0166667,	50.18333333,	194.2833333,	93.85,	80.15,	135.7,	89.23333333,	129.8833333,	127.9833333,	125.4833333,	99.91666667,	28.16666667
};
	const double FPN_hg[] = {180.2,	75.03333333,	217.9166667,	118.1833333,	103.9333333,	158.5,	113.0833333,	152.6,	151.45,	149.0333333,	123.65,	51.15
};

	const char auto_v20_lg = 0x09;
	const char auto_v20_hg = 0x0b;

#elif defined(A10)

	const double FPN[] = {182.875,	188.0416667,	338.8125,	217.4270833,	137.2604167,	203.65625,	201.6979167,	111.0208333,	158.3125,	268.25,	327.15625,	119.8125};

	const double FPN_hg[] = {142.3020833,	145.2395833,	281.8333333,	174.9375,	97.69791667,	163.375,	160.7395833,	78.38541667,	115.125,	205.5416667,	273.6354167,	78.73958333};


	const char auto_v20_lg = 0x08;
	const char auto_v20_hg = 0x0b;

#elif defined(A14)

	const double FPN[] = {148.5,	58.21666667,	198.0833333,	69.31666667,	124.8,	110.05,	136.0166667,	161.6,	97.11666667,	112.0833333,	104.3833333,	168.6166667
};

	const double FPN_hg[] = {172.2,	84.41666667,	237.2333333,	96.15,	149.5833333,	135.4,	158.8166667,	188.5833333,	122.2666667,	137.15,	129.4,	192.7
};


	const char auto_v20_lg = 0x09;
	const char auto_v20_hg = 0x0b;

#elif defined(Chip4)

	const double FPN[] = {178.5333333,	78.6,	142.2333333,	162.9,	167.2833333,	130.3,	71.76666667,	82.86666667,	179.7833333,	161.1166667,	148.45,	182
};

	const double FPN_hg[] = {211.9833333,	108.7166667,	170.9333333,	191.6,	196.5,	161.5,	99.61666667,	111.7666667,	212.2166667,	189.6,	179.9333333,	219.6166667
};


	const char auto_v20_lg = 0x09;
	const char auto_v20_hg = 0x0b;

#elif defined(B10)

	const double FPN[] = {141.9027778,	78.36111111,	124.7083333,	116.5694444,	178.5694444,	127.5694444,	26.75,	107.6388889,	62.69444444,	80,	90.65277778,	55.33333333
	};

	const double FPN_hg[] = {162.4861111,	96.625,	    141.8472222,	139.4305556,	199.0833333,	150.4027778,	45.55555556,	127.1111111,	87.125,	      99.66666667,	112.9444444,	76.90277778
	};

	const char auto_v20_lg = 0x09;
	const char auto_v20_hg = 0x0b;

#elif defined(B11)

	const double FPN[] = {131.7,	186.5666667,	113.7333333,	166.6833333,	126.3,	191.7333333,	262.3333333,	193.4833333,	135.1166667,	192.2833333,	315.7333333,	139.8333333
};

	const double FPN_hg[] = {150.85,	205.4,	132.7666667,	186.8,	145.2666667,	210.3833333,	282.2,	212.4833333,	154.2166667,	211.1,	328.1666667,	157.4
};

	const char auto_v20_lg = 0x09;
	const char auto_v20_hg = 0x0b;

#elif defined(B13)

//	const double FPN[] = {170.6837607,	204.1623932,	144.2564103,	96.41880342,	189.4188034,	134.4615385,	97.85470085,	170.4529915,	159.008547,	140.4102564,	303.1794872,	147.2393162
//};

	const double FPN[] = {168.1547619,	196.6785714,	122,	68.48809524,	176.3571429,	120.952381,	73.39285714,	159.547619,	156.3452381,	129.5119048,	251.6309524,	133.3571429
	};


//	const double FPN_hg[] = {198.9,	298.1444444,	157.0666667,	107.3555556,	245.7888889,	150.9,	109.9666667,	189.6333333,	182.6444444,	161.1888889,	325.9888889,	165.0555556
//};

	const double FPN_hg[] = {189.09375,	255.7395833,	139.84375,	86.16666667,	196.5,	139.9270833,	91.97916667,	177.5625,	176.0208333,	148.2395833,	308.0625,	151.75
	};

	const char auto_v20_lg = 0x09;
	const char auto_v20_hg = 0x0b;

#elif defined(B14)

	const double FPN[] = {153.9166667,	202.5416667,	288.7604167,	214.90625,	166.4791667,	90.02083333,	74.10416667,	238.875,	152.0625,	147.6041667,	139.09375,	92.77083333
};

	const double FPN_hg[] = {189,	272.7777778,	317.3055556,	300.5138889,	202.5972222,	127.1666667,	112.1527778,	298.7777778,	187.2777778,	184.4166667,	176.5972222,	129.1944444
};

	const char auto_v20_lg = 0x08;
	const char auto_v20_hg = 0x0a;

#elif defined(B15)

	const double FPN[] = {149.9583333,	154.4583333,	140.4305556,	99.55555556,	137.3055556,	92.40277778,	162.5833333,	126.4722222,	105.0277778,	140.5277778,	115.75,	114.2638889

};

	const double FPN_hg[] = {172.4,	174.8666667,	163.4666667,	120.65,	160.4833333,	113.15,	185.6166667,	147.4,	128.05,	160.8333333,	139.0333333,	134.5833333

};

	const char auto_v20_lg = 0x09;
	const char auto_v20_hg = 0x0b;

#elif defined(C01)

	const double FPN[] = {272,	107.2777778,	153.9305556,	94.44444444,	74.51388889,	111.7083333,	161.4722222,	101.7222222,	203.5833333,	171.0138889,	126.0833333,	170.7777778


};

	const double FPN_hg[] = {310.7777778,	138.1388889,	183.6388889,	126.9583333,	104.3055556,	144.9444444,	192.25,	132.4444444,	273.2222222,	203.4722222,	155.3611111,	201.4861111


};

	const char auto_v20_lg = 0x09;
	const char auto_v20_hg = 0x0b;

#elif defined(C02)

	const double FPN[] = {151.8888889,	75.19444444,	123.9444444,	286.5555556,	140.7916667,	97.27777778,	72.48611111,	96.66666667,	48.56944444,	80.04166667,	88.47222222,	103.0277778
};

	const double FPN_hg[] = {176.9166667,	98.20833333,	149.9166667,	315.7916667,	166.2916667,	122.1111111,	98.48611111,	121.8194444,	74.5,	103.5138889,	112.8888889,	127.2361111
};

	const char auto_v20_lg = 0x09;
	const char auto_v20_hg = 0x0b;

#elif defined(C03)

	const double FPN[] = {164.8888889,	127.1527778,	328.4861111,	131.4583333,	236.125,	143.7361111,	200.3194444,	166.7638889,	210.5,	203.5555556,	102.75,	148.3888889
};

	const double FPN_hg[] = {200.9305556,	161.3055556,	364.4444444,	167.2777778,	297.1527778,	179.25,	257.4444444,	204.1527778,	277.3194444,	262.1944444,	138.7916667,	184.6527778
};

	const char auto_v20_lg = 0x09;
	const char auto_v20_hg = 0x0b;

#elif defined(C04)

	const double FPN[] = {105.125,	130.8333333,	190.8472222,	130.8055556,	160.6666667,	123.375,	64.5,	88.56944444,	135.5416667,	154.4027778,	144.4861111,	148.4722222

};

	const double FPN_hg[] = {129.3611111,	154.5833333,	263.6805556,	155.4305556,	187.1805556,	147.3472222,	88.80555556,	112.9583333,	158.7222222,	176.7777778,	168.6527778,	171.8333333

};

	const char auto_v20_lg = 0x09;
	const char auto_v20_hg = 0x0b;

#elif defined(C05)

	const double FPN[] = {280.0416667,	202.1527778,	162.125,	198.7083333,	180.125,	286.4027778,	210.0277778,	159.125,	143.4166667,	179.7361111,	107.8333333,	265.0277778

};

	const double FPN_hg[] = {309.4444444,	250.375,	190.8333333,	265.375,	207.5555556,	317.4583333,	294.8888889,	188.1111111,	171.8055556,	207.625,	136.5972222,	299.75

};

	const char auto_v20_lg = 0x08;
	const char auto_v20_hg = 0x0a;

#elif defined(C06)

//	const double FPN[] = {103.0138889,	112.7222222,	93.54166667,	143.25,	134.4166667,	172.0972222,	78.98611111,	122.6805556,	131.875,	107.2361111,	177.1805556,	114.1805556
//};

//	const double FPN_hg[] = {116.8194444,	123.0416667,	100.875,	148.0972222,	142.7777778,	180.4583333,	89.45833333,	134.5277778,	138.0555556,	115.25,	184.2777778,	120.2777778
//};

	const double FPN[] = {82.19444444,	83.43055556,	58.01388889,	95.77777778,	99.19444444,	137.8055556,	54.72222222,	101.9861111,	67.79166667,	74.40277778,	143.8055556,	61.77777778
};

	const double FPN_hg[] = {96.08333333,	96.77777778,	69.43055556,	106.5138889,	112.2777778,	149.1666667,	67.08333333,	115.3472222,	77.61111111,	89.125,	155.5972222,	73.63888889
};

	const char auto_v20_lg = 0x0a;
	const char auto_v20_hg = 0x0c;

#elif defined(C07)

	const double FPN[] = {104.8055556,	139.2222222,	167.6805556,	126.2638889,	168.4583333,	63.43055556,	71.84722222,	102.4305556,	130.1111111,	127.0833333,	100.2916667,	156.9305556
};

	const double FPN_hg[] = {126.6805556,	159.4722222,	188.3472222,	148.0416667,	189.1527778,	84.80555556,	91.68055556,	122.2916667,	150.1944444,	147.8055556,	120.75,	179.7361111


};

	const char auto_v20_lg = 0x09;
	const char auto_v20_hg = 0x0b;

#elif defined(C08)

	const double FPN[] = {175.7083333,	161,	143.8472222,	252.2777778,	174.4166667,	103,	108.8194444,	113.7638889,	139.75,	183.4305556,	158.4861111,	146.125

};

	const double FPN_hg[] = {211.25,	185.5,	170.3194444,	313.8472222,	198.9583333,	128.6666667,	135.0416667,	140.1944444,	166.0277778,	209.1944444,	185.7222222,	171.5694444

};

	const char auto_v20_lg = 0x09;
	const char auto_v20_hg = 0x0b;
			
#endif

#define ADC_CORRECT
#define DARK_MANAGE


#define DARK_LEVEL 100

//------------------

// CGraDlg dialog

IMPLEMENT_DYNAMIC(CGraDlg, CDialogEx)

CGraDlg::CGraDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGraDlg::IDD, pParent)
	, m_GainMode(0)
	, m_FrameSize(0)
	, m_OrigOut(FALSE)
{

	m_EditReadRow = _T("");
	m_ShowAllDataInt = 0;
	m_ReadHexInt = 0;
	m_PixelData = _T("");
	m_ADCRecdata = _T("");

//	m_GainMode = 0;
//	m_FrameSize = 0;
	m_PixOut = true;
//	m_OrigOut = false;

	for(int i=0; i<24; i++) {
		adc_result[i] = 0;
	}
}

CGraDlg::~CGraDlg()
{
}

void CGraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_READROW, m_EditReadRow);
	DDX_Control(pDX, IDC_RADIO_SHOWALLDATA, m_ShowAllData);
	DDX_Radio(pDX, IDC_RADIO_SHOWALLDATA, m_ShowAllDataInt);
	DDX_Control(pDX, IDC_RADIO_READHEX, m_ReadHex);
	DDX_Radio(pDX, IDC_RADIO_READHEX, m_ReadHexInt);
	DDX_Control(pDX, IDC_RADIO_READDEC, m_ReadDec);
	DDX_Control(pDX, IDC_RADIO_ADCDATA, m_ADCData);
	DDX_Control(pDX, IDC_RADIO_SHOWVALIDDATA, m_ShowValidData);
	DDX_Text(pDX, IDC_EDIT_RecData, m_PixelData);
	DDX_Text(pDX, IDC_EDIT_ADCDATA, m_ADCRecdata);
	DDX_Radio(pDX, IDC_RADIOLOWGAIN, m_GainMode);
	DDX_Radio(pDX, IDC_RADIO_12, m_FrameSize);
	DDX_Check(pDX, IDC_CHECK_PIXOUT, m_PixOut);
	DDX_Check(pDX, IDC_CHECK_ORIGOUT, m_OrigOut);
}


BEGIN_MESSAGE_MAP(CGraDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_READROW, &CGraDlg::OnClickedBtnReadrow)
	ON_MESSAGE(UM_GRAPROCESS,OnGraProcess)
	ON_BN_CLICKED(IDC_BTN_DPROW24, &CGraDlg::OnBnClickedBtnDprow24)
	ON_BN_CLICKED(IDC_BTN_DPPAGE12, &CGraDlg::OnBnClickedBtnDppage12)
	ON_BN_CLICKED(IDC_BTN_DPPAGE24, &CGraDlg::OnBnClickedBtnDppage24)
	ON_BN_CLICKED(IDC_BTN_DPVEDIO, &CGraDlg::OnBnClickedBtnDpvedio)
	ON_BN_CLICKED(IDC_BTN_STOPVIDEO, &CGraDlg::OnBnClickedBtnStopvideo)
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CGraDlg::OnBnClickedBtnClear)
	ON_BN_CLICKED(IDC_BTN_ADCCONVERT, &CGraDlg::OnBnClickedBtnAdcconvert)
	ON_BN_CLICKED(IDC_BTN_CLEARADC, &CGraDlg::OnBnClickedBtnClearadc)
	ON_BN_CLICKED(IDC_BUTTON1, &CGraDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CGraDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BTN_DPVIDEO24, &CGraDlg::OnBnClickedBtnDpvideo24)
	ON_BN_CLICKED(IDC_RADIO_ADCDATA, &CGraDlg::OnBnClickedRadioAdcdata)
//	ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_SLIDER1, &CGraDlg::OnThumbposchangingSlider1)
//	ON_WM_HSCROLL()
//	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CGraDlg::OnNMCustomdrawSlider1)
ON_WM_HSCROLL()
ON_BN_CLICKED(IDC_RADIOLOWGAIN, &CGraDlg::OnClickedRadiolowgain)
//ON_UPDATE_COMMAND_UI(IDC_RADIOHDR, &CGraDlg::OnUpdateRadiohdr)
//ON_UPDATE_COMMAND_UI(IDC_RADIOHIGHGAIN, &CGraDlg::OnUpdateRadiohighgain)
ON_COMMAND(IDC_RADIOHIGHGAIN, &CGraDlg::OnRadiohighgain)
ON_COMMAND(IDC_RADIOHDR, &CGraDlg::OnRadiohdr)
ON_BN_CLICKED(IDC_RADIO_12, &CGraDlg::OnClickedRadio12)
ON_COMMAND(IDC_RADIO_24, &CGraDlg::OnRadio24)
ON_BN_CLICKED(IDC_BUTTON_CAPTURE, &CGraDlg::OnBnClickedButtonCapture)
ON_BN_CLICKED(IDC_CHECK_PIXOUT, &CGraDlg::OnClickedCheckPixout)
ON_BN_CLICKED(IDC_CHECK_ORIGOUT, &CGraDlg::OnClickedCheckOrigout)
ON_BN_CLICKED(IDC_BTN_READSPI, &CGraDlg::OnBnClickedBtnReadspi)
END_MESSAGE_MAP()


//*********************************************************************
//Own function
//*********************************************************************

//µ÷ÓÃÖ÷¶Ô»°¿ò¶ÔÓ¦ÏûÏ¢´¦Àíº¯Êý
void CGraDlg::GraCalMainMsg()
{
	WPARAM a = 8;
	LPARAM b = 9;
	HWND hwnd = AfxGetApp()->GetMainWnd()->GetSafeHwnd();
	::SendMessage(hwnd,WM_GraDlg_event,a,b);
}


LRESULT CGraDlg::OnGraProcess(WPARAM wParam, LPARAM lParam)
{
	// 12 / 24 pixel distinguish
	BYTE type;
	type = RxData[4];	//´Ó½ÓÊÕÊý¾ÝÖÐÈ¡³ötype·ÖÖ§

	//Êý¾Ý´¦Àí¼°ÏÔÊ¾

	int pixelNum;

	// ¸ù¾Ý·µ»ØµÄtypeÇø·ÖÊÇÏÔÊ¾12ÁÐÊý¾Ý»¹ÊÇ24ÁÐÊý¾Ý
	switch(type)
	{
	case dprow12:		// 12ÁÐ»­ÐÐ
	case dppage12:		// 12ÁÐ»­Ò³
	case dpvideo12:		// 12ÁÐ»­vedio
	case 0x0c:			// »ý·ÖÊ±¼ä³¬¹ý10ms
		pixelNum = 12;
		break;

	case dprow24:		// 24ÁÐ»­ÐÐ
	case dppage24:		// 24ÁÐ»­Ò³
	case dpvideo24:		// 24ÁÐ»­vedio
		pixelNum = 24;
		break;

	default: break;
	}

	WORD * DecData = new WORD [pixelNum];
	int NumData = 0;
	int iDecVal;
	CString TmHex;
	CString sDecData;	// ×îºóÏÔÊ¾µÄÊ®½øÖÆ×Ö·û´®
	char sDecVal[10];
	BYTE lByte;
	CString sRownum;
	sRownum.Format(" %d",RxData[5]);

	// zd comment RxData[5] is the row number.
	int rn = RxData[5];

	for (NumData=0; NumData<pixelNum; NumData++)			//½«Ã¿Á½¸öbyteÕûºÏ³ÉÒ»¸öword
	{
		lByte=RxData[NumData*2+6];				//È¡³öµÍ4Î»byte
		lByte = lByte<<4;						//½«µÍ4Î»byte×óÒÆ4Î»
		DecData[NumData] = RxData[NumData*2+7];		//½«¸ß8Î»byte¸³Öµ¸øword±äÁ¿
		DecData[NumData] <<= 8;						//word buffer×óÒÆ8Î»£¬½«¸ß8Î»byteÊý¾Ý·ÅÖÃµ½¸ß8Î»
		DecData[NumData] |= lByte;				//½«µÍ4Î»byte·Åµ½word bufferµÍ8Î»
		DecData[NumData] >>= 4;						//½«word bufferÕûÌåÓÒÒÆ4Î»£¬±ä³ÉÓÐÐ§12Î»Êý¾Ý
	}

	for (NumData=0; NumData<pixelNum; NumData++)			//½«Ò»ÐÐÖÐµÄÃ¿¸öPixelÊ®Áù½øÖÆÓÐÐ§Êý¾Ý×ª³ÉÊ®½øÖÆ
	{
		TmHex.Format("%2X",DecData[NumData]);		//½«Ã¿¸öword buffer×ª³ÉÊ®Áù½øÖÆ×Ö·û´®
		iDecVal = ChangeNum(TmHex, TmHex.GetLength());		//½«Ã¿¸öÊ®Áù½øÖÆ×Ö·û´®×ª³ÉÓÐÐ§Ê®½øÖÆÊý
		gcvt(iDecVal,4,sDecVal);		//½«Ê®½øÖÆ¸¡µãÊý×ª»»³É×Ö·û´®
		//µÚ¶þ¸ö²ÎÊý´ú±íÊ®½øÖÆÊýÓÐ¶àÉÙÎ»
		//µÚÈý¸ö²ÎÊý£¬±ØÐëÎªchar *, ÈçÉÏÃæ¶ÔsDecValµÄ¶¨Òå

		sDecData += sDecVal;				//½«char* ¸³Öµ¸øCString£¬ÓÃÀ´ÏÔÊ¾
		sDecData += "  ";					//Ã¿¸öÊý¾Ý¼ä¼Ó¿Õ¸ñ
	}

	sDecData += sRownum;
	delete[] DecData;

	// µ±·µ»ØÃüÁîÎª0x14Ê±£¬Ö»ÏÔÊ¾ÏÂÊö×Ö·û´®
	if (RxData[2] == 0x14)
	{
		sDecData = "";
		sDecData.Format("the cycle number is %d",RxData[3]);
	}

	int ioffset = 0; // ilbc, iresult;
	int lbc,result;
	int TemInt;
	char fstrbuf[9];
	CString TemHex;
	CString sADCData;	// ×îºóÏÔÊ¾µÄADCÊý¾Ý×Ö·û´®
	int hb,lb;
	int hbln,lbp,hbhn;
	bool oflow,uflow;
	BYTE LowByte,HighByte;
//	BYTE bTem;

	CString soffset,slbc,shbln,slbp,shbhn,sresult;

	for (NumData=0; NumData<pixelNum; NumData++)
	{
		//¸ß8Î»£¨1byte£©Êý¾Ý´¦Àí£¬Ëãoffset(step1)¡¢ÅÐ¶ÏÒç³ö(step2)¡¢Ëãresult(step3)
		HighByte=RxData[NumData*2+7];
		TemHex.Format("%2X",HighByte);
		TemInt = ChangeNum(TemHex, TemHex.GetLength());
		hb = TemInt;

		int nd = 0;
	
		if(pixelNum == 12) nd= NumData;
		else nd = NumData>>1;

#ifdef ADC_CORRECT
		ioffset = (int)(kb[2*nd]*hb + kb[2*nd+1]);
#endif

		hbln = hb % 16;		//µÃµ½ÓÃÓÚÒç³öÅÐ¶ÏµÄhbln  step 2

		hbhn = hb / 16;		//µÃµ½ÓÃÓÚÇóresultµÄhbhn  stpe3

		//µÍ8Î»£¨1byte£©Êý¾Ý´¦Àí£¬Ëãlbc
		LowByte = RxData[NumData*2+6];
		TemHex.Format("%2X",LowByte);
		TemInt = ChangeNum(TemHex,TemHex.GetLength());
		lb = TemInt;
		lbc = lb + ioffset;

		//ÅÐ¶ÏÒç³ö
		TemHex.Format("%2X",hbln);
		TemInt = ChangeNum(TemHex,TemHex.GetLength());
		lbp = TemInt*16+7;

		if ( (lbp-ioffset) > 255 + 32)
			oflow = true;
		else if( (lbp-ioffset) > 255 && (lbp - lbc) > 48)
			oflow = true;
		else if( (lbp-ioffset) > 191 && (lbp - lbc) > 127)
			oflow = true;
		else
		{
			if ( (lbp-ioffset) < 0 - 32)
				uflow = true;
			else if((lbp-ioffset) < 0 && (lbp - lbc) < -48)
				uflow = true;
			else if((lbp-ioffset) < 64 && (lbp - lbc) < -127)
				uflow = true;
			else
			{
				oflow = false;
				uflow = false;
			}
		}

		if (oflow)
			result = hb * 16 + 7;
		else
		{
			if (uflow)
				result = hb * 16 + 7;
			else
				result = hbhn * 256 + lbc;
		}


#ifdef DARK_MANAGE

		if(!gain_mode) result += -(int) (FPN_hg[nd]) + DARK_LEVEL;
		else result += -(int) (FPN[nd]) + DARK_LEVEL;
#endif

		if(result < 0) result = 0;

// Bad pixel correct
#ifdef B11

	if(pixelNum == 24 && NumData == 21 && rn == 11) result *= 0.90; 
	else if(pixelNum == 12 && NumData == 10 && rn == 5 && g_TxBin == 0x4) result *= 0.90; 
	else if(pixelNum == 12 && NumData == 10 && rn == 5 && g_TxBin == 0x5) result *= 0.945; 
	else if(pixelNum == 12 && NumData == 10 && rn == 5 && g_TxBin == 0x6) result *= 0.945;
	else if(pixelNum == 12 && NumData == 10 && rn == 5 && g_TxBin == 0xc) result *= 0.945; 

#endif

		adc_result[NumData] = result; // zd add

		itoa (result,fstrbuf,10);		//½«½á¹û×ª³É×Ö·û´®ÏÔÊ¾
		sADCData += fstrbuf;
		sADCData += " ";
	}

	sADCData += sRownum;

	// µ±·µ»ØÃüÁîÎª0x14Ê±£¬Ö»ÏÔÊ¾ÏÂÊö×Ö·û´®
	if (RxData[2] == 0x14)
	{
		sADCData = "";
		sADCData.Format("the cycle number is %d",RxData[3]);
	}

	if (m_OrigOut && m_ReadHex.GetCheck())			//ÒÔÊ®Áù½øÖÆ
	{
		if (m_ShowAllData.GetCheck())	//ÏÔÊ¾ËùÓÐÊý¾Ý
		{
			//			m_PixelData = RegRecStr + m_PixelData;		//×îÐÂÊý¾ÝÔÚ±à¼­¿òµÚÒ»ÐÐÏÔÊ¾
			m_PixelData += (RegRecStr+"\r\n");					//×îÐÂÊý¾ÝÔÚ±à¼­¿ò×îºóÒ»ÐÐÏÔÊ¾
			SetDlgItemText(IDC_EDIT_RecData,m_PixelData);

			//±à¼­¿ò´¹Ö±¹ö¶¯µ½µ×¶Ë
			POINT pt;
			GetDlgItem(IDC_EDIT_RecData)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
			pt.x=0;
			GetDlgItem(IDC_EDIT_RecData)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
			RegRecStr = "";
		}

		if (m_ShowValidData.GetCheck())	//ÏÔÊ¾ÓÐÐ§Êý¾Ý
		{
			//			m_PixelData = Valid_RegRecStr + m_PixelData;	//×îÐÂÊý¾ÝÔÚ±à¼­¿òµÚÒ»ÐÐÏÔÊ¾
			m_PixelData += (Valid_RegRecStr+"\r\n");					//×îÐÂÊý¾ÝÔÚ±à¼­¿ò×îºóÒ»ÐÐÏÔÊ¾
			SetDlgItemText(IDC_EDIT_RecData,m_PixelData);

			//±à¼­¿ò´¹Ö±¹ö¶¯µ½µ×¶Ë
			POINT pt;
			GetDlgItem(IDC_EDIT_RecData)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
			pt.x=0;
			GetDlgItem(IDC_EDIT_RecData)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
			Valid_RegRecStr = "";
		}		

	}

	if (m_OrigOut && m_ReadDec.GetCheck())			//ÒÔÊ®½øÖÆÏÔÊ¾
	{
		//		m_PixelData = sTest + m_PixelData +"\r\n";			//Ã¿ÐÐÊý¾Ý½«¼Ó»Ø³µ
		//×îÐÂÊý¾ÝÔÚ±à¼­¿òµÚÒ»ÐÐÏÔÊ¾
		m_PixelData += (sDecData+"\r\n");						
		SetDlgItemText(IDC_EDIT_RecData,m_PixelData);		//Ã¿ÐÐÊý¾Ý½«¼Ó»Ø³µ
		//×îÐÂÊý¾ÝÔÚ±à¼­¿ò×îºóÒ»ÐÐÏÔÊ¾	

		//±à¼­¿ò´¹Ö±¹ö¶¯µ½µ×¶Ë
		POINT pt;
		GetDlgItem(IDC_EDIT_RecData)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
		pt.x=0;
		GetDlgItem(IDC_EDIT_RecData)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
	}

	if (m_PixOut && m_GainMode <= 1)		//ADCÊý¾ÝÏÔÊ¾
	{
		m_ADCRecdata += (sADCData+"\r\n");
		SetDlgItemText(IDC_EDIT_ADCDATA,m_ADCRecdata);		//Ã¿ÐÐÊý¾Ý½«¼Ó»Ø³µ
		//×îÐÂÊý¾ÝÔÚ±à¼­¿ò×îºóÒ»ÐÐÏÔÊ¾

		//±à¼­¿ò´¹Ö±¹ö¶¯µ½µ×¶Ë
		POINT pt;
		GetDlgItem(IDC_EDIT_ADCDATA)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
		pt.x=0;
		GetDlgItem(IDC_EDIT_ADCDATA)->SendMessage(EM_LINESCROLL,pt.x,pt.y);

	}

	CDC *pDC;		//´´½¨Ä¿±êDCÖ¸Õë
	pDC=GetDlgItem(IDC_Bmp)->GetDC();	


	CRect   rect;
	CBrush brush[RowNum24][ColNum24];	
	int i,l;
	int gray_level;

	for (i=0; i<ColNum24; i++)
	{
		gray_level = adc_result[i]/16;
		if(gray_level > 255) gray_level = 255;
		else if(gray_level < 0) gray_level = 0;

		brush[RxData[5]][i].CreateSolidBrush(RGB(gray_level, gray_level, gray_level));		// È¡µÍbyteÎªÓÐÐ§Î» // zd mod use corrected adc data with green tint
//		brush[RxData[5]][i].CreateSolidBrush(RGB(RxData[i*2+7],RxData[i*2+7],RxData[i*2+7]));		// È¡µÍbyteÎªÓÐÐ§Î»
//		brush[RxData[5]][i].CreateSolidBrush(RGB(RxData[i*2+6],RxData[i*2+6],RxData[i*2+6]));		// È¡¸ßbyteÎªÓÐÐ§Î»
	}

	switch(type)
	{
	case dprow12:	
		{
		
			CBitmap bmp;
			bmp.CreateCompatibleBitmap(pDC,RowNum12*pixelsize12,ColNum12*pixelsize12);	

			CDC bDC; 
			bDC.CreateCompatibleDC(pDC);	

			bDC.SelectObject(&bmp);		

		
			for(l=0; l<ColNum12; l++)		// l´ú±íÁÐºÅ£»rxdata[5]ÖÐÊý¾ÝÊÇÐÐºÅ
			{
				rect.SetRect(pixelsize12*l,pixelsize12*RxData[5],pixelsize12*(l+1),pixelsize12*(RxData[5]+1));
				bDC.Rectangle(rect);
				bDC.FillRect(&rect,&brush[RxData[5]][l]);
			}
			pDC->BitBlt(0,0,900, 380, &bDC, 0, 0, SRCCOPY);

			//Êý¾Ý´«ÊäbufferÇåÁã
			memset(RxData,0,sizeof(RxData));	

			break;
		}
	case dppage12:	
		{
			for(l=0; l<ColNum12; l++)		// l´ú±íÁÐºÅ
			{
				if(m_GainMode <= 1) {
				rect.SetRect(pixelsize12*l,pixelsize12*RxData[5],pixelsize12*(l+1),pixelsize12*(RxData[5]+1));
				pDC->Rectangle(rect);
				pDC->FillRect(&rect,&brush[RxData[5]][l]);
				}
				else {

				//zd add

				if(!hdr_phase) {
					pdata_0[rn][l] = adc_result[l];
				}
				else {
					pdata_1[rn][l] = adc_result[l];
				}
				}
			}

			//Êý¾Ý´«ÊäbufferÇåÁã
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case 0x0c:		// »ý·ÖÊ±¼ä³¬¹ý10ms
		{
			for(l=0; l<ColNum12; l++)		// l´ú±íÁÐºÅ
			{
				rect.SetRect(pixelsize12*l,pixelsize12*RxData[5],pixelsize12*(l+1),pixelsize12*(RxData[5]+1));
				pDC->Rectangle(rect);
				pDC->FillRect(&rect,&brush[RxData[5]][l]);
			}

			//Êý¾Ý´«ÊäbufferÇåÁã
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case dprow24:	
		{

			CBitmap bmp;	
			bmp.CreateCompatibleBitmap(pDC,RowNum24*pixelsize24,ColNum24*pixelsize24);	

			CDC bDC;  
			bDC.CreateCompatibleDC(pDC);	

			bDC.SelectObject(&bmp);		

		
			for(l=0; l<ColNum24; l++)		
			{
				rect.SetRect(pixelsize24*l,pixelsize24*RxData[5],pixelsize24*(l+1),pixelsize24*(RxData[5]+1));
				bDC.Rectangle(rect);
				bDC.FillRect(&rect,&brush[RxData[5]][l]);
			}
			pDC->BitBlt(0,0,900, 380, &bDC, 0, 0, SRCCOPY);

			//Êý¾Ý´«ÊäbufferÇåÁã
			memset(RxData,0,sizeof(RxData));	

			break;
		}
	case dppage24:	
		{
			for(l=0; l<ColNum24; l++)		// l´ú±íÁÐºÅ
			{
				if(m_GainMode <= 1) {
				rect.SetRect(pixelsize24*l,pixelsize24*RxData[5],pixelsize24*(l+1),pixelsize24*(RxData[5]+1));
				pDC->Rectangle(rect);
				pDC->FillRect(&rect,&brush[RxData[5]][l]);
				}
				else {
				if(!hdr_phase) {
					pdata_0[rn][l] = adc_result[l];
				}
				else {
					pdata_1[rn][l] = adc_result[l];
				}
				}
			}


			//Êý¾Ý´«ÊäbufferÇåÁã
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case dpvideo12:	//video12
		{
			for(l=0; l<ColNum12; l++)		// l´ú±íÁÐºÅ
			{
				rect.SetRect(pixelsize12*l,pixelsize12*RxData[5],pixelsize12*(l+1),pixelsize12*(RxData[5]+1));
				pDC->Rectangle(rect);
				pDC->FillRect(&rect,&brush[RxData[5]][l]);
			}

			//Êý¾Ý´«ÊäbufferÇåÁã
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case dpvideo24:	//video24
		{
			for(l=0; l<ColNum24; l++)		// l´ú±íÁÐºÅ
			{
				rect.SetRect(pixelsize24*l,pixelsize24*RxData[5],pixelsize24*(l+1),pixelsize24*(RxData[5]+1));
				pDC->Rectangle(rect);
				pDC->FillRect(&rect,&brush[RxData[5]][l]);
			}

			//Êý¾Ý´«ÊäbufferÇåÁã
			memset(RxData,0,sizeof(RxData));

			break;
		}

	default:
		break;

	}


	return 0;
}



// CGraDlg message handlers


void CGraDlg::OnClickedBtnReadrow()
{
	// TODO: Add your control notification handler code here
	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	GraFlag = sendgramsg;

	CString sRownum;
	GetDlgItemText(IDC_EDIT_READROW,sRownum);
	GraBuf[0] = atoi(sRownum);

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x01;		//data type, date edit first byte
	TxData[4] = GraBuf[0];		//real data
	TxData[5] = 0x00;		//Ô¤ÁôÎ»
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	TxData[8] = 0x00;
	TxData[9] = 0x00;
	TxData[10] = 0x00;
	TxData[11] = 0x00;
	TxData[12] = 0x00;
	TxData[13] = 0x00;
	TxData[14] = 0x00;
	TxData[15] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7]+TxData[8]+TxData[9]
	+TxData[10]+TxData[11]+TxData[12]+TxData[13]+TxData[14];		//check sum
	if (TxData[15]==0x17)
		TxData[15]=0x18;
	else
		TxData[15]=TxData[15];
	TxData[16] = 0x17;		//back code
	TxData[17] = 0x17;		//back code


	//Send message to main dialog
	GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò
	
}


void CGraDlg::OnBnClickedBtnDprow24()
{
	// TODO: Add your control notification handler code here
	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	GraFlag = sendgramsg;

	CString sRownum;
	GetDlgItemText(IDC_EDIT_READROW,sRownum);
	GraBuf[0] = atoi(sRownum);

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x07;		//data type, date edit first byte
	TxData[4] = GraBuf[0];		//real data
	TxData[5] = 0x00;		//Ô¤ÁôÎ»
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	TxData[8] = 0x00;
	TxData[9] = 0x00;
	TxData[10] = 0x00;
	TxData[11] = 0x00;
	TxData[12] = 0x00;
	TxData[13] = 0x00;
	TxData[14] = 0x00;
	TxData[15] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7]+TxData[8]+TxData[9]
	+TxData[10]+TxData[11]+TxData[12]+TxData[13]+TxData[14];		//check sum
	if (TxData[15]==0x17)
		TxData[15]=0x18;
	else
		TxData[15]=TxData[15];
	TxData[16] = 0x17;		//back code
	TxData[17] = 0x17;		//back code


	//Send message to main dialog
	GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò
}


void CGraDlg::OnBnClickedBtnDppage12()
{
	// TODO: Add your control notification handler code here

	SetGainMode(0);
	hdr_phase = 0;

	GraFlag = sendpagemsg;
	Gra_pageFlag = TRUE;	// ¿ªÊ¼Ñ­»·

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x02;		//data type, date edit first byte
	TxData[4] = 0xff;		//real data
	TxData[5] = 0x00;		//Ô¤ÁôÎ»
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	TxData[8] = 0x00;
	TxData[9] = 0x00;
	TxData[10] = 0x00;
	TxData[11] = 0x00;
	TxData[12] = 0x00;
	TxData[13] = 0x00;
	TxData[14] = 0x00;
	TxData[15] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7]+TxData[8]+TxData[9]
	+TxData[10]+TxData[11]+TxData[12]+TxData[13]+TxData[14];		//check sum
	if (TxData[15]==0x17)
		TxData[15]=0x18;
	else
		TxData[15]=TxData[15];
	TxData[16] = 0x17;		//back code
	TxData[17] = 0x17;		//back code


	//Send message to main dialog
	GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò


//===========
	SetGainMode(1);
	hdr_phase = 1;

	GraFlag = sendpagemsg;
	Gra_pageFlag = TRUE;	// ¿ªÊ¼Ñ­»·

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x02;		//data type, date edit first byte
	TxData[4] = 0xff;		//real data
	TxData[5] = 0x00;		//Ô¤ÁôÎ»
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	TxData[8] = 0x00;
	TxData[9] = 0x00;
	TxData[10] = 0x00;
	TxData[11] = 0x00;
	TxData[12] = 0x00;
	TxData[13] = 0x00;
	TxData[14] = 0x00;
	TxData[15] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7]+TxData[8]+TxData[9]
	+TxData[10]+TxData[11]+TxData[12]+TxData[13]+TxData[14];		//check sum
	if (TxData[15]==0x17)
		TxData[15]=0x18;
	else
		TxData[15]=TxData[15];
	TxData[16] = 0x17;		//back code
	TxData[17] = 0x17;		//back code


	//Send message to main dialog
	GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò

	DisplayHDR();

//===========
}


void CGraDlg::OnBnClickedBtnDppage24()
{
	// TODO: Add your control notification handler code here
	SetGainMode(0);
	hdr_phase = 0;

	GraFlag = sendpagemsg;
	Gra_pageFlag = TRUE;	// ¿ªÊ¼Ñ­»·

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x08;		//data type, date edit first byte
	TxData[4] = 0xff;		//real data
	TxData[5] = 0x00;		//Ô¤ÁôÎ»
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	TxData[8] = 0x00;
	TxData[9] = 0x00;
	TxData[10] = 0x00;
	TxData[11] = 0x00;
	TxData[12] = 0x00;
	TxData[13] = 0x00;
	TxData[14] = 0x00;
	TxData[15] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7]+TxData[8]+TxData[9]
	+TxData[10]+TxData[11]+TxData[12]+TxData[13]+TxData[14];		//check sum
	if (TxData[15]==0x17)
		TxData[15]=0x18;
	else
		TxData[15]=TxData[15];
	TxData[16] = 0x17;		//back code
	TxData[17] = 0x17;		//back code

	//Send message to main dialog
	GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò

//==========Second pass====================

	SetGainMode(1);
	hdr_phase = 1;

	GraFlag = sendpagemsg;
	Gra_pageFlag = TRUE;	// ¿ªÊ¼Ñ­»·

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x08;		//data type, date edit first byte
	TxData[4] = 0xff;		//real data
	TxData[5] = 0x00;		//Ô¤ÁôÎ»
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	TxData[8] = 0x00;
	TxData[9] = 0x00;
	TxData[10] = 0x00;
	TxData[11] = 0x00;
	TxData[12] = 0x00;
	TxData[13] = 0x00;
	TxData[14] = 0x00;
	TxData[15] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7]+TxData[8]+TxData[9]
	+TxData[10]+TxData[11]+TxData[12]+TxData[13]+TxData[14];		//check sum
	if (TxData[15]==0x17)
		TxData[15]=0x18;
	else
		TxData[15]=TxData[15];
	TxData[16] = 0x17;		//back code
	TxData[17] = 0x17;		//back code

	//Send message to main dialog
	GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò

	DisplayHDR24();
}

//Start Video
void CGraDlg::OnBnClickedBtnDpvedio()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	if(!m_GainMode) {	
		SetGainMode(1);
	}
	else if(m_GainMode == 1) {
		SetGainMode(0);
	}
	else {
		MessageBox("Video in HDR mode not allowed");
		return;
	}

	GraFlag = sendvideomsg;
	Gra_videoFlag = true;	// video¿ªÊ¼Ñ­»·
	Gra_pageFlag = TRUE;	// »­Ò³¿ªÊ¼Ñ­»·

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x02;		//data type, date edit first byte
	TxData[4] = 0xff;		//real data
	TxData[5] = 0x00;		//Ô¤ÁôÎ»
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	TxData[8] = 0x00;
	TxData[9] = 0x00;
	TxData[10] = 0x00;
	TxData[11] = 0x00;
	TxData[12] = 0x00;
	TxData[13] = 0x00;
	TxData[14] = 0x00;
	TxData[15] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7]+TxData[8]+TxData[9]
	+TxData[10]+TxData[11]+TxData[12]+TxData[13]+TxData[14];		//check sum
	if (TxData[15]==0x17)
		TxData[15]=0x18;
	else
		TxData[15]=TxData[15];
	TxData[16] = 0x17;		//back code
	TxData[17] = 0x17;		//back code

	//Send message to main dialog
	GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò
}


void CGraDlg::OnBnClickedBtnDpvideo24()
{
	// TODO: Add your control notification handler code here
	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	if(!m_GainMode) {	
		SetGainMode(1);
	}
	else if(m_GainMode == 1) {
		SetGainMode(0);
	}
	else {
		MessageBox("Video in HDR mode not allowed");
		return;
	}

	GraFlag = sendvideomsg;
	Gra_videoFlag = true;	// video¿ªÊ¼Ñ­»·
	Gra_pageFlag = TRUE;	// »­Ò³¿ªÊ¼Ñ­»·

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x08;		//data type, date edit first byte
	TxData[4] = 0xff;		//real data
	TxData[5] = 0x00;		//Ô¤ÁôÎ»
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	TxData[8] = 0x00;
	TxData[9] = 0x00;
	TxData[10] = 0x00;
	TxData[11] = 0x00;
	TxData[12] = 0x00;
	TxData[13] = 0x00;
	TxData[14] = 0x00;
	TxData[15] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7]+TxData[8]+TxData[9]
	+TxData[10]+TxData[11]+TxData[12]+TxData[13]+TxData[14];		//check sum
	if (TxData[15]==0x17)
		TxData[15]=0x18;
	else
		TxData[15]=TxData[15];
	TxData[16] = 0x17;		//back code
	TxData[17] = 0x17;		//back code

	//Send message to main dialog
	GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò
}


//Stop Video
void CGraDlg::OnBnClickedBtnStopvideo()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}
	
	GraFlag = sendvideomsg;
	Gra_videoFlag = false;
	memset(TxData,0,sizeof(TxData));
	memset(GraBuf,0,sizeof(GraBuf));
	GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò
	
/*
//---------------------------------------------------------------
// Ô­videoÃüÁî£¬2014-10-10¸ÄÎªÓÃ»­Ò³µÄ·½·¨ÏÔÊ¾video£¨ÈçÉÏÃüÁî£©
//---------------------------------------------------------------
	GraFlag = sendgramsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x03;		//data type, date edit first byte
	TxData[4] = 0x01;		//real data
	TxData[5] = 0x00;		//Ô¤ÁôÎ»
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	TxData[8] = 0x00;
	TxData[9] = 0x00;
	TxData[10] = 0x00;
	TxData[11] = 0x00;
	TxData[12] = 0x00;
	TxData[13] = 0x00;
	TxData[14] = 0x00;
	TxData[15] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7]+TxData[8]+TxData[9]
	+TxData[10]+TxData[11]+TxData[12]+TxData[13]+TxData[14];		//check sum
	if (TxData[15]==0x17)
		TxData[15]=0x18;
	else
		TxData[15]=TxData[15];
	TxData[16] = 0x17;		//back code
	TxData[17] = 0x17;		//back code


	//Send message to main dialog
	GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò
*/
}


void CGraDlg::OnBnClickedBtnClear()
{
	// TODO: Add your control notification handler code here

	m_PixelData = "";
	SetDlgItemText(IDC_EDIT_RecData,m_PixelData);
}


void CGraDlg::OnBnClickedBtnAdcconvert()
{
	// TODO: Add your control notification handler code here
}


void CGraDlg::OnBnClickedBtnClearadc()
{
	// TODO: Add your control notification handler code here
	
	m_ADCRecdata ="";
	SetDlgItemText(IDC_EDIT_ADCDATA,m_ADCRecdata);
}


void CGraDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	CString str;
	// ´´½¨Áí´æ¶Ô»°¿ò
	CFileDialog saveDlg(FALSE,".txt",
		NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"ÎÄ±¾ÎÄ¼þ(*.txt)|*.txt|""ÎÄµµÎÄ¼þ(*.doc)|*.doc|""±í¸ñÎÄ¼þ(*.xls)|*.xls|""All Files(*.*)|*.*||",
		NULL,0,TRUE);

	//	saveDlg.m_ofn.lpstrInitialDir = "c:\\";		// Áí´æ¶Ô»°¿òÄ¬ÈÏÂ·¾¶ÎªcÅÌ

	// ½øÐÐ±£´æ¶¯×÷
	if (saveDlg.DoModal() == IDOK)
	{
		ofstream ofs(saveDlg.GetPathName());
		CStatic * pst = (CStatic*)GetDlgItem(IDC_EDIT_RecData);	// »ñÈ¡Òª±£´æ±à¼­¿ò¿Ø¼þÄÚµÄÊý¾Ý
		// IDC_EDIT_FILEÊÇ±à¼­¿ò¿Ø¼þ¾ä±ú
		pst->GetWindowTextA(str);
		ofs << str;
	}
}


void CGraDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here

	CString str;
	// ´´½¨Áí´æ¶Ô»°¿ò
	CFileDialog saveDlg(FALSE,".txt",
		NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Text(*.txt)|*.txt|""Word doc(*.doc)|*.doc|""All Files(*.*)|*.*||",
		NULL,0,TRUE);

	//	saveDlg.m_ofn.lpstrInitialDir = "c:\\";		// Áí´æ¶Ô»°¿òÄ¬ÈÏÂ·¾¶ÎªcÅÌ

	// ½øÐÐ±£´æ¶¯×÷
	if (saveDlg.DoModal() == IDOK)
	{
		ofstream ofs(saveDlg.GetPathName());
		CStatic * pst = (CStatic*)GetDlgItem(IDC_EDIT_ADCDATA);	// »ñÈ¡Òª±£´æ±à¼­¿ò¿Ø¼þÄÚµÄÊý¾Ý
		// IDC_EDIT_FILEÊÇ±à¼­¿ò¿Ø¼þ¾ä±ú
		pst->GetWindowTextA(str);
		ofs << str;
	}
}



void CGraDlg::OnBnClickedRadioAdcdata()
{
	// TODO: Add your control notification handler code here
}



void CGraDlg::SetGainMode(int gain)
{
	GraFlag = sendgramsg;

	if (!gain)
	{
		TxData[0] = 0xaa;		//preamble code
		TxData[1] = 0x01;		//command
		TxData[2] = 0x02;		//data length
		TxData[3] = 0x07;		//data type, date edit first byte
		TxData[4] = 0x00;	//real data, date edit second byte
		//0x01 means send vedio data
		//0x00 means stop vedio data
		TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
		if (TxData[5]==0x17)
			TxData[5]=0x18;
		else
			TxData[5]=TxData[5];
		TxData[6] = 0x17;		//back code
		TxData[7] = 0x17;		//back code

		//Call maindlg message
//		GraCalMainMsg();
		gain_mode = 0;
	}
	else	
	{	
			TxData[0] = 0xaa;		//preamble code
			TxData[1] = 0x01;		//command
			TxData[2] = 0x02;		//data length
			TxData[3] = 0x07;		//data type, date edit first byte
			TxData[4] = 0x01;	//real data, date edit second byte
			//0x01 means send vedio data
			//0x00 means stop vedio data
			TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
			if (TxData[5]==0x17)
				TxData[5]=0x18;
			else
				TxData[5]=TxData[5];
			TxData[6] = 0x17;		//back code
			TxData[7] = 0x17;		//back code

			//Call maindlg message
//			GraCalMainMsg();
			gain_mode = 1;
	}

	GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò

#ifdef DARK_MANAGE
	
	if(g_pTrimDlg) {
		if(!gain) g_pTrimDlg->SetV20(auto_v20_hg);
		else g_pTrimDlg->SetV20(auto_v20_lg);
	}

#endif

}


void CGraDlg::DisplayHDR(void)
{	
	CDC *pDC;		//´´½¨Ä¿±êDCÖ¸Õë
	pDC=GetDlgItem(IDC_Bmp)->GetDC();	

	CRect   rect;
	CBrush brush[12][12];	
	int i, j;
	int gray_level;
	int pdata[12][12];

	for (i=0; i<12; i++)
	{
		for(j=0; j<12; j++)
		{
			if(pdata_1[i][j] < 400) pdata[i][j] = pdata_0[i][j];
			else pdata[i][j] = (pdata_1[i][j]-DARK_LEVEL)*8 + DARK_LEVEL;
		}
	}

	for (i=0; i<12; i++)
	{
		for(j=0; j<12; j++)
		{
			gray_level = pdata[i][j]/contrast;
			if(gray_level > 255) gray_level = 255;
			else if(gray_level < 0) gray_level = 0;
			brush[i][j].CreateSolidBrush(RGB(gray_level, gray_level, gray_level));		
		}
	}

	for(i=0; i<12; i++) {
			for(j=0; j<12; j++)		// l´ú±íÁÐºÅ
			{
				rect.SetRect(pixelsize12*j,pixelsize12*i,pixelsize12*(j+1),pixelsize12*(i+1));
				pDC->Rectangle(rect);
				pDC->FillRect(&rect,&brush[i][j]);
			}
	}

	CString sRownum;
	char fstrbuf[10];
	CString sADCData;	// ×îºóÏÔÊ¾µÄADCÊý¾Ý×Ö·û´		

	for (i=0; i<12; i++)
	{
		sRownum.Format(" %d",i);
		for(j=0; j<12; j++)
		{
			itoa (pdata[i][j],fstrbuf,10);		//½«½á¹û×ª³É×Ö·û´®ÏÔÊ¾
			sADCData += fstrbuf;
			sADCData += " ";
		}
		sADCData += sRownum;

		if(m_PixOut) {
			m_ADCRecdata += (sADCData+"\r\n");
			SetDlgItemText(IDC_EDIT_ADCDATA,m_ADCRecdata);		//Ã¿ÐÐÊý¾Ý½«¼Ó»Ø³µ
			//×îÐÂÊý¾ÝÔÚ±à¼­¿ò×îºóÒ»ÐÐÏÔÊ¾
			//±à¼­¿ò´¹Ö±¹ö¶¯µ½µ×¶Ë
			POINT pt;
			GetDlgItem(IDC_EDIT_ADCDATA)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
			pt.x=0;
			GetDlgItem(IDC_EDIT_ADCDATA)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
		}
		sADCData = "";
	}
}


void CGraDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default

	if(m_GainMode <= 1) return; // only works for HDR mode.

	int pos;

/*	CSliderCtrl* pSlider = reinterpret_cast<CSliderCtrl*>(pScrollBar);  

    // Check which slider sent the notification  
    if (pSlider == &c_Slider1)  
    {  
    }
    else if (pSlider == &c_Slider2)  
    {  
    }  
*/
    // Check what happened  
    switch(nSBCode)
    {
    case TB_LINEUP:  
    case TB_LINEDOWN:  
    case TB_PAGEUP:  
    case TB_PAGEDOWN:  
    case TB_THUMBPOSITION: 
		pos = nPos;
		contrast = 116 - pos;
		if(!m_FrameSize)
			DisplayHDR();
		else
			DisplayHDR24();
		break;
    case TB_TOP:  
    case TB_BOTTOM:  
    case TB_THUMBTRACK:  
    case TB_ENDTRACK:  
    default:  
        break;  
    }


	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CGraDlg::DisplayHDR24(void)
{
	CDC *pDC;		//´´½¨Ä¿±êDCÖ¸Õë
	pDC=GetDlgItem(IDC_Bmp)->GetDC();	

	CRect   rect;
	CBrush brush[24][24];	
	int i, j;
	int gray_level;
	int pdata[24][24];

	for (i=0; i<24; i++)
	{
		for(j=0; j<24; j++)
		{
			if(pdata_1[i][j] < 400) pdata[i][j] = pdata_0[i][j];
			else pdata[i][j] = (pdata_1[i][j]-DARK_LEVEL)*8 + DARK_LEVEL;
		}
	}

	for (i=0; i<24; i++)
	{
		for(j=0; j<24; j++)
		{
			gray_level = pdata[i][j]/contrast;
			if(gray_level > 255) gray_level = 255;
			else if(gray_level < 0) gray_level = 0;
			brush[i][j].CreateSolidBrush(RGB(gray_level, gray_level, gray_level));		
		}
	}

	for(i=0; i<24; i++) {
			for(j=0; j<24; j++)		// l´ú±íÁÐºÅ
			{
				rect.SetRect(pixelsize24*j,pixelsize24*i,pixelsize24*(j+1),pixelsize24*(i+1));
				pDC->Rectangle(rect);
				pDC->FillRect(&rect,&brush[i][j]);
			}
	}

	CString sRownum;
	char fstrbuf[10];
	CString sADCData;	// ×îºóÏÔÊ¾µÄADCÊý¾Ý×Ö·û´		

	for (i=0; i<24; i++)
	{
		sRownum.Format(" %d",i);
		for(j=0; j<24; j++)
		{
			itoa (pdata[i][j],fstrbuf,10);		//½«½á¹û×ª³É×Ö·û´®ÏÔÊ¾
			sADCData += fstrbuf;
			sADCData += " ";
		}
		sADCData += sRownum;

		if(m_PixOut) {
			m_ADCRecdata += (sADCData+"\r\n");
			SetDlgItemText(IDC_EDIT_ADCDATA,m_ADCRecdata);		//Ã¿ÐÐÊý¾Ý½«¼Ó»Ø³µ
			//×îÐÂÊý¾ÝÔÚ±à¼­¿ò×îºóÒ»ÐÐÏÔÊ¾
			//±à¼­¿ò´¹Ö±¹ö¶¯µ½µ×¶Ë
			POINT pt;
			GetDlgItem(IDC_EDIT_ADCDATA)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
			pt.x=0;
			GetDlgItem(IDC_EDIT_ADCDATA)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
		}
		sADCData = "";
	}
}


void CGraDlg::OnClickedRadiolowgain()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
}


void CGraDlg::OnRadiohighgain()
{
	// TODO: Add your command handler code here
		UpdateData(true);
}


void CGraDlg::OnRadiohdr()
{
	// TODO: Add your command handler code here
		UpdateData(true);
}


void CGraDlg::OnClickedRadio12()
{
	// TODO: Add your control notification handler code here
			UpdateData(true);
}


void CGraDlg::OnRadio24()
{
	// TODO: Add your command handler code here
			UpdateData(true);
}


void CGraDlg::OnBnClickedButtonCapture()
{
	// TODO: Add your control notification handler code here


	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

//	m_PixelData = "";
//	SetDlgItemText(IDC_EDIT_RecData,m_PixelData);

//	OnBnClickedBtnClearadc();

	if(!m_FrameSize) {
		if(m_GainMode <= 1) {
			CaptureFrame12();
		}
		else {
			OnBnClickedBtnDppage12();
		}
	}
	else {
		if(m_GainMode <= 1) {
			CaptureFrame24();
		}
		else {
			OnBnClickedBtnDppage24();
		}
		if(g_pTrimDlg) g_pTrimDlg->ResetTxBin();
	}
}


void CGraDlg::CaptureFrame12(void)
{
	if(!m_GainMode) {	
		SetGainMode(1);
	}
	else {
		SetGainMode(0);
	}

//	hdr_phase = 0;

	GraFlag = sendpagemsg;
	Gra_pageFlag = TRUE;	// ¿ªÊ¼Ñ­»·

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x02;		//data type, date edit first byte
	TxData[4] = 0xff;		//real data
	TxData[5] = 0x00;		//Ô¤ÁôÎ»
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	TxData[8] = 0x00;
	TxData[9] = 0x00;
	TxData[10] = 0x00;
	TxData[11] = 0x00;
	TxData[12] = 0x00;
	TxData[13] = 0x00;
	TxData[14] = 0x00;
	TxData[15] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7]+TxData[8]+TxData[9]
	+TxData[10]+TxData[11]+TxData[12]+TxData[13]+TxData[14];		//check sum
	if (TxData[15]==0x17)
		TxData[15]=0x18;
	else
		TxData[15]=TxData[15];
	TxData[16] = 0x17;		//back code
	TxData[17] = 0x17;		//back code

	//Send message to main dialog
	GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò
}


void CGraDlg::CaptureFrame24(void)
{
	if(!m_GainMode) {	
		SetGainMode(1);
	}
	else {
		SetGainMode(0);
	}

	if(g_pTrimDlg) g_pTrimDlg->ResetTxBin();
	

//	hdr_phase = 0;

	GraFlag = sendpagemsg;
	Gra_pageFlag = TRUE;	// ¿ªÊ¼Ñ­»·

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x08;		//data type, date edit first byte
	TxData[4] = 0xff;		//real data
	TxData[5] = 0x00;		//Ô¤ÁôÎ»
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	TxData[8] = 0x00;
	TxData[9] = 0x00;
	TxData[10] = 0x00;
	TxData[11] = 0x00;
	TxData[12] = 0x00;
	TxData[13] = 0x00;
	TxData[14] = 0x00;
	TxData[15] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7]+TxData[8]+TxData[9]
	+TxData[10]+TxData[11]+TxData[12]+TxData[13]+TxData[14];		//check sum
	if (TxData[15]==0x17)
		TxData[15]=0x18;
	else
		TxData[15]=TxData[15];
	TxData[16] = 0x17;		//back code
	TxData[17] = 0x17;		//back code

	//Send message to main dialog
	GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò
}


void CGraDlg::OnClickedCheckPixout()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
}


void CGraDlg::OnClickedCheckOrigout()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
}


void CGraDlg::OnBnClickedBtnReadspi()
{
	// TODO: Add your control notification handler code here
	int_time += 4;
	if(g_pTrimDlg) g_pTrimDlg->SetIntTime(int_time);
}
