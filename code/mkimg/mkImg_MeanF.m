function mkImg_MeanF(inPath, outPath, inSuffix, outSuffix)

procType = 'MeanF';
hsize = [3 5 7];

if ~exist(outPath,'dir'); mkdir(outPath); end
if isunix; cmdStatus = system(['chmod a+w ' outPath]); end

imgList = dir([inPath '*' inSuffix]);
x = length(imgList);
inSuffixL = length(inSuffix);

hsize_i = hsize(randi([1 length(hsize)],1,x));
save(['paras_' procType '.mat'],'hsize_i');

parfor i = 1:1:x
    I = imread([inPath,imgList(i).name]);
    I = padarray(I,[5 5],'symmetric');
    h = fspecial('average', [hsize_i(i), hsize_i(i)]);
    I = imfilter(I, h);
    I = I(6:end-5,6:end-5);
    imwrite(I, [outPath imgList(i).name(1:end-inSuffixL) outSuffix], outSuffix); 
end
