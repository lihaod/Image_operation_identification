#include <iostream>
#include <math.h>
#include <cmath>
#include <vector>
#include <mex.h>
#include <cstring>

#ifndef CONFIG_H_
#define CONFIG_H_

class Config
{
public:
	bool verbose;
	int T;
	int order;
	int *SPAMsymmCoord;
	int SPAMsymmDim;
	int *MINMAXsymmCoord;
	int MINMAXsymmDim;
	int numInMap;
	bool mergeSpams;
	bool eraseLSB;
	bool parity;
	bool roundup5;          // if roundup5==true, round(0.5)=1, else round(0.5)=0. In conventional SRM roundup5=false.
	bool mapMode;

	Config(bool verbose, int T, int order, bool symmSign, bool symmReverse, bool symmMinMax, bool mergeSpams, bool eraseLSB, bool parity, bool roundup5, bool mapMode = true, int numInMap = 1)
	{
		this->verbose = verbose;
		this->T = T;
		this->order = order;
		this->mergeSpams = mergeSpams;
		this->eraseLSB = eraseLSB;
		this->parity = parity;
		this->roundup5 = roundup5;
		this->mapMode = mapMode;
		this->numInMap = numInMap;
		
		GetSymmCoords(symmSign, symmReverse, symmMinMax);
	}

	~Config()
	{
		delete [] SPAMsymmCoord;
		delete [] MINMAXsymmCoord;
	}

private:

	void GetSymmCoords(bool symmSign, bool symmReverse, bool symmMinMax)
	{
		// Preparation of inSymCoord matrix for co-occurrence and symmetrization
		int B = 2*this->T+1;
		int fullDim = (int)std::pow((float)B, this->order);
	
		int alreadyUsed;
	
		// MINMAX
		alreadyUsed = 0;
		MINMAXsymmCoord = new int[2*fullDim]; // [0, fullDim-1] = min; [fullDim, 2*fullDim-1] = max
		for (int i=0; i<2*fullDim; i++) MINMAXsymmCoord[i] = -1;

		for (int numIter=0; numIter < fullDim; numIter++)
		{
			if (MINMAXsymmCoord[numIter] == -1)
			{
				int coordReverse = 0;
				int num = numIter;
				for (int i=0; i<this->order; i++)
				{
					coordReverse += (num % B) * ((int)std::pow((float)B, order-i-1));
					num = num / B;
				}
				// To the same bin: min(X), max(-X), min(Xreverse), max(-Xreverse)
				if (MINMAXsymmCoord[numIter] == -1)
				{
					MINMAXsymmCoord[numIter] = alreadyUsed; // min(X)
					if (symmMinMax) MINMAXsymmCoord[2*fullDim-numIter-1] = alreadyUsed; // max(-X)
					if (symmReverse) MINMAXsymmCoord[coordReverse] = alreadyUsed; // min(Xreverse)
					if ((symmMinMax) && (symmReverse)) MINMAXsymmCoord[2*fullDim-coordReverse-1] = alreadyUsed; // max(-Xreverse)
					alreadyUsed++;
				}
			}
		}
		for (int numIter=0; numIter < fullDim; numIter++)
		{
			if (MINMAXsymmCoord[fullDim+numIter] == -1)
			{
				int coordReverse = 0;
				int num = numIter;
				for (int i=0; i<this->order; i++)
				{
					coordReverse += (num % B) * ((int)std::pow((float)B, order-i-1));
					num = num / B;
				}
				// To the same bin: max(X), min(-X), max(Xreverse), min(-Xreverse)
				if (MINMAXsymmCoord[fullDim+numIter] == -1)
				{
					MINMAXsymmCoord[fullDim+numIter] = alreadyUsed; // max(X)
					if (symmMinMax) MINMAXsymmCoord[fullDim-numIter-1] = alreadyUsed; // min(-X)
					if (symmReverse) MINMAXsymmCoord[fullDim+coordReverse] = alreadyUsed; // max(Xreverse)
					if ((symmMinMax) && (symmReverse)) MINMAXsymmCoord[fullDim-coordReverse-1] = alreadyUsed; // min(-Xreverse)
					alreadyUsed++;
				}
			}
		}
		MINMAXsymmDim = alreadyUsed;
		
		// SPAM
		alreadyUsed = 0;
		SPAMsymmCoord = new int[fullDim];
		for (int i=0; i<fullDim; i++) SPAMsymmCoord[i] = -1;
		for (int numIter=0; numIter < fullDim; numIter++)
		{
			if (SPAMsymmCoord[numIter] == -1)
			{
				int coordReverse = 0;
				int num = numIter;
				for (int i=0; i<this->order; i++)
				{
					coordReverse += (num % B) * ((int)std::pow((float)B, order-i-1));
					num = num / B;
				}
				// To the same bin: X, -X, Xreverse, -Xreverse
				SPAMsymmCoord[numIter] = alreadyUsed; // X
				if (symmSign) SPAMsymmCoord[fullDim-numIter-1] = alreadyUsed; // -X
				if (symmReverse) SPAMsymmCoord[coordReverse] = alreadyUsed; // Xreverse
				if ((symmSign) && (symmReverse)) SPAMsymmCoord[fullDim-coordReverse-1] = alreadyUsed; // -Xreverse
				alreadyUsed++;
			}
		}
		SPAMsymmDim = alreadyUsed;
		// In order to have the same order of the features as the matlab SRM - shift +1
		for (int i=0; i<fullDim; i++) 
		{
			if (SPAMsymmCoord[i]==alreadyUsed-1) SPAMsymmCoord[i]=0;
			else SPAMsymmCoord[i]++;
		}
	}
};

class QuantStep
{
public:
	std::vector<float> s1_minmax22h;
	std::vector<float> s1_minmax22v;
	std::vector<float> s1_minmax24;
	std::vector<float> s1_minmax34;
	std::vector<float> s1_minmax34h;
	std::vector<float> s1_minmax34v;
	std::vector<float> s1_minmax41;
	std::vector<float> s1_minmax48h;
	std::vector<float> s1_minmax48v;
	std::vector<float> s1_minmax54;
	std::vector<float> s1_spam14h;
	std::vector<float> s1_spam14v;
	std::vector<float> s2_minmax21;
	std::vector<float> s2_minmax24h;
	std::vector<float> s2_minmax24v;
	std::vector<float> s2_minmax32;
	std::vector<float> s2_minmax41;
	std::vector<float> s2_spam12h;
	std::vector<float> s2_spam12v;
	std::vector<float> s3_minmax22h;
	std::vector<float> s3_minmax22v;
	std::vector<float> s3_minmax24;
	std::vector<float> s3_minmax34;
	std::vector<float> s3_minmax34h;
	std::vector<float> s3_minmax34v;
	std::vector<float> s3_minmax41;
	std::vector<float> s3_minmax48h;
	std::vector<float> s3_minmax48v;
	std::vector<float> s3_minmax54;
	std::vector<float> s3_spam14h;
	std::vector<float> s3_spam14v;
	std::vector<float> s3x3_minmax22h;
	std::vector<float> s3x3_minmax22v;
	std::vector<float> s3x3_minmax24;
	std::vector<float> s3x3_minmax41;
	std::vector<float> s3x3_spam11;
	std::vector<float> s3x3_spam14h;
	std::vector<float> s3x3_spam14v;
	std::vector<float> s5x5_minmax22h;
	std::vector<float> s5x5_minmax22v;
	std::vector<float> s5x5_minmax24;
	std::vector<float> s5x5_minmax41;
	std::vector<float> s5x5_spam11;
	std::vector<float> s5x5_spam14h;
	std::vector<float> s5x5_spam14v;

	void setQuanStepForALL(const mxArray *fieldContent)
	{
		setQuanStepForS1(fieldContent);
		setQuanStepForS2(fieldContent);
		setQuanStepForS3(fieldContent);
		setQuanStepForS3x3(fieldContent);
		setQuanStepForS5x5(fieldContent);
	}

	void setQuanStepForS1(const mxArray *fieldContent)
	{
		setQuanStepForS("s1_minmax22h", fieldContent);
		setQuanStepForS("s1_minmax22v", fieldContent);
		setQuanStepForS("s1_minmax24", fieldContent);
		setQuanStepForS("s1_minmax34", fieldContent);
		setQuanStepForS("s1_minmax34h", fieldContent);
		setQuanStepForS("s1_minmax34v", fieldContent);
		setQuanStepForS("s1_minmax41", fieldContent);
		setQuanStepForS("s1_minmax48h", fieldContent);
		setQuanStepForS("s1_minmax48v", fieldContent);
		setQuanStepForS("s1_minmax54", fieldContent);
		setQuanStepForS("s1_spam14h", fieldContent);
		setQuanStepForS("s1_spam14v", fieldContent);
	}

	void setQuanStepForS2(const mxArray *fieldContent)
	{
		setQuanStepForS("s2_minmax21", fieldContent);
		setQuanStepForS("s2_minmax24h", fieldContent);
		setQuanStepForS("s2_minmax24v", fieldContent);
		setQuanStepForS("s2_minmax32", fieldContent);
		setQuanStepForS("s2_minmax41", fieldContent);
		setQuanStepForS("s2_spam12h", fieldContent);
		setQuanStepForS("s2_spam12v", fieldContent);
	}

	void setQuanStepForS3(const mxArray *fieldContent)
	{
		setQuanStepForS("s3_minmax22h", fieldContent);
		setQuanStepForS("s3_minmax22v", fieldContent);
		setQuanStepForS("s3_minmax24", fieldContent);
		setQuanStepForS("s3_minmax34", fieldContent);
		setQuanStepForS("s3_minmax34h", fieldContent);
		setQuanStepForS("s3_minmax34v", fieldContent);
		setQuanStepForS("s3_minmax41", fieldContent);
		setQuanStepForS("s3_minmax48h", fieldContent);
		setQuanStepForS("s3_minmax48v", fieldContent);
		setQuanStepForS("s3_minmax54", fieldContent);
		setQuanStepForS("s3_spam14h", fieldContent);
		setQuanStepForS("s3_spam14v", fieldContent);
	}

	void setQuanStepForS3x3(const mxArray *fieldContent)
	{
		setQuanStepForS("s3x3_minmax22h", fieldContent);
		setQuanStepForS("s3x3_minmax22v", fieldContent);
		setQuanStepForS("s3x3_minmax24", fieldContent);
		setQuanStepForS("s3x3_minmax41", fieldContent);
		setQuanStepForS("s3x3_spam11", fieldContent);
		setQuanStepForS("s3x3_spam14h", fieldContent);
		setQuanStepForS("s3x3_spam14v", fieldContent);
	}

	void setQuanStepForS5x5(const mxArray *fieldContent)
	{
		setQuanStepForS("s5x5_minmax22h", fieldContent);
		setQuanStepForS("s5x5_minmax22v", fieldContent);
		setQuanStepForS("s5x5_minmax24", fieldContent);
		setQuanStepForS("s5x5_minmax41", fieldContent);
		setQuanStepForS("s5x5_spam11", fieldContent);
		setQuanStepForS("s5x5_spam14h", fieldContent);
		setQuanStepForS("s5x5_spam14v", fieldContent);
	}

	void setQuanStepForS(const char *fieldName, const mxArray *fieldContent)
	{
		if (strcmp(fieldName, "s1_minmax22h") == 0)
			if (mxIsDouble(fieldContent))
			{
				s1_minmax22h.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s1_minmax22h.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s1_minmax22v") == 0)
			if (mxIsDouble(fieldContent))
			{
				s1_minmax22v.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s1_minmax22v.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s1_minmax24") == 0)
			if (mxIsDouble(fieldContent))
			{
				s1_minmax24.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s1_minmax24.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s1_minmax34") == 0)
			if (mxIsDouble(fieldContent))
			{
				s1_minmax34.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s1_minmax34.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s1_minmax34h") == 0)
			if (mxIsDouble(fieldContent))
			{
				s1_minmax34h.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s1_minmax34h.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s1_minmax34v") == 0)
			if (mxIsDouble(fieldContent))
			{
				s1_minmax34v.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s1_minmax34v.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s1_minmax41") == 0)
			if (mxIsDouble(fieldContent))
			{
				s1_minmax41.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s1_minmax41.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s1_minmax48h") == 0)
			if (mxIsDouble(fieldContent))
			{
				s1_minmax48h.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s1_minmax48h.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s1_minmax48v") == 0)
			if (mxIsDouble(fieldContent))
			{
				s1_minmax48v.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s1_minmax48v.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s1_minmax54") == 0)
			if (mxIsDouble(fieldContent))
			{
				s1_minmax54.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s1_minmax54.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s1_spam14h") == 0)
			if (mxIsDouble(fieldContent))
			{
				s1_spam14h.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s1_spam14h.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s1_spam14v") == 0)
			if (mxIsDouble(fieldContent))
			{
				s1_spam14v.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s1_spam14v.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s2_minmax21") == 0)
			if (mxIsDouble(fieldContent))
			{
				s2_minmax21.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s2_minmax21.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s2_minmax24h") == 0)
			if (mxIsDouble(fieldContent))
			{
				s2_minmax24h.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s2_minmax24h.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s2_minmax24v") == 0)
			if (mxIsDouble(fieldContent))
			{
				s2_minmax24v.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s2_minmax24v.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s2_minmax32") == 0)
			if (mxIsDouble(fieldContent))
			{
				s2_minmax32.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s2_minmax32.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s2_minmax41") == 0)
			if (mxIsDouble(fieldContent))
			{
				s2_minmax41.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s2_minmax41.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s2_spam12h") == 0)
			if (mxIsDouble(fieldContent))
			{
				s2_spam12h.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s2_spam12h.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s2_spam12v") == 0)
			if (mxIsDouble(fieldContent))
			{
				s2_spam12v.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s2_spam12v.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s3_minmax22h") == 0)
			if (mxIsDouble(fieldContent))
			{
				s3_minmax22h.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s3_minmax22h.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s3_minmax22v") == 0)
			if (mxIsDouble(fieldContent))
			{
				s3_minmax22v.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s3_minmax22v.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s3_minmax24") == 0)
			if (mxIsDouble(fieldContent))
			{
				s3_minmax24.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s3_minmax24.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s3_minmax34") == 0)
			if (mxIsDouble(fieldContent))
			{
				s3_minmax34.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s3_minmax34.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s3_minmax34h") == 0)
			if (mxIsDouble(fieldContent))
			{
				s3_minmax34h.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s3_minmax34h.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s3_minmax34v") == 0)
			if (mxIsDouble(fieldContent)) 
			{
				s3_minmax34v.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s3_minmax34v.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s3_minmax41") == 0)
			if (mxIsDouble(fieldContent))
			{
				s3_minmax41.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s3_minmax41.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s3_minmax48h") == 0)
			if (mxIsDouble(fieldContent))
			{
				s3_minmax48h.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s3_minmax48h.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s3_minmax48v") == 0)
			if (mxIsDouble(fieldContent))
			{
				s3_minmax48v.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s3_minmax48v.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s3_minmax54") == 0)
			if (mxIsDouble(fieldContent))
			{
				s3_minmax54.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s3_minmax54.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s3_spam14h") == 0)
			if (mxIsDouble(fieldContent))
			{
				s3_spam14h.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s3_spam14h.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s3_spam14v") == 0)
			if (mxIsDouble(fieldContent))
			{
				s3_spam14v.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s3_spam14v.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s3x3_minmax22h") == 0)
			if (mxIsDouble(fieldContent))
			{
				s3x3_minmax22h.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s3x3_minmax22h.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s3x3_minmax22v") == 0)
			if (mxIsDouble(fieldContent))
			{
				s3x3_minmax22v.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s3x3_minmax22v.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s3x3_minmax24") == 0)
			if (mxIsDouble(fieldContent))
			{
				s3x3_minmax24.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s3x3_minmax24.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s3x3_minmax41") == 0)
			if (mxIsDouble(fieldContent))
			{
				s3x3_minmax41.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s3x3_minmax41.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s3x3_spam11") == 0)
			if (mxIsDouble(fieldContent))
			{
				s3x3_spam11.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s3x3_spam11.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s3x3_spam14h") == 0)
			if (mxIsDouble(fieldContent))
			{
				s3x3_spam14h.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s3x3_spam14h.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s3x3_spam14v") == 0)
			if (mxIsDouble(fieldContent))
			{
				s3x3_spam14v.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s3x3_spam14v.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s5x5_minmax22h") == 0)
			if (mxIsDouble(fieldContent))
			{
				s5x5_minmax22h.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s5x5_minmax22h.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s5x5_minmax22v") == 0)
			if (mxIsDouble(fieldContent))
			{
				s5x5_minmax22v.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s5x5_minmax22v.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s5x5_minmax24") == 0)
			if (mxIsDouble(fieldContent))
			{
				s5x5_minmax24.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s5x5_minmax24.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s5x5_minmax41") == 0)
			if (mxIsDouble(fieldContent))
			{
				s5x5_minmax41.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s5x5_minmax41.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s5x5_spam11") == 0)
			if (mxIsDouble(fieldContent))
			{
				s5x5_spam11.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s5x5_spam11.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s5x5_spam14h") == 0)
			if (mxIsDouble(fieldContent))
			{
				s5x5_spam14h.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s5x5_spam14h.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
		else if (strcmp(fieldName, "s5x5_spam14v") == 0)
			if (mxIsDouble(fieldContent))
			{
				s5x5_spam14v.clear();
				for (int i = 0; i < mxGetNumberOfElements(fieldContent); i++)
					s5x5_spam14v.push_back((float)mxGetPr(fieldContent)[i]);
			}
			else mexErrMsgTxt("Quantization step must be a double scalar or vector");
	}

	void setQuantStep(const mxArray *quantStepStruct)
	{
		int nfields = mxGetNumberOfFields(quantStepStruct);
		if (nfields == 0) mexErrMsgTxt("The structure config.quant_step is empty.");
		for (int fieldIndex = 0; fieldIndex < nfields; fieldIndex++)
		{
			const char *fieldName = mxGetFieldNameByNumber(quantStepStruct, fieldIndex);
			const mxArray *fieldContent = mxGetFieldByNumber(quantStepStruct, 0, fieldIndex);
			if (strcmp(fieldName, "all") == 0)
				setQuanStepForALL(fieldContent);
			else if (strcmp(fieldName, "s1") == 0)
				setQuanStepForS1(fieldContent);
			else if (strcmp(fieldName, "s2") == 0)
				setQuanStepForS2(fieldContent);
			else if (strcmp(fieldName, "s3") == 0)
				setQuanStepForS3(fieldContent);
			else if (strcmp(fieldName, "s3x3") == 0)
				setQuanStepForS3x3(fieldContent);
			else if (strcmp(fieldName, "s5x5") == 0)
				setQuanStepForS5x5(fieldContent);
			else
				setQuanStepForS(fieldName, fieldContent);
		}
	}

	void chkQuantStep(const bool mergeSpams)
	{
		if (s1_minmax22h.size() == 0) { s1_minmax22h.push_back(1); s1_minmax22h.push_back(2); }
		if (s1_minmax22v.size() == 0) { s1_minmax22v.push_back(1); s1_minmax22v.push_back(2); }
		if (s1_minmax24.size() == 0) { s1_minmax24.push_back(1); s1_minmax24.push_back(2); }
		if (s1_minmax34.size() == 0) { s1_minmax34.push_back(1); s1_minmax34.push_back(2); }
		if (s1_minmax34h.size() == 0) { s1_minmax34h.push_back(1); s1_minmax34h.push_back(2); }
		if (s1_minmax34v.size() == 0) { s1_minmax34v.push_back(1); s1_minmax34v.push_back(2); }
		if (s1_minmax41.size() == 0) { s1_minmax41.push_back(1); s1_minmax41.push_back(2); }
		if (s1_minmax48h.size() == 0) { s1_minmax48h.push_back(1); s1_minmax48h.push_back(2); }
		if (s1_minmax48v.size() == 0) { s1_minmax48v.push_back(1); s1_minmax48v.push_back(2); }
		if (s1_minmax54.size() == 0) { s1_minmax54.push_back(1); s1_minmax54.push_back(2); }
		if (s1_spam14h.size() == 0) { s1_spam14h.push_back(1); s1_spam14h.push_back(2); }
		if (s1_spam14v.size() == 0) { s1_spam14v.push_back(1); s1_spam14v.push_back(2); }

		if (s2_minmax21.size() == 0) { s2_minmax21.push_back(1); s2_minmax21.push_back(1.5); s2_minmax21.push_back(2); }
		if (s2_minmax24h.size() == 0) { s2_minmax24h.push_back(1); s2_minmax24h.push_back(1.5); s2_minmax24h.push_back(2); }
		if (s2_minmax24v.size() == 0) { s2_minmax24v.push_back(1); s2_minmax24v.push_back(1.5); s2_minmax24v.push_back(2); }
		if (s2_minmax32.size() == 0) { s2_minmax32.push_back(1); s2_minmax32.push_back(1.5); s2_minmax32.push_back(2); }
		if (s2_minmax41.size() == 0) { s2_minmax41.push_back(1); s2_minmax41.push_back(1.5); s2_minmax41.push_back(2); }
		if (s2_spam12h.size() == 0) { s2_spam12h.push_back(1); s2_spam12h.push_back(1.5); s2_spam12h.push_back(2); }
		if (s2_spam12v.size() == 0) { s2_spam12v.push_back(1); s2_spam12v.push_back(1.5); s2_spam12v.push_back(2); }

		if (s3_minmax22h.size() == 0) { s3_minmax22h.push_back(1); s3_minmax22h.push_back(1.5); s3_minmax22h.push_back(2); }
		if (s3_minmax22v.size() == 0) { s3_minmax22v.push_back(1); s3_minmax22v.push_back(1.5); s3_minmax22v.push_back(2); }
		if (s3_minmax24.size() == 0) { s3_minmax24.push_back(1); s3_minmax24.push_back(1.5); s3_minmax24.push_back(2); }
		if (s3_minmax34.size() == 0) { s3_minmax34.push_back(1); s3_minmax34.push_back(1.5); s3_minmax34.push_back(2); }
		if (s3_minmax34h.size() == 0) { s3_minmax34h.push_back(1); s3_minmax34h.push_back(1.5); s3_minmax34h.push_back(2); }
		if (s3_minmax34v.size() == 0) { s3_minmax34v.push_back(1); s3_minmax34v.push_back(1.5); s3_minmax34v.push_back(2); }
		if (s3_minmax41.size() == 0) { s3_minmax41.push_back(1); s3_minmax41.push_back(1.5); s3_minmax41.push_back(2); }
		if (s3_minmax48h.size() == 0) { s3_minmax48h.push_back(1); s3_minmax48h.push_back(1.5); s3_minmax48h.push_back(2); }
		if (s3_minmax48v.size() == 0) { s3_minmax48v.push_back(1); s3_minmax48v.push_back(1.5); s3_minmax48v.push_back(2); }
		if (s3_minmax54.size() == 0) { s3_minmax54.push_back(1); s3_minmax54.push_back(1.5); s3_minmax54.push_back(2); }
		if (s3_spam14h.size() == 0) { s3_spam14h.push_back(1); s3_spam14h.push_back(1.5); s3_spam14h.push_back(2); }
		if (s3_spam14v.size() == 0) { s3_spam14v.push_back(1); s3_spam14v.push_back(1.5); s3_spam14v.push_back(2); }

		if (s3x3_minmax22h.size() == 0) { s3x3_minmax22h.push_back(1); s3x3_minmax22h.push_back(1.5); s3x3_minmax22h.push_back(2); }
		if (s3x3_minmax22v.size() == 0) { s3x3_minmax22v.push_back(1); s3x3_minmax22v.push_back(1.5); s3x3_minmax22v.push_back(2); }
		if (s3x3_minmax24.size() == 0) { s3x3_minmax24.push_back(1); s3x3_minmax24.push_back(1.5); s3x3_minmax24.push_back(2); }
		if (s3x3_minmax41.size() == 0) { s3x3_minmax41.push_back(1); s3x3_minmax41.push_back(1.5); s3x3_minmax41.push_back(2); }
		if (s3x3_spam11.size() == 0) { s3x3_spam11.push_back(1); s3x3_spam11.push_back(1.5); s3x3_spam11.push_back(2); }
		if (s3x3_spam14h.size() == 0) { s3x3_spam14h.push_back(1); s3x3_spam14h.push_back(1.5); s3x3_spam14h.push_back(2); }
		if (s3x3_spam14v.size() == 0) { s3x3_spam14v.push_back(1); s3x3_spam14v.push_back(1.5); s3x3_spam14v.push_back(2); }

		if (s5x5_minmax22h.size() == 0) { s5x5_minmax22h.push_back(1); s5x5_minmax22h.push_back(1.5); s5x5_minmax22h.push_back(2); }
		if (s5x5_minmax22v.size() == 0) { s5x5_minmax22v.push_back(1); s5x5_minmax22v.push_back(1.5); s5x5_minmax22v.push_back(2); }
		if (s5x5_minmax24.size() == 0) { s5x5_minmax24.push_back(1); s5x5_minmax24.push_back(1.5); s5x5_minmax24.push_back(2); }
		if (s5x5_minmax41.size() == 0) { s5x5_minmax41.push_back(1); s5x5_minmax41.push_back(1.5); s5x5_minmax41.push_back(2); }
		if (s5x5_spam11.size() == 0) { s5x5_spam11.push_back(1); s5x5_spam11.push_back(1.5); s5x5_spam11.push_back(2); }
		if (s5x5_spam14h.size() == 0) { s5x5_spam14h.push_back(1); s5x5_spam14h.push_back(1.5); s5x5_spam14h.push_back(2); }
		if (s5x5_spam14v.size() == 0) { s5x5_spam14v.push_back(1); s5x5_spam14v.push_back(1.5); s5x5_spam14v.push_back(2); }

		if (mergeSpams)
		{
			if (s1_spam14h.size() != s1_spam14v.size()) mexErrMsgTxt("Quantization steps must be the same for submodels to be merged.");
			else
				for (int i = 0; i < s1_spam14h.size(); i++)
					if (std::abs(s1_spam14h[i] - s1_spam14v[i]) > 0.00000001)
						mexErrMsgTxt("Quantization steps must be the same for submodels to be merged.");
			if (s2_spam12h.size() != s2_spam12v.size()) mexErrMsgTxt("Quantization steps must be the same for submodels to be merged.");
			else
				for (int i = 0; i < s2_spam12h.size(); i++)
					if (std::abs(s2_spam12h[i] - s2_spam12v[i]) > 0.00000001)
						mexErrMsgTxt("Quantization steps must be the same for submodels to be merged.");
			if (s3_spam14h.size() != s3_spam14v.size()) mexErrMsgTxt("Quantization steps must be the same for submodels to be merged.");
			else
				for (int i = 0; i < s3_spam14h.size(); i++)
					if (std::abs(s3_spam14h[i] - s3_spam14v[i]) > 0.00000001)
						mexErrMsgTxt("Quantization steps must be the same for submodels to be merged.");
			if (s3x3_spam14h.size() != s3x3_spam14v.size()) mexErrMsgTxt("Quantization steps must be the same for submodels to be merged.");
			else
				for (int i = 0; i < s3x3_spam14h.size(); i++)
					if (std::abs(s3x3_spam14h[i] - s3x3_spam14v[i]) > 0.00000001)
						mexErrMsgTxt("Quantization steps must be the same for submodels to be merged.");
			if (s5x5_spam14h.size() != s5x5_spam14v.size()) mexErrMsgTxt("Quantization steps must be the same for submodels to be merged.");
			else
				for (int i = 0; i < s5x5_spam14h.size(); i++)
					if (std::abs(s5x5_spam14h[i] - s5x5_spam14v[i]) > 0.00000001)
						mexErrMsgTxt("Quantization steps must be the same for submodels to be merged.");
			if (s3x3_spam14h.size() != s5x5_spam14h.size()) mexErrMsgTxt("Quantization steps must be the same for submodels to be merged.");
			else
				for (int i = 0; i < s3x3_spam14h.size(); i++)
					if (std::abs(s3x3_spam14h[i] - s5x5_spam14h[i]) > 0.00000001)
						mexErrMsgTxt("Quantization steps must be the same for submodels to be merged.");

		}
	}
};

#endif