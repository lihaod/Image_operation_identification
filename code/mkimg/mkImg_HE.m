function mkImg_HE(inPath, outPath, inSuffix, outSuffix)

procType = 'HE';

if ~exist(outPath,'dir'); mkdir(outPath); end
if isunix; cmdStatus = system(['chmod a+w ' outPath]); end

imgList = dir([inPath '*' inSuffix]);
x = length(imgList);
inSuffixL = length(inSuffix);

parfor i = 1:1:x
    I = imread([inPath,imgList(i).name]);
    I = histeq(I, 256);
    imwrite(I, [outPath imgList(i).name(1:end-inSuffixL) outSuffix], outSuffix); 
end


