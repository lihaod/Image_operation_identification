#include "SRMclass.h"
#include "submodel.h"
#include "image.h"
#include <stdio.h>
#include <ctime>
#include <vector>
#include "exception.cpp"
#include "s.h"
#include "s1/s1.cpp"
#include "s2/s2.cpp"
#include "s3/s3.cpp"
#include "s3x3/s3x3.cpp"
#include "s5x5/s5x5.cpp"
#include <vector>
#include "config.cpp"
#include "s1/s1_spam14h.cpp"

SRMclass::SRMclass(QuantStep *quantStep, Config *config)
{
	this->config = config;

	// Set quantization steps for different submodels
	
	//std::vector<float> qs_1_2 = std::vector<float>(); 
	//qs_1_2.push_back(1);qs_1_2.push_back(2);
	//std::vector<float> qs_1_15_2 = std::vector<float>(); 
	//qs_1_15_2.push_back(1);qs_1_15_2.push_back(1.5);qs_1_15_2.push_back(2);

	/* SUBMODELS CLASSES */
	// s1
	submodelClasses.push_back((s *)(new s1(quantStep, config)));
	// s2
	submodelClasses.push_back((s *)(new s2(quantStep, config)));
	// s3
	submodelClasses.push_back((s *)(new s3(quantStep, config)));
	// s3x3
	submodelClasses.push_back((s *)(new s3x3(quantStep, config)));
	// s5x5
	submodelClasses.push_back((s *)(new s5x5(quantStep, config)));
}

SRMclass::~SRMclass()
{
	delete (s1 *)submodelClasses[0];
	delete (s2 *)submodelClasses[1];
	delete (s3 *)submodelClasses[2];
	delete (s3x3 *)submodelClasses[3];
	delete (s5x5 *)submodelClasses[4];
	this->submodelClasses.clear();
	clearAddedMergesSpams();
}

std::vector<Submodel *> SRMclass::GetSubmodels()
{
	std::vector<Submodel *> submodels;
	for (int classIndex=0; classIndex < (int)submodelClasses.size(); classIndex++)
		for (int qIndex=0; qIndex < (int)submodelClasses[classIndex]->submodels.size(); qIndex++)
			for (int submodelIndex=0; submodelIndex < (int)submodelClasses[classIndex]->submodels[qIndex].size(); submodelIndex++)
				submodels.push_back(submodelClasses[classIndex]->submodels[qIndex][submodelIndex]);

	submodels = PostProcessing(submodels);

	return submodels;
}

mat2D<int> *SRMclass::Image2mat2D(image *img)
{
	mat2D<int> *I = new mat2D<int>(img->height, img->width);
	for (int r=0; r<I->rows; r++)
		for (int c=0; c<I->cols; c++)
			I->Write(r, c, (int)img->pixels[c*I->rows+r]);
	return I;
}

void PrintSorted(Submodel* submodel)
{
	std::cout << std::endl << submodel->GetName() << std::endl;
	std::vector<float *> fea = submodel->ReturnFea();
	float* feaLine = fea[fea.size()-1];
	for (int i=0; i<submodel->symmDim; i++) 
	{
		float min = 0;
		int minind = -1;
		for (int j=0; j<submodel->symmDim; j++)
		{
			if (feaLine[j] > min)
			{
				min = feaLine[j];
				minind = j;
			}
		}	
		feaLine[minind] = 0;
		std::cout << min << "  ";
	}
	std::cout << "\n-----------------\n";	
}

mat2D<int> *SRMclass::LoadImage(std::string path)
{
	if(config->verbose) std::cout << "processing " << path << std::endl;

	// read image using the script in 'image.cpp'
	image *img = new image();
	if (path.substr(path.find_last_of(".")) == ".pgm")
		img->load_from_pgm(path);
    else
        throw exception("File '" + path + "' is in unknown format, we support grayscale 8bit pgm.");
		
	// move the image into a mat2D class
	mat2D<int> *I = new mat2D<int>(img->height, img->width);
	for (int r=0; r<I->rows; r++)
		for (int c=0; c<I->cols; c++)
			I->Write(r, c, (int)img->pixels[c*I->rows+r]);
	delete img;

	// remove LSBs from the image if in config
	if (config->eraseLSB)
		for (int r=0; r<I->rows; r++)
			for (int c=0; c<I->cols; c++)
			{
				int iValue = I->Read(r, c);
				iValue = iValue - (iValue % 2);
				I->Write(r, c, iValue);
			}

	return I;
}

void SRMclass::ComputeFeatures(std::vector<std::string> imagePaths)
{
	for (int i=0; i<(int)imagePaths.size(); i++)
	{
		this->imageNames.push_back(imagePaths[i].substr( imagePaths[i].find_last_of( '\\' ) +1 ));
	}

	clock_t init, final;
	init=clock();

	for(int imageIndex=0; imageIndex<(int)imagePaths.size(); imageIndex++) 
	{
		mat2D<int> *I = LoadImage(imagePaths[imageIndex]);

		mat2D<double> *map = new mat2D<double>(I->rows, I->cols);
		for (int r = 0; r<I->rows; r++)
			for (int c = 0; c<I->cols; c++)
				map->Write(r, c, 1);

		mat2D<int> *parity = NULL;
		if (config->parity) parity = GetParity(I);

		for (int submodelClassIndex=0; submodelClassIndex<(int)this->submodelClasses.size(); submodelClassIndex++)
		{
			s *currentSubmodelClass = this->submodelClasses[submodelClassIndex];
			currentSubmodelClass->ComputeImage(I, map, parity);
		}
		delete I;
		delete map;
		if (NULL != parity) delete parity;
	}

	final=clock()-init;
	if (config->verbose) printf("\nTime of calculation is %.4f seconds\n", (double)final / ((double)CLOCKS_PER_SEC));
}

void SRMclass::ComputeFeatures(mat2D<int> * I, mat2D<double> * map)  //Reload for numerical image data input
{
	mat2D<int> *parity = NULL;
	if (config->parity) parity = GetParity(I);

	clock_t init, final;
	init = clock();

	for (int submodelClassIndex = 0; submodelClassIndex<(int)this->submodelClasses.size(); submodelClassIndex++)
	{
		s *currentSubmodelClass = this->submodelClasses[submodelClassIndex];
		currentSubmodelClass->ComputeImage(I, map, parity);
	}

	final = clock() - init;
	if (config->verbose) printf("\nTime of calculation is %.4f seconds\n", (double)final / ((double)CLOCKS_PER_SEC));
}


std::vector<Submodel *> SRMclass::PostProcessing(std::vector<Submodel *> submodels)
{
	if (this->config->mergeSpams) 
	{
		// Merging of SPAMs "h" + "v" to "hv" with twice the dimension
		// Find all pairs of submodels to be merged
		clearAddedMergesSpams(); // this->AddedMergesSpams.clear();
		const std::string removeStrConst = "RM";
		std::vector<Submodel *> Hspams;
		std::vector<Submodel *> Vspams;
		for (int subHIndex=0; subHIndex < (int)submodels.size(); subHIndex++)
		{
			if (!(submodels[subHIndex]->mergeInto=="") && (submodels[subHIndex]->mergeInto!=removeStrConst))
			{
				Submodel *subH = submodels[subHIndex];
				std::string mergedName = subH->mergeInto;
				int subVIndex;
				// find the second element to the pair
				for (subVIndex=subHIndex+1; (subVIndex < (int)submodels.size()) && !((submodels[subVIndex]->mergeInto == mergedName) && (submodels[subVIndex]->q == submodels[subHIndex]->q)); subVIndex++);
				// When a pair is found
				if (subVIndex < (int)submodels.size())
				{
					// subV is removed from the list, subH serves as a base for new submodel
					Hspams.push_back(subH);
					submodels[subVIndex]->mergeInto = removeStrConst;
					Vspams.push_back(submodels[subVIndex]);
				}
			}
		}
		// Merge those pairs into HV and move it instead of H;
		for (int pairIndex=0; pairIndex<(int)Hspams.size(); pairIndex++)
		{
			Submodel *subH = Hspams[pairIndex];
			Submodel *subV = Vspams[pairIndex];
			Submodel *subHV = new s1_spam14h(subH->q, config);
			// merge co-occurrences
			std::vector<float *> newCooc;
			for (int imageIndex=0; imageIndex < (int)subV->ReturnFea().size(); imageIndex ++)
			{
				float *newCoocLine = new float[subH->symmDim*subH->numInMap+subV->symmDim*subV->numInMap]; 

				for (int j = 0; j < subH->numInMap; j++)
				{
					int offsetH = j*(subH->symmDim);
					int offsetV = j*(subV->symmDim);
					for (int i = 0; i<subH->symmDim; i++) newCoocLine[i + offsetH + offsetV] = subH->fea[imageIndex][i + offsetH];
					for (int i = 0; i < subV->symmDim; i++) newCoocLine[subH->symmDim + i + offsetH + offsetV] = subV->fea[imageIndex][i + offsetV];
				}
				newCooc.push_back(newCoocLine);
			}
			subHV->fea = newCooc;
			subHV->symmDim = subH->symmDim+ subV->symmDim;
			subHV->modelName = subH->mergeInto;
			subHV->mergeInto = "";
			subH->mergeInto = removeStrConst;
			AddedMergesSpams.push_back(subHV);
		}
		// remove subV from the list and delete it
		std::vector<Submodel *> newSubmodels;
		for (int i=0; i<(int)submodels.size(); i++)
		{
			if (submodels[i]->mergeInto!=removeStrConst)
				newSubmodels.push_back(submodels[i]);
		}
		for (int i=0; i<(int)AddedMergesSpams.size(); i++)
		{
			newSubmodels.push_back(AddedMergesSpams[i]);
		}
		submodels = newSubmodels;
	}
	return submodels;
}

mat2D<int> *SRMclass::GetParity(mat2D<int> *I)
{
	mat2D<int> *parity = new mat2D<int>(I->rows, I->cols);
	for (int r=0; r<I->rows; r++)
		for (int c=0; c<I->cols; c++)
		{
			int iValue = I->Read(r, c);
			iValue = 2*(iValue % 2) - 1;
			parity->Write(r, c, iValue);
		}
	return parity;
}

void SRMclass::clearAddedMergesSpams()
{
	for (int i = 0; i < (int)AddedMergesSpams.size(); i++)
	{
		delete AddedMergesSpams[i];
	}
	this->AddedMergesSpams.clear();
}