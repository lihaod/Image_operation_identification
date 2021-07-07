function mkImg_GF(inPath, outPath, inSuffix, outSuffix)

procType = 'GF';
hsize = [3 5 7];
sigma = [0.8 0.9 1.0 1.1 1.2 1.3 1.4 1.5 1.6];

if ~exist(outPath,'dir'); mkdir(outPath); end
if isunix; cmdStatus = system(['chmod a+w ' outPath]); end

imgList = dir([inPath '*' inSuffix]);
x = length(imgList);
inSuffixL = length(inSuffix);

hsize_i = hsize(randi([1 length(hsize)],1,x));
sigma_i = sigma(randi([1 length(sigma)],1,x));
save(['paras_' procType '.mat'],'hsize_i','sigma_i');

parfor i = 1:1:x
    I = imread([inPath,imgList(i).name]);
    I = padarray(I,[5 5],'symmetric');
    h = fspecial('gaussian', hsize_i(i), sigma_i(i));
    I = imfilter(I, h);
    I = I(6:end-5,6:end-5);
    imwrite(I, [outPath imgList(i).name(1:end-inSuffixL) outSuffix], outSuffix); 
end
