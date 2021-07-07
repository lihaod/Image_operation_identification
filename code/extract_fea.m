function varargout = extract_fea(procType,imgPath,dataPath,feaName,feaFun)

matList = cell(size(procType));
for i = 1:length(imgPath)
    imgList = dir([imgPath{i} '*.pgm']);
    F = zeros(length(imgList),714,'single');
    parfor k = 1:length(imgList)
        I  = imread([imgPath{i} imgList(k).name]);
        F(k,:) = feaFun(I);
    end
    save([dataPath procType{i} '_' feaName '.mat'],'F');
    matList{i} = [dataPath procType{i} '_' feaName '.mat'];
end

if nargout
    varargout{1} = matList;
end