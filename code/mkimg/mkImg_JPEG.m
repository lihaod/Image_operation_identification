function mkImg_JPEG(inPath, outPath, inSuffix, outSuffix)

procType = 'JPEG';

if ~exist(outPath,'dir'); mkdir(outPath); end
if isunix; cmdStatus = system(['chmod a+w ' outPath]); end

imgList = dir([inPath '*' inSuffix]);
x = length(imgList);
inSuffixL = length(inSuffix);

QF_i = randi([75 99],1,x);
save(['paras_' procType '.mat'],'QF_i');

parfor i = 1:1:x
    I = imread([inPath,imgList(i).name]);
    imwrite(I,[outPath imgList(i).name(1:end-inSuffixL) outSuffix],'jpg','Quality',QF_i(i)); 
    I = imread([outPath imgList(i).name(1:end-inSuffixL) outSuffix],'jpg');
    imwrite(I, [outPath imgList(i).name(1:end-inSuffixL) outSuffix], outSuffix); 
end


