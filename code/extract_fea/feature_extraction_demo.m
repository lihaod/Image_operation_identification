function F = feature_extraction_demo(I)

F = struct();
extractor = {
    'ce', @ce;
    'cebf', @cebf;
    'ar', @(a) ar(a,10);
    'glf', @(a) glf(a,10,3,2);
    'ppi', @(a) ppi(a,26);
    'jpa', @jpa;
    'mmf', @mmf;
    'lbp', @lbp_shi;
    'srm', @srm;
    'srm_t1_o3_qo', @srm_T1_O3_qo;
    'tv', @tv;
    'lai_1', @lai_1;
    'lai_2', @lai_2;
    'zeng', @zeng;
    };

for i = 1:size(extractor,1)
    fprintf('Extracting %s feature...\n', extractor{i,1});
    F.(extractor{i,1}) = extractor{i,2}(I);
end