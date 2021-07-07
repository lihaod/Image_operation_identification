function mkImg_JP2(inPath, outPath, inSuffix, outSuffix)

procType = 'JP2';

if ~exist(outPath,'dir'); mkdir(outPath); end
if isunix; cmdStatus = system(['chmod a+w ' outPath]); end

imgList = dir([inPath '*' inSuffix]);
x = length(imgList);
inSuffixL = length(inSuffix);

compressRatio_i = randi([20 80],1,x)/10;
save(['paras_' procType '.mat'],'compressRatio_i');

parfor i = 1:1:x
    I = imread([inPath,imgList(i).name]);
    imwrite(I,[outPath imgList(i).name(1:end-inSuffixL) outSuffix],'jp2','CompressionRatio',compressRatio_i(i),'Mode','lossy'); 
    I = imread([outPath imgList(i).name(1:end-inSuffixL) outSuffix],'jp2');
    imwrite(I, [outPath imgList(i).name(1:end-inSuffixL) outSuffix], outSuffix); 
end


