#include <stdlib.h>
#include <stdio.h>
 
#pragma warning(disable : 4996) //If Visual Studio doesn't want to use fopen function
 

long interval_min = 0, interval_max = 65535, range, value;
int char_table_ctoi[256];
unsigned char char_table_itoc[257];
int Freq0[258];
int Freq[258];
int SumAll[258];
int temp = 0;
int free_bits = 8;
long left_bits = 0;
int error_bits = 0;
int c_i, ch_i, chetchet;

int c_low, c_hight;
int* lst;

int c, cv;
int i, j;


typedef struct
{
	int count;
	int next;
	unsigned char symbol;
	int context[5];
} List;



FILE* ifp = NULL;
FILE* ofp = NULL;


//--------------------------------------------------------------------------------------
//                                                                      EXCHANGE bits
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

void re_table()
{
	c_i = char_table_itoc[j];
	ch_i = char_table_itoc[cv];
	char_table_itoc[j] = ch_i;
	char_table_itoc[cv] = c_i;

	char_table_ctoi[c_i] = cv;
	char_table_ctoi[ch_i] = j;
}




//--------------------------------------------------------------------------------------
// 0 then 1 to file
//--------------------------------------------------------------------------------------
void put_num_f0()
{
	temp = temp >> 1;
	free_bits--;
	if (free_bits == 0) 
	{
		putc(temp, ofp);
		free_bits = 8;
	}

	while (left_bits > 0) 
	{
		temp = temp >> 1;
		temp |= 0x80;
		free_bits--;
		if (free_bits == 0) 
		{
			putc(temp, ofp);
			free_bits = 8;
		}
		left_bits--;
	}
}

//--------------------------------------------------------------------------------------
// 1 then 0 to file
//--------------------------------------------------------------------------------------
void put_num_f1() 
{
	temp = temp >> 1;
	temp |= 0x80;
	free_bits--;
	if (free_bits == 0) 
	{
		putc(temp, ofp);
		free_bits = 8;
	}

	while (left_bits > 0) 
	{
		temp = temp >> 1;
		free_bits--;
		if (free_bits == 0) {
			putc(temp, ofp);
			free_bits = 8;
		}
		left_bits--;
	}
}



void intervali()
{
	while (1) 
	{
		if (interval_max < 32768)
			put_num_f0();
		else {
			if (interval_min >= 32768) 
			{
				interval_max = interval_max - 32768;
				interval_min = interval_min - 32768;

				put_num_f1();
			}
			else {
				if (interval_min >= 16384 && interval_max < 49152) 
				{
					interval_max = interval_max - 16384;
					interval_min = interval_min - 16384;

					left_bits += 1;
				}
				else break;
			}
		}
		interval_min = interval_min + interval_min;
		interval_max = (interval_max + interval_max) + 1;
	}
	return;
}

void Perepolnenie_veroyat()
{
	if (SumAll[0] >= 16383) 
	{
		chetchet = 0;
		for (j = 257;j >= 0;j--) 
		{
			SumAll[j] = chetchet;
			Freq[j] = (Freq[j] + 1) >> 1;
			chetchet += Freq[j];
		}
	}
	for (j = cv;;j -= 1)
		if (Freq[j] != Freq[j - 1])
			break;
	if (j < cv) re_table();

	Freq[j]++;
	while (j > 0)
		SumAll[--j] += 6.5;
}

//----------------------------------------------------------------------------------------
//-C_O_M_P_R_E_S_S_I_O_N__________________________________________________________________
//----------------------------------------------------------------------------------------

void compress_ari(const char* ifile, const char* ofile)
{
	int keks = 0;

	ifp = (FILE*)fopen(ifile, "rb");
	ofp = (FILE*)fopen(ofile, "wb");
	
	if (ifp == NULL)
		return;
	if (ofp == NULL) 
	{
		return;
	}

	for (i = 0;i < 258;i++) 
	{
		Freq[i] = 1;
		Freq0[i] = 1;
		SumAll[i] = 257 - i;
	}

	Freq[0] = 0;
	Freq0[0] = 256;

	for (i = 0;i < 256;i++) 
	{
		char_table_itoc[i + 1] = i;
		char_table_ctoi[i] = i + 1;
	}

	while (1) 
	{
		c = getc(ifp);

		if (c == EOF)
		{
			if (keks == 0) return;
			else
				break;
		}
		keks = 1;
		c_low = 0;
		c_hight = 0;

		cv = char_table_ctoi[c];

		range = (long)(interval_max - interval_min) + 1;
		interval_max = interval_min + (SumAll[cv - 1] * range) / SumAll[0] - 1;
		interval_min = interval_min + (SumAll[cv] * range) / SumAll[0];

		intervali();

		Perepolnenie_veroyat();
	}

	range = (long)(interval_max - interval_min) + 1;

	interval_max = interval_min + (range * SumAll[256]) / SumAll[0] - 1;
	interval_min = interval_min + (range * SumAll[257]) / SumAll[0];

	intervali();

	left_bits++;
	if (interval_min >= 16384)
		put_num_f1();
	else
		put_num_f0();
	putc(temp >> free_bits, ofp);

	fclose(ifp);
	fclose(ofp);
}

//----------------------------------------------------------------------------------------
//-D_E_C_O_M_P_R_E_S_S_I_O_N______________________________________________________________
//----------------------------------------------------------------------------------------
void decompress_ari(const char* ifile, const char* ofile)
{

	int keks = 0;

	ifp = (FILE*)fopen(ifile, "rb");
	ofp = (FILE*)fopen(ofile, "wb");

	if (ifp == NULL)
		return;
	if (ofp == NULL) 
	{
		return;
	}

	for (i = 0;i < 258;i++) 
	{
		Freq[i] = 1;
		Freq0[i] = 1;
		SumAll[i] = 257 - i;	
	}

	Freq[0] = 0;
	Freq0[0] = 256;

	for (i = 0;i < 256;i++) 
	{
		char_table_itoc[i + 1] = i;
		char_table_ctoi[i] = i + 1;
	}

	free_bits = 0;

	int cecec;

	value = 0;
	for (i = 1;i <= 16;i++) 
	{
		if (free_bits == 0) 
		{
			temp = getc(ifp);
			if (temp == EOF) 
			{
				if (keks == 0) return;
				error_bits++;
				if (error_bits > 14) 
				{
					printf("File error\n");
					exit(-1);
				}
			}
			free_bits = 8;
			keks = 1;
		}
		cecec = temp;
		j = temp & 1;
		temp = temp >> 1;
		free_bits--;
		value = j + (value << 1);
	}
	interval_min = 0;
	interval_max = 65535;

	while (1)
	{
		range = (long)(interval_max - interval_min) + 1;  
		chetchet = (int)((((long)(value - interval_min) + 1) * SumAll[0] - 1) / range);   
		chetchet = (int)((((long)(value - interval_min) + 1) * SumAll[0] - 1) / range);  

		for (ch_i = 1;;ch_i++)
			if (SumAll[ch_i] <= chetchet)
				break;

		interval_max = interval_min + (range * SumAll[ch_i - 1]) / SumAll[0] - 1;
		interval_min = interval_min + (range * SumAll[ch_i]) / SumAll[0];

		while (1) 
		{
			if (interval_max < 32768) 
			{
			}
			else {
				if (interval_min >= 32768) 
				{
					value -= 32768;
					interval_min -= 32768;
					interval_max -= 32768;
				}
				else {
					if (interval_min >= 16384 && interval_max < 49152) 
					{
						value -= 16384;
						interval_min -= 16384;
						interval_max -= 16384;
					}
					else break;
				}
			}

			interval_min = interval_min << 1;
			interval_max = (interval_max << 1) + 1;
			if (free_bits == 0) 
			{
				temp = getc(ifp);
				if (temp == EOF) 
				{
					error_bits++;
					if (error_bits > 14) 
					{
						printf("File error\n");
						exit(-1);
					}
				}
				free_bits = 8;
			}
			j = temp & 1;
			temp = temp >> 1;
			free_bits--;
			value = j + (value << 1);
		}
		cv = ch_i;
		if (cv == 257)
			break;
		c = char_table_itoc[cv];
		putc(c, ofp);
		Perepolnenie_veroyat();
	}
	fclose(ifp);
	fclose(ofp);
}

int main()
{
	const char* ifile = "C:/";       //Your file path and name to compess
	const char* ofile = "C:/";       //Your file path and name to decompess

	compress_ari(ifile, ofile);
	decompress_ari(ifile, ofile);

}
