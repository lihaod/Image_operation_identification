function mkImg_UM(inPath, outPath, inSuffix, outSuffix)

procType = 'UM';
radius = [0.5 0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3 1.4 1.5];
amount = [0.5 0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3 1.4 1.5];

if ~exist(outPath,'dir'); mkdir(outPath); end
if isunix; cmdStatus = system(['chmod a+w ' outPath]); end

imgList = dir([inPath '*' inSuffix]);
x = length(imgList);
inSuffixL = length(inSuffix);

radius_i = radius(randi([1 length(radius)],1,x));
amount_i = amount(randi([1 length(amount)],1,x));
save(['paras_' procType '.mat'],'radius_i','amount_i');

parfor i = 1:1:x
    I = imread([inPath,imgList(i).name]);
    I = padarray(I,[5 5],'symmetric');
    I = imsharpen(I,'Radius',radius_i(i),'Amount',amount_i(i));
    I = I(6:end-5,6:end-5);
    imwrite(I, [outPath imgList(i).name(1:end-inSuffixL) outSuffix], outSuffix); 
end

