//#define CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

#ifdef GLNXA64
	#include <stdio.h>
	#include <cstring>
#endif

#include <vector>
#include "submodel.h"
#include "SRMclass.h"

#include <mex.h>

/*
	prhs[0] - cell array of image paths
	prhs[1] - struct config
				config.T			- int32		- default 2				- residual threshold
				config.order		- int32		- default 4				- co-occurrence order
				config.quant_step	- struct	- default [1, (1.5), 2]	- quantization steps for each submodel
				config.merge_spams	- logical	- default true			- if true then spam features are merged
				config.symm_sign	- logical	- default true			- if true then spam symmetry is used
				config.symm_reverse	- logical	- default true			- if true then reverse symmetry is used
				config.symm_minmax	- logical	- default true			- if true then minmax symmetry is used
				config.eraseLSB		- logical	- default false			- if true then all LSB are erased from the image
				config.parity		- logical	- default false			- if true then parity residual is applied
				config.roundup5		- logical	- default false			- if true then 0.5 will be rounded to 1.0
*/
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) 
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	if ((nrhs != 1) && (nrhs != 2))
		mexErrMsgTxt("One or two inputs are required.\n1 input - [cell_array Paths | uint8_array Image]\n2 inputs - [cell_array Paths | uint8_array Image] [struct config]");

	const mxArray *imageSet = prhs[0];
	const mxArray *configStruct = prhs[1];
	const char *classofimageSet = mxGetClassName(imageSet);

	// Default config
	int T = 2;
	int order = 4;
	bool mergeSpams = true;
	bool ss = true, sr = true, sm = true;
	bool eraseLSB = false, parity = false;
	bool roundup5 = false;
	bool mapMode = true;
	int numInMap = 1;
	QuantStep *quantStep = new QuantStep;
	mat2D<double> *c_map = NULL;

	if (strcmp(classofimageSet, "cell") != 0 && strcmp(classofimageSet, "uint8") != 0)
		mexErrMsgTxt ("The first input must be a cell array with image paths or a uint8 image matrix.");
	if (nrhs == 2)
	{
		if (!mxIsStruct(configStruct)) mexErrMsgTxt("The config must be a structure.");
		int nfields = mxGetNumberOfFields(configStruct);
		if (nfields==0) mexErrMsgTxt ("The config structure is empty.");
		for(int fieldIndex=0; fieldIndex<nfields; fieldIndex++)
		{
			const char *fieldName = mxGetFieldNameByNumber(configStruct, fieldIndex);
			const mxArray *fieldContent = mxGetFieldByNumber(configStruct, 0, fieldIndex);
			if (strcmp(fieldName, "quant_step") == 0)
				if (mxIsStruct(fieldContent)) quantStep->setQuantStep(fieldContent);
				else mexErrMsgTxt("'config.quant_step' must be a structure");
			else if (strcmp(fieldName, "map") == 0)
				if (mxIsDouble(fieldContent))
				{
					int rows = (int)mxGetM(fieldContent);
					int cols = (int)mxGetN(fieldContent);
					c_map = new mat2D<double>(rows, cols);
					double *map_array = (double *)mxGetData(fieldContent);
					for (int c = 0; c<cols; c++)
						for (int r = 0; r<rows; r++)
							c_map->Write(r, c, map_array[r + c*rows]);
				}
				else mexErrMsgTxt("'config.map' must be a double matrix");
			// if a field is not scalar
			else if ((mxGetM(fieldContent)!= 1) || (mxGetN(fieldContent)!= 1))
				mexErrMsgTxt("All config fields must be scalars, except for 'quant_step' and 'map'.");
			// if every field is scalar
			else if (strcmp(fieldName, "T") == 0)
				if (mxIsClass(fieldContent, "int32")) T = (int)mxGetScalar(fieldContent);
				else mexErrMsgTxt("'config.T' must be of type 'int32'");
			else if (strcmp(fieldName, "order") == 0)
				if (mxIsClass(fieldContent, "int32")) order = (int)mxGetScalar(fieldContent);
				else mexErrMsgTxt("'config.order' must be of type 'int32'");
			else if (strcmp(fieldName, "merge_spams") == 0)
				if (mxIsLogical(fieldContent)) mergeSpams = mxIsLogicalScalarTrue(fieldContent);
				else mexErrMsgTxt("'config.mergeSpams' must be of type 'logical'");
			else if (strcmp(fieldName, "symm_sign") == 0)
				if (mxIsLogical(fieldContent)) ss = mxIsLogicalScalarTrue(fieldContent);
				else mexErrMsgTxt("'config.symm_sign' must be of type 'logical'");
			else if (strcmp(fieldName, "symm_reverse") == 0)
				if (mxIsLogical(fieldContent)) sr = mxIsLogicalScalarTrue(fieldContent);
				else mexErrMsgTxt("'config.symm_reverse' must be of type 'logical'");
			else if (strcmp(fieldName, "symm_minmax") == 0)
				if (mxIsLogical(fieldContent)) sm = mxIsLogicalScalarTrue(fieldContent);
				else mexErrMsgTxt("'config.symm_minmax' must be of type 'logical'");
			else if (strcmp(fieldName, "eraseLSB") == 0)
				if (mxIsLogical(fieldContent)) eraseLSB = mxIsLogicalScalarTrue(fieldContent);
				else mexErrMsgTxt("'config.eraseLSB' must be of type 'logical'");
			else if (strcmp(fieldName, "parity") == 0)
				if (mxIsLogical(fieldContent)) parity = mxIsLogicalScalarTrue(fieldContent);
				else mexErrMsgTxt("'config.parity' must be of type 'logical'");
			else if (strcmp(fieldName, "roundup5") == 0)
				if (mxIsLogical(fieldContent)) roundup5 = mxIsLogicalScalarTrue(fieldContent);
				else mexErrMsgTxt("'config.roundup5' must be of type 'logical'");
			else if (strcmp(fieldName, "mapIsWeights") == 0)
				if (mxIsLogical(fieldContent)) mapMode = mxIsLogicalScalarTrue(fieldContent);
				else mexErrMsgTxt("'config.mapModeIsWeights' must be of type 'logical', 1 for 'weights', 0 for 'segments'");
			else if (strcmp(fieldName, "numInMap") == 0)
				if (mxIsClass(fieldContent, "int32")) numInMap = (int)mxGetScalar(fieldContent);
				else mexErrMsgTxt("'config.numInMap' must be of type 'int32'");
		}
	}

	//  sheck and set default quantization steps if they are not specified
	quantStep->chkQuantStep(mergeSpams);

	// if no map is given, set mapMode = true. mapMode = false will divide the image into serveral subimages for feature extraction.
	if (NULL == c_map) mapMode = true;
	// if map is weights, set numInMap as 1
	if (mapMode) numInMap = 1;

	// create config object
	Config *config = new Config(false, T, order, ss, sr, sm, mergeSpams, eraseLSB, parity, roundup5, mapMode, numInMap);

	// create object with all the submodels and compute the features
	SRMclass *SRMobj = new SRMclass(quantStep,config);

	if (strcmp(classofimageSet, "uint8") == 0)
	{

		// convert mxArray *imageSet to mat2D
		int rows = mxGetM(imageSet);
		int cols = mxGetN(imageSet);
		mat2D<int> *c_image = new mat2D<int>(rows, cols);
		const unsigned char *image_array = (unsigned char *)mxGetData(imageSet);
		for (int c = 0; c<cols; c++)
			for (int r = 0; r<rows; r++)
				c_image->Write(r, c, image_array[c*rows + r]);

		if (NULL == c_map)
		{
			c_map = new mat2D<double>(rows, cols);
			for (int c = 0; c<cols; c++)
				for (int r = 0; r<rows; r++)
					c_map->Write(r, c, 1);
		}

		// Run the feature computation
		SRMobj->ComputeFeatures(c_image,c_map);

		delete c_image;
		delete c_map;
	}
	else if (strcmp(classofimageSet, "cell") == 0)
	{

		int imageCount = (int)mxGetNumberOfElements(imageSet);
		std::vector<std::string> imageNameVector;
		for (int imageIndex = 0; imageIndex < imageCount; imageIndex++)
		{
			char *buf;
			int   buflen;
			int   status;
			mxArray *mxImagePath = mxGetCell(imageSet, imageIndex);
			buflen = (int)(mxGetM(mxImagePath) * mxGetN(mxImagePath)) + 1;
			buf = (char*)mxCalloc(buflen, sizeof(char));
			status = mxGetString(mxImagePath, buf, buflen);
			if (status != 0)
				mexErrMsgTxt("Could not convert input to a string.");
			imageNameVector.push_back(buf);
		}

		// Run the feature computation
		SRMobj->ComputeFeatures(imageNameVector);
	}

	std::vector<Submodel *> submodels = SRMobj->GetSubmodels();
	const char **submodelNames = new const char*[submodels.size()];
	std::string *submodelNamesAsString = new std::string[submodels.size()];
	for (int submodelIndex=0; submodelIndex < (int)submodels.size(); submodelIndex++) 
	{
		submodelNamesAsString[submodelIndex] = submodels[submodelIndex]->GetName();
		submodelNames[submodelIndex] = submodelNamesAsString[submodelIndex].c_str();
	}
	mwSize structSize[2];
	structSize[0] = 1;
	structSize[1] = 1;
	plhs[0] = mxCreateStructArray(1, structSize, submodels.size(), submodelNames);
	for (int submodelIndex=0; submodelIndex < submodels.size(); submodelIndex++)
	{
		Submodel *currentSubmodel = submodels[submodelIndex];
		mwSize feaSize[3];
		feaSize[0] = (int)currentSubmodel->ReturnFea().size();
		feaSize[1] = currentSubmodel->symmDim;
		feaSize[2] = currentSubmodel->numInMap;
		mxArray *fea = mxCreateNumericArray(3, feaSize, mxSINGLE_CLASS, mxREAL);
		for (int r = 0; r < (int)feaSize[0]; r++)
			for (int c = 0; c < currentSubmodel->symmDim; c++)
				for (int h = 0; h < currentSubmodel->numInMap; h++)
					((float*)mxGetPr(fea))[(h*(int)feaSize[0] * (int)feaSize[1]) + (c*(int)feaSize[0]) + r] = (currentSubmodel->ReturnFea())[r][c + h*(int)feaSize[1]];
		mxSetFieldByNumber(plhs[0],0,submodelIndex,fea);
	}

	delete[] submodelNamesAsString;
	delete[] submodelNames;
	
	delete quantStep;
	delete config;
	delete SRMobj;
} 

