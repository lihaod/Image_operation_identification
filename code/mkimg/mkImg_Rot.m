function mkImg_Rot(inPath, outPath, inSuffix, outSuffix)

procType = 'Rot';
angle = [1 3 5 10 15 20 25 30 35 40 45];
inte  = {'bilinear','bicubic'};

if ~exist(outPath,'dir'); mkdir(outPath); end
if isunix; cmdStatus = system(['chmod a+w ' outPath]); end

imgList = dir([inPath '*' inSuffix]);
x = length(imgList);
inSuffixL = length(inSuffix);

angle_i = angle(randi([1 length(angle)],1,x));
inte_i = inte(randi([1 length(inte)],1,x));
save(['paras_' procType '.mat'],'angle_i','inte_i');

parfor i = 1:1:x
    I = imread([inPath,imgList(i).name]);
    I = imrotate(I,angle_i(i),inte_i{i},'crop');
    imwrite(I, [outPath imgList(i).name(1:end-inSuffixL) outSuffix], outSuffix); 
end
