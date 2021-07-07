function mkImg_Sca(inPath, outPath, inSuffix, outSuffix)

procType = 'Sca';
scale = [1.01 1.03 1.05 1.1 1.2 1.3 1.4 1.5 1.6 1.7 1.8 1.9 0.99 0.97 0.95 0.9 0.85 0.8 0.75 0.7 0.65 0.6 0.55];
inte  = {'bilinear','bicubic'};

if ~exist(outPath,'dir'); mkdir(outPath); end
if isunix; cmdStatus = system(['chmod a+w ' outPath]); end

imgList = dir([inPath '*' inSuffix]);
x = length(imgList);
inSuffixL = length(inSuffix);

scale_i = scale(randi([1 length(scale)],1,x));
inte_i = inte(randi([1 length(inte)],1,x));
save(['paras_' procType '.mat'],'scale_i','inte_i');

parfor i = 1:1:x
    I = imread([inPath,imgList(i).name]);
    I = imresize(I, scale_i(i),inte_i{i}, 'Antialiasing', false);
    imwrite(I, [outPath imgList(i).name(1:end-inSuffixL) outSuffix], outSuffix); 
end


