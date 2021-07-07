function Fea = srm(I,varargin)

if ischar(I); I = imread(I); end
fun = @SRM;
if nargin==1
    F1 = fun(I);
else
    F1 = fun(I,varargin{:});
end


Fea = cell2mat(transpose(struct2cell(F1)));