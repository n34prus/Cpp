#include <math.h>

//����� �� ���, ����� ����������� ����������� ����������� � �������� �� -127 �� +126 (������� �������� ��� �� � �����)
//������ B57861 10k 1% (B25/100)K=3988, R/T = 8016 (����)
signed int list_of_ADC_3988 [40][2] =	//[i][0] - �������� ���, [i][1] - �������������� ��� �����������
{
	{1008,-50},
	{1002,-45},
	{993,-40}, 
	{983,-35}, 
	{968,-30}, 	
	{950,-25}, 
	{928,-20},
	{900,-15},
	{867,-10},
	{831,-5},  		//-50*� .. -5*�
	{784,0},
	{735,5},
	{682,10},
	{627,15},
	{570,20},
	{514,25},
	{460,30},
	{408,35},
	{360,40},
	{316,45},		//0*C .. 45*C
	{276,50},
	{241,55},
	{210,60},
	{183,65},
	{160,70},
	{140,75},
	{122,80},
	{107,85},
	{94,90},
	{83,95},		//50*C .. 95*C
	{74,100},
	{66,105},
	{59,110},
	{53,115},
	{48,120},
	{43,125},
	{39,130},
	{36,135},
	{33,140},
	{31,145}		//100*c .. 145*c
};
//������ B57164K 10k 5% (B25/100)K=4300, R/T = 2904 (���)
signed int list_of_ADC_4300 [40][2] =
{
	{1011,-50},
	{1006,-45},
	{999,-40},
	{990,-35},
	{978,-30},
	{962,-25},
	{941,-20},
	{915,-25},
	{883,-10},
	{844,-5},		//-50*� .. -5*�
	{799,0},
	{748,5},
	{693,10},
	{634,15},
	{547,20},
	{514,25},
	{456,30},
	{400,35},
	{349,40},
	{303,45},		//0*C .. 45*C
	{262,50},
	{225,55},
	{194,60},
	{167,65},
	{143,70},
	{124,75},
	{107,80},
	{93,85},
	{81,90},
	{71,95},			//50*C .. 95*C
	{62,100},
	{55,105},
	{49,110},
	{44,115},
	{39,120},
	{36,125},
	{32,130},
	{30,135},
	{27,140},
	{25,145}
};	//100*C .. 145*C
//������ carel NTC015WH01 10k 1% B=3435
signed int list_of_ADC_3435 [161][2] =
{
	{	993	,	-50	}	,
	{	991	,	-49	}	,
	{	990	,	-48	}	,
	{	988	,	-47	}	,
	{	986	,	-46	}	,
	{	984	,	-45	}	,
	{	982	,	-44	}	,
	{	979	,	-43	}	,
	{	977	,	-42	}	,
	{	975	,	-41	}	,
	{	972	,	-40	}	,
	{	969	,	-39	}	,
	{	966	,	-38	}	,
	{	963	,	-37	}	,
	{	960	,	-36	}	,
	{	957	,	-35	}	,
	{	954	,	-34	}	,
	{	951	,	-33	}	,
	{	947	,	-32	}	,
	{	943	,	-31	}	,
	{	940	,	-30	}	,
	{	935	,	-29	}	,
	{	931	,	-28	}	,
	{	927	,	-27	}	,
	{	923	,	-26	}	,
	{	918	,	-25	}	,
	{	913	,	-24	}	,
	{	908	,	-23	}	,
	{	903	,	-22	}	,
	{	898	,	-21	}	,
	{	893	,	-20	}	,
	{	887	,	-19	}	,
	{	881	,	-18	}	,
	{	876	,	-17	}	,
	{	869	,	-16	}	,
	{	863	,	-15	}	,
	{	857	,	-14	}	,
	{	850	,	-13	}	,
	{	844	,	-12	}	,
	{	837	,	-11	}	,
	{	830	,	-10	}	,
	{	823	,	-9	}	,
	{	815	,	-8	}	,
	{	808	,	-7	}	,
	{	800	,	-6	}	,
	{	792	,	-5	}	,
	{	784	,	-4	}	,
	{	776	,	-3	}	,
	{	768	,	-2	}	,
	{	760	,	-1	}	,
	{	751	,	0	}	,
	{	743	,	1	}	,
	{	734	,	2	}	,
	{	725	,	3	}	,
	{	716	,	4	}	,
	{	707	,	5	}	,
	{	698	,	6	}	,
	{	689	,	7	}	,
	{	679	,	8	}	,
	{	670	,	9	}	,
	{	661	,	10	}	,
	{	651	,	11	}	,
	{	642	,	12	}	,
	{	632	,	13	}	,
	{	622	,	14	}	,
	{	613	,	15	}	,
	{	603	,	16	}	,
	{	593	,	17	}	,
	{	584	,	18	}	,
	{	574	,	19	}	,
	{	565	,	20	}	,
	{	555	,	21	}	,
	{	545	,	22	}	,
	{	536	,	23	}	,
	{	526	,	24	}	,
	{	517	,	25	}	,
	{	507	,	26	}	,
	{	498	,	27	}	,
	{	488	,	28	}	,
	{	479	,	29	}	,
	{	470	,	30	}	,
	{	461	,	31	}	,
	{	452	,	32	}	,
	{	443	,	33	}	,
	{	434	,	34	}	,
	{	425	,	35	}	,
	{	417	,	36	}	,
	{	408	,	37	}	,
	{	400	,	38	}	,
	{	391	,	39	}	,
	{	383	,	40	}	,
	{	375	,	41	}	,
	{	367	,	42	}	,
	{	359	,	43	}	,
	{	351	,	44	}	,
	{	344	,	45	}	,
	{	336	,	46	}	,
	{	329	,	47	}	,
	{	322	,	48	}	,
	{	315	,	49	}	,
	{	308	,	50	}	,
	{	301	,	51	}	,
	{	294	,	52	}	,
	{	288	,	53	}	,
	{	281	,	54	}	,
	{	275	,	55	}	,
	{	269	,	56	}	,
	{	263	,	57	}	,
	{	257	,	58	}	,
	{	251	,	59	}	,
	{	245	,	60	}	,
	{	240	,	61	}	,
	{	234	,	62	}	,
	{	229	,	63	}	,
	{	224	,	64	}	,
	{	219	,	65	}	,
	{	213	,	66	}	,
	{	209	,	67	}	,
	{	204	,	68	}	,
	{	200	,	69	}	,
	{	195	,	70	}	,
	{	190	,	71	}	,
	{	186	,	72	}	,
	{	182	,	73	}	,
	{	178	,	74	}	,
	{	173	,	75	}	,
	{	170	,	76	}	,
	{	166	,	77	}	,
	{	163	,	78	}	,
	{	159	,	79	}	,
	{	155	,	80	}	,
	{	151	,	81	}	,
	{	148	,	82	}	,
	{	145	,	83	}	,
	{	142	,	84	}	,
	{	138	,	85	}	,
	{	135	,	86	}	,
	{	132	,	87	}	,
	{	130	,	88	}	,
	{	127	,	89	}	,
	{	124	,	90	}	,
	{	121	,	91	}	,
	{	119	,	92	}	,
	{	116	,	93	}	,
	{	114	,	94	}	,
	{	111	,	95	}	,
	{	109	,	96	}	,
	{	106	,	97	}	,
	{	104	,	98	}	,
	{	102	,	99	}	,
	{	100	,	100	}	,
	{	98	,	101	}	,
	{	96	,	102	}	,
	{	94	,	103	}	,
	{	92	,	104	}	,
	{	90	,	105	}	,
	{	89	,	106	}	,
	{	87	,	107	}	,
	{	85	,	108	}	,
	{	84	,	109	}	,
	{	82	,	110	}
};
//������ ESMU PT-1000
signed int list_of_ADC_1000 [34][2] =
{
	{	512	,	-50	}	,
	{	518	,	-45	}	,
	{	523	,	-40	}	,
	{	529	,	-35	}	,
	{	534	,	-30	}	,
	{	539	,	-25	}	,
	{	544	,	-20	}	,
	{	549	,	-15	}	,
	{	553	,	-10	}	,
	{	558	,	-5	}	,
	{	563	,	0	}	,
	{	567	,	5	}	,
	{	571	,	10	}	,
	{	576	,	15	}	,
	{	580	,	20	}	,
	{	584	,	25	}	,
	{	588	,	30	}	,
	{	592	,	35	}	,
	{	596	,	40	}	,
	{	600	,	45	}	,
	{	603	,	50	}	,
	{	607	,	55	}	,
	{	611	,	60	}	,
	{	614	,	65	}	,
	{	618	,	70	}	,
	{	621	,	75	}	,
	{	624	,	80	}	,
	{	628	,	85	}	,
	{	631	,	90	}	,
	{	634	,	95	}	,
	{	637	,	100	}	,
	{	640	,	105	}	,
	{	643	,	110	}	,
	{	646	,	115	}
};

float getTempFromADC (int bk, unsigned int dataADC)					//������ ���������, � bk �������� ������������� ����������, ������� �������� �� �������, � dataADC - ��������� ���
{
	signed int (*list_of_ADC)[2];	//��������� �� ��������� ������ (��������, ��� ��������� �� ��������� �� ��� �����)
	unsigned char length = 0;	//��� �����, ������ �������
	if (bk == 3988)
	{
		list_of_ADC = list_of_ADC_3988;
		length = sizeof(list_of_ADC_3988) / sizeof(list_of_ADC_3988[0]) -1;	//��������� ������ ������ �������, -1 ������ ��� �� ���� �������
	}
	else if (bk == 4300)
	{
		list_of_ADC = list_of_ADC_4300;
		length = sizeof(list_of_ADC_4300) / sizeof(list_of_ADC_4300[0]) -1;	//��������� ������ ������ �������, -1 ������ ��� �� ���� �������
	}
	else if (bk == 3435)
	{
		list_of_ADC = list_of_ADC_3435;
		length = sizeof(list_of_ADC_3435) / sizeof(list_of_ADC_3435[0]) -1;	//��������� ������ ������ �������, -1 ������ ��� �� ���� �������
	}
	else if (bk == 1000)
	{
		list_of_ADC = list_of_ADC_1000;
		length = sizeof(list_of_ADC_1000) / sizeof(list_of_ADC_3435[0]) -1;	//��������� ������ ������ �������, -1 ������ ��� �� ���� �������
	}

	
	if (dataADC > list_of_ADC[0][0]) return -128;		//����� �������
	else if (dataADC < list_of_ADC[length][0]) return 127;	//�� �������
	else for (int i = 1; i <= length; i++)				//������ �����������. 35� ������� ������� - 125*�, ���� �� ���������� � char
	{
		//cout<<"length "<<(int)length<<" dataADC "<<(int)dataADC<<" list_of_ADC[i] "<<(int)list_of_ADC[i]<<" i "<<(int)i<<endl;
		if (dataADC == list_of_ADC[i][0]) return (list_of_ADC[i][1]);
		else if (dataADC > list_of_ADC[i][0])		//������� �� �������, ���� ��������� ������ �������� ���, ��...
		{
			float shift = (float)(list_of_ADC[i-1][0] - dataADC)/(float)(list_of_ADC[i-1][0] - list_of_ADC[i][0]);	//��������� ���������� �������� ������ � ����� ������� ������ � ����� �����������
			//cout<<"shift "<<shift<<endl;
			shift*= list_of_ADC[i][1] - list_of_ADC[i-1][1];    //�������� �� ������������� ���
			//cout<<"shift "<<shift<<endl;
			return (list_of_ADC[i-1][1] + shift);		//� ���������� ����������� (������� ����� ��������� �������� ��� + ��������)
		}
	}
	return 0;
}

signed char getRoundedTempFromADC (int bk, unsigned int dataADC)	//��� ������� ���������, ��������� ���������� � ���� ����
{
	float buffer = getTempFromADC(bk, dataADC);
	if (buffer >= 127) return 127;
	else if (buffer <= -128) return -128;
	else return (signed char)round(buffer);
}

//���� ��� ���� ������ � ��������� �� ������� ����� ������� � ���������� ����-������ � ���, �� ����� ������������ � ��� �����
//1� ���� - ������� ����� ����� NaturalTemp
//2� ���� - ������� ������� �����(������� ����) TenthTemp
//�������� ��� ����� ��������. ��� �������� ��� ������ ��� Temp = NaturalTemp+(TenthTemp/10)

signed char getNaturalTempFromADC (int bk, unsigned int dataADC)	//��� ����������� ���������, ���������� ����� ����� �� ������ �����������, ��������� ���������� � ���� ���� (-127..+128)
{
	float buffer = getTempFromADC(bk, dataADC);
	if (buffer >= 127) return 127;
	else if (buffer <= -128) return -128;
	else return (signed char)buffer;
}

signed char getTenthTempFromADC (int bk, unsigned int dataADC)		//��� ����������� ���������, ���������� ������� ���� �� ������ ���������. ��������, ��� ����������� 37,81235 ������ 8
{
	signed int buffer = (signed int)(getTempFromADC(bk, dataADC)*10)%10;
	return (signed char)fabs(buffer);
}