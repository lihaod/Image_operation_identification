function crop256(inPath,outPath)
wsize = 256;
w = wsize/2;

imgList = dir([inPath '*.pgm']);
parfor k = 1:length(imgList)
    I = imread([inPath imgList(k).name]);
    [a,b] = size(I);
    cenA = floor(a/2);
    cenB = floor(b/2);
    I = I(cenA-w+1:cenA+w,cenB-w+1:cenB+w);
    imwrite(I,[outPath imgList(k).name]);
end

