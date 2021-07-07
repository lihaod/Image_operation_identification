%%
clear
addpath('mkimg/','extract_fea/','ensemble/');

imgdbPath   = '../imgdb/';
procType = {'Orig';'GC';'HE';'UM';'MeanF';'GF';'MedF';'WF';'Sca';'Rot';'JPEG';'JP2'};
inSuffix  = 'pgm';
outSuffix = 'pgm';

%%
mkdir([imgdbPath 'orig_gray_256/']);
crop256([imgdbPath 'orig_gray_512/'],[imgdbPath 'orig_gray_256/']);
imgPath = {[imgdbPath 'orig_gray_256/']};

for k = 2:length(procType)
    outPath = [imgdbPath 'proc_' procType{k} '/'];
    imgPath{end+1,1} = outPath;
    mkImgFun = str2func(['mkImg_'  procType{k}]);
    mkImgFun([imgdbPath 'orig_gray_512/'], outPath, inSuffix, outSuffix);
    crop256(outPath, outPath);
end

%% Extract features
dataPath = '../data/';
mkdir('../data/');
feaName = 'srm_t1_o3_qo';
feaFun = @srm_T1_O3_qo;
matList = extract_fea(procType,imgPath,dataPath,feaName,feaFun);

delete(gcp('nocreate'));

%% Classification
acc = train_test_binary(matList(1),matList(2:end),2)
[meanConfuseMat,acc] = train_test_multi(matList, '../model/', '../result/', ['M_' feaName], 2, 10)

