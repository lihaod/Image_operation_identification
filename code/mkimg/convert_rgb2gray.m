%%
clear
inPath   = '/home/lihaodong/szudb/orig_color_512/';
outPath  = '/home/lihaodong/szudb/orig_gray_512/';
inSuffix  = 'ppm';
outSuffix = 'pgm';

imgList = dir([inPath '*' inSuffix]);
inSuffixL = length(inSuffix);
parfor k = 1:length(imgList)
    I = rgb2gray(imread([inPath imgList(k).name]));
    imwrite(I,[outPath imgList(k).name(1:end-inSuffixL) outSuffix]);
end