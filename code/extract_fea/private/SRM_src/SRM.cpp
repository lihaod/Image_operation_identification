#ifndef NDEBUG
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

/* Add boost libraries - Depend on Windows (32/64 bit) and run-time (debug/release) */
#if _WIN32 || _WIN64
	#if _WIN64
		#ifdef _V120
			#ifdef NDEBUG
				#pragma comment( lib, "lib/x64/Release/libboost_filesystem-vc120-mt-1_57.lib" )
				#pragma comment( lib, "lib/x64/Release/libboost_program_options-vc120-mt-1_57.lib" )
				#pragma comment( lib, "lib/x64/Release/libboost_system-vc120-mt-1_57.lib" )
			#else
				#pragma comment( lib, "lib/x64/Debug/libboost_filesystem-vc120-mt-gd-1_57.lib" )
				#pragma comment( lib, "lib/x64/Debug/libboost_program_options-vc120-mt-gd-1_57.lib" )
				#pragma comment( lib, "lib/x64/Debug/libboost_system-vc120-mt-gd-1_57.lib" )
			#endif
		#else
			#ifdef NDEBUG
	  			#pragma comment( lib, "lib/x64/Release/libboost_filesystem-vc100-mt-1_47.lib" )
				#pragma comment( lib, "lib/x64/Release/libboost_program_options-vc100-mt-1_47.lib" )
				#pragma comment( lib, "lib/x64/Release/libboost_system-vc100-mt-1_47.lib" )
			#else
		 		#pragma comment( lib, "lib/x64/Debug/libboost_filesystem-vc100-mt-gd-1_47.lib" )
				#pragma comment( lib, "lib/x64/Debug/libboost_program_options-vc100-mt-gd-1_47.lib" )
				#pragma comment( lib, "lib/x64/Debug/libboost_system-vc100-mt-gd-1_47.lib" )
			#endif
		#endif
	#else
		#ifdef _V120
			#ifdef NDEBUG
	  			#pragma comment( lib, "lib/win32/Release/libboost_filesystem-vc120-mt-1_57.lib" )
				#pragma comment( lib, "lib/win32/Release/libboost_program_options-vc120-mt-1_57.lib" )
				#pragma comment( lib, "lib/win32/Release/libboost_system-vc120-mt-1_57.lib" )
			#else
	 			#pragma comment( lib, "lib/win32/Debug/libboost_filesystem-vc120-mt-gd-1_57.lib" )
				#pragma comment( lib, "lib/win32/Debug/libboost_program_options-vc120-mt-gd-1_57.lib" )
				#pragma comment( lib, "lib/win32/Debug/libboost_system-vc120-mt-gd-1_57.lib" )
			#endif
		#else
			#ifdef NDEBUG
				#pragma comment( lib, "lib/win32/Release/libboost_filesystem-vc100-mt-1_47.lib" )
				#pragma comment( lib, "lib/win32/Release/libboost_program_options-vc100-mt-1_47.lib" )
				#pragma comment( lib, "lib/win32/Release/libboost_system-vc100-mt-1_47.lib" )
			#else
				#pragma comment( lib, "lib/win32/Debug/libboost_filesystem-vc100-mt-gd-1_47.lib" )
				#pragma comment( lib, "lib/win32/Debug/libboost_program_options-vc100-mt-gd-1_47.lib" )
				#pragma comment( lib, "lib/win32/Debug/libboost_system-vc100-mt-gd-1_47.lib" )
			#endif
		#endif
	#endif
#endif 

#include <vector>
#include "submodel.h"
#include "SRMclass.h"

#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "exception.cpp"
#include <time.h>
#include <vector>
#include "SRMclass.h"
#include "submodel.h"
#include "config.cpp"

typedef unsigned int uint;
namespace fs = boost::filesystem;
namespace po = boost::program_options;

void printInfo(){
    std::cout << "Extracts SRM features from all greyscale pgm images in the directory input-dir.\n";
	std::cout << "Author: Vojtech Holub.\n";
	std::cout << "For further details read:\n";
	std::cout << "   Rich Models for Steganalysis of Digital Images, J. Fridrich and J. Kodovsky,\n   IEEE Transactions on Information Forensics and Security, 2012\n";
	std::cout << "   http://dde.binghamton.edu/kodovsky/pdf/TIFS2012-SRM.pdf \n\n";
	std::cout << "usage: SRM -I input-dir -O output-dir [-v] [--T threshold] [--order markov-order] [--q1 quant-step-for-1st-order] [--qothers quant-step-for-other-orders] [--merge-spams] [-ss] [-sr] [-sm] [--eraseLSB] [--parity] [--roundup5] \n\n";
}

void WriteFeaToFiles(SRMclass *SRMobj, std::string oDir, bool verbose)
{
	if (verbose) std::cout << std::endl << "---------------------" << std::endl << "Writing features to the output directory" << std::endl;
	std::vector<Submodel *> submodels = SRMobj->GetSubmodels();
	for (int submodelIndex=0; submodelIndex < (int)submodels.size(); submodelIndex++)
	{
		Submodel *currentSubmodel = submodels[submodelIndex];
		std::string submodelName = currentSubmodel->GetName();
		if (verbose) std::cout << "   " << submodelName << std::endl;

		fs::path dir (oDir);
		fs::path file (submodelName);
		std::string full_path = ((dir / file).string()+ ".fea");

		if (fs::exists(full_path)) fs::remove(full_path);

		std::ofstream outputFile;
		outputFile.open(full_path.c_str());
		for (int imageIndex=0; imageIndex < (int)currentSubmodel->ReturnFea().size(); imageIndex++)
		{
			float *currentLine = (currentSubmodel->ReturnFea())[imageIndex];
			for (int feaIndex=0; feaIndex < currentSubmodel->symmDim; feaIndex++)
			{
				outputFile << currentLine[feaIndex] << " ";
			}
			outputFile << SRMobj->imageNames[imageIndex] << std::endl;
		}
		outputFile.close();
	}
}

int main(int argc, char** argv)
{

#ifndef NDEBUG
_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try { 
		int T, order;
		std::string iDir, oDir;
		std::vector< std::string > images;
		bool verbose = false;
		bool symmSign = false, symmReverse = false, symmMinMax = false; bool mergeSpams = false;
		bool eraseLSB = false, parity = false;
		bool roundup5 = false;
		po::variables_map vm;

		std::string q_1_string;
		std::string q_others_string;

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
			("verbose,V",       po::bool_switch(&verbose),                     "print out verbose messages")
            ("input-dir,I",     po::value<std::string>(&iDir),                 "directory with images to calculate the features from")
            ("images,i",        po::value<std::vector<std::string> >(&images),  "list of images to calculate the features from")
			("output-dir,O",    po::value<std::string>(&oDir),                 "dir to output features from all submodels")
            ("T",               po::value<int>(&T)->default_value(2),          "upper bound on absolute value of the residuals")
            ("order",           po::value<int>(&order)->default_value(4),      "order of the co-occurrence matrix")
			("q1",				po::value<std::string>(&q_1_string),           "quantization steps for 1st order")
			("qothers",			po::value<std::string>(&q_others_string),      "quantization steps for other orders")
			("merge-spams",		po::bool_switch(&mergeSpams),                  "switch off merging SPAM features")
			("ss",				po::bool_switch(&symmSign),                    "switch off sign symmetry")
			("sr",				po::bool_switch(&symmReverse),                 "switch off reverse symmetry")
			("sm",				po::bool_switch(&symmMinMax),                  "switch off minmax symmetry")
			("eraseLSB",		po::bool_switch(&eraseLSB),					"switch on erasing image LSBs before processing")
			("parity",			po::bool_switch(&parity),					"switch on perity residual")
			("roundup5",		po::bool_switch(&roundup5),					"switch on rounding 0.5 to 1.0")
            ;

        po::positional_options_description p;
        p.add("cover-images", -1);

        po::store(po::command_line_parser(argc,argv).options(desc).positional(p).run(), vm);
        po::notify(vm);		

		symmSign = !symmSign;
		symmReverse = !symmReverse;
		symmMinMax = !symmMinMax;
		mergeSpams = !mergeSpams;

        if (vm.count("help"))  { printInfo(); std::cout << desc << "\n"; return 1; }
        if (!vm.count("output-dir")){ std::cout << "'output-dir' is required.\n" << desc << "\n"; return 1; }
		else if (!fs::is_directory(fs::path(oDir))) { std::cout << "'output-dir' must be an existing directory.\n" << desc << "\n"; return 1; }

		// add all pgm and files from the input directory to the vector
		fs::directory_iterator end_itr; // default construction yields past-the-end
		if (vm.count("input-dir"))
			for ( fs::directory_iterator itr(iDir); itr!=end_itr; ++itr ) 
			{
				if ( (!fs::is_directory(itr->status())) && (itr->path().extension()==".pgm") )
					images.push_back(itr->path().string());
            }


		// set the quantization steps
		std::vector<float> q_1 = std::vector<float>();
		std::vector<float> q_others = std::vector<float>();
		float temp = 0, tempMul = 1, factor = 1;

		for (int i = 0; i < q_1_string.size(); i++)
		{
			factor *= tempMul;
			if (isdigit(q_1_string[i]))
				temp = temp * 10 + (q_1_string[i] - '0');
			else if (q_1_string[i] == '-')
			{
				q_1.push_back(temp / factor);
				temp = 0;
				tempMul = 1;
				factor = 1;
			}
			else if (q_1_string[i] == '.')
				tempMul = 10;
		}
		if (q_1.size() == 0)
		{
			q_1.push_back(1); q_1.push_back(2);
		}

		for (int i = 0; i < q_others_string.size(); i++)
		{
			factor *= tempMul;
			if (isdigit(q_others_string[i]))
				temp = temp * 10 + (q_others_string[i] - '0');
			else if (q_others_string[i] == '-')
			{
				q_others.push_back(temp / factor);
				temp = 0;
				tempMul = 1;
				factor = 1;
			}
			else if (q_others_string[i] == '.')
				tempMul = 10;
		}
		if (q_others.size() == 0)
		{
			q_others.push_back(1); q_others.push_back(1.5); q_others.push_back(2);
		}


		// create config object
		Config *config = new Config(verbose, T, order, q_1, q_others, symmSign, symmReverse, symmMinMax, mergeSpams, eraseLSB, parity, roundup5);
		// create object with all the submodels and compute the features
		SRMclass *SRMobj = new SRMclass(config);

		// Run the feature computation
		SRMobj->ComputeFeatures(images);

		// writes features from all the submodels to the separate files
		WriteFeaToFiles(SRMobj, oDir, verbose);

		// Remove SRMobj from the memory
		delete SRMobj;

		// Remove config from the memory
		delete config;
    } 
	catch(std::exception& e) 
	{ 
		std::cerr << "error: " << e.what() << "\n"; return 1; 
	} 
	catch(...) 
	{ 
		std::cerr << "Exception of unknown type!\n";  return 1;
	}

	return 0;
}
