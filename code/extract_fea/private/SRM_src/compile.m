% compile mex
clear all
outputName = 'SRM';
debugFlag = 0;

try
    delete([outputName '.' mexext]);
    delete([outputName '.' mexext '.pdb']);
catch exception
    rethrow(exception)
end

if debugFlag
    mex('-O', '-g', '-largeArrayDims', '-output', outputName, ...
        'SRM_matlab.cpp', 'SRMclass.cpp', 'submodel.cpp', 's.cpp', 'image.cpp', 'exception.cpp', 'config.cpp', ...
        '-I../include', ['-D' computer]);
else
    mex('-O', '-largeArrayDims', '-output', outputName, ...
        'SRM_matlab.cpp', 'SRMclass.cpp', 'submodel.cpp', 's.cpp', 'image.cpp', 'exception.cpp', 'config.cpp', ...
        '-I../include', ['-D' computer]);
end

