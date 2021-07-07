function fea = srm_T1_O3_qo(image)

config.T = int32(1);
config.order = int32(3);
% config.merge_spams = logical(true);
% config.symm_sign = logical(true);
% config.symm_reverse = logical(true);
% config.symm_minmax = logical(true);
% config.eraseLSB = logical(false);
% config.parity = logical(false);
% config.roundup5 = false;
% config.mapIsWeights = false;
config.quant_step.s1_minmax22h = 2;
config.quant_step.s1_minmax22v = 2;
config.quant_step.s1_minmax24 = 1;
config.quant_step.s1_minmax34 = 2;
config.quant_step.s1_minmax34h = 2;
config.quant_step.s1_minmax34v = 1;
config.quant_step.s1_minmax41 = 2;
config.quant_step.s1_minmax48h = 1;
config.quant_step.s1_minmax48v = 1;
config.quant_step.s1_minmax54 = 1;
config.quant_step.s1_spam14h = 3;
config.quant_step.s1_spam14v = 3;
config.quant_step.s2_minmax21 = 1;
config.quant_step.s2_minmax24h = 1;
config.quant_step.s2_minmax24v = 1;
config.quant_step.s2_minmax32 = 1;
config.quant_step.s2_minmax41 = 1;
config.quant_step.s2_spam12h = 1;
config.quant_step.s2_spam12v = 1;
config.quant_step.s3_minmax22h = 2;
config.quant_step.s3_minmax22v = 2;
config.quant_step.s3_minmax24 = 1;
config.quant_step.s3_minmax34 = 1;
config.quant_step.s3_minmax34h = 2;
config.quant_step.s3_minmax34v = 1;
config.quant_step.s3_minmax41 = 2;
config.quant_step.s3_minmax48h = 1;
config.quant_step.s3_minmax48v = 1;
config.quant_step.s3_minmax54 = 1;
config.quant_step.s3_spam14h = 1;
config.quant_step.s3_spam14v = 1;
config.quant_step.s3x3_minmax22h = 1;
config.quant_step.s3x3_minmax22v = 1;
config.quant_step.s3x3_minmax24 = 2;
config.quant_step.s3x3_minmax41 = 2;
config.quant_step.s3x3_spam11 = 1;
config.quant_step.s3x3_spam14h = 2;
config.quant_step.s3x3_spam14v = 2;
config.quant_step.s5x5_minmax22h = 1;
config.quant_step.s5x5_minmax22v = 1;
config.quant_step.s5x5_minmax24 = 2;
config.quant_step.s5x5_minmax41 = 1;
config.quant_step.s5x5_spam11 = 1;
config.quant_step.s5x5_spam14h = 2;
config.quant_step.s5x5_spam14v = 2;

F = SRM(image,config);

submodelnames = fieldnames(F);
names = {'s1_minmax22h', 's1_minmax22v', 's1_minmax24', 's1_minmax34',...
    's1_minmax34h', 's1_minmax34v', 's1_minmax41', 's1_minmax48h', 's1_minmax48v',...
    's1_minmax54', 's2_minmax21', 's2_minmax24h', 's2_minmax24v', 's2_minmax32',...
    's2_minmax41', 's3_minmax22h', 's3_minmax22v', 's3_minmax24', 's3_minmax34',...
    's3_minmax34h', 's3_minmax34v', 's3_minmax41', 's3_minmax48h', 's3_minmax48v',...
    's3_minmax54', 's3x3_minmax22h', 's3x3_minmax22v', 's3x3_minmax24', 's3x3_minmax41',...
    's5x5_minmax22h', 's5x5_minmax22v', 's5x5_minmax24', 's5x5_minmax41', 's1_spam14hv',...
    's2_spam12hv', 's3_spam14hv', 's35_spam11', 's3x3_spam14hv', 's5x5_spam14hv'};
Fs = cell2struct(cell(length(names),1),names,1);
for k = 1:length(submodelnames)
    Fs.(submodelnames{k}(1:strfind(submodelnames{k},'_q')-1)) = F.(submodelnames{k});
end

fea = cell2mat(transpose(struct2cell(Fs)));