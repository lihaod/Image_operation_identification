function mkImg_GC(inPath, outPath, inSuffix, outSuffix)

procType = 'GC';
gamma = [0.5 0.6 0.7 0.8 0.9 1.2 1.4 1.6 1.8 2.0];

if ~exist(outPath,'dir'); mkdir(outPath); end
if isunix; cmdStatus = system(['chmod a+w ' outPath]); end

imgList = dir([inPath '*' inSuffix]);
x = length(imgList);
inSuffixL = length(inSuffix);

gamma_i = gamma(randi([1 length(gamma)],1,x));
save(['paras_' procType '.mat'],'gamma_i');

parfor i = 1:1:x
    I = imread([inPath,imgList(i).name]);
    I = imadjust(I, [], [], gamma_i(i));
    imwrite(I, [outPath imgList(i).name(1:end-inSuffixL) outSuffix], outSuffix); 
end
