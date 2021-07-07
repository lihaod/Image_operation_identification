function varargout = ensembleTrnTst(fileSet1,fileSet2,settings)
% Fields of settings:
% settings.mode = <1,2,3>;
% settings.trnRate = <[0,1]>;
% settings.randomPerm = ;
% settings.isShuffle = <ture,false>;
% settings.verbose = <0,1,2,3>;
% settings.saveModel
% settings.saveModelPath
% settings.saveResult
% settings.saveResultPath
% settings.preprocessing.fun = @
% settings.preprocessing.arg = ;
% settings.feaName = 'F';
% ------------------------------------
% change log:
% 2014/10/13 add settings.feaName to cope with customized fea name in file.
% 2014/10/12 add settings.preprocessing to synchronize features (for srm).
% 2014/10/05 initial version.

if nargin < 3
    settings = struct;
end
settings  = parseSettings(settings);
switch settings.mode
    case 1
        nargoutchk(0,1);
    case 2
        nargoutchk(0,2);
    case 3
        nargoutchk(0,4);
end
[fileSet1, fileSet2] = parseFilePair(fileSet1,fileSet2,settings);

switch settings.mode
    case {1,3}
        trained_ensemble_cell = cell(size(fileSet1,1),size(fileSet2,2));
        tst_results_cover = cell(size(fileSet1,1),size(fileSet2,2));
        tst_results_stego = cell(size(fileSet1,1),size(fileSet2,2));
        tst_results_rate = cell(size(fileSet1,1),size(fileSet2,2));
        for row = 1:size(fileSet1,1)
            for col = 1:size(fileSet2,2)
                if isempty(fileSet2{row,col})
                    continue;
                end
                if strcmp(fileSet1{row},fileSet2{row,col})
                    warning('Skipping for two files with the same name.');
                    continue;
                end
                
                cover = load(fileSet1{row});
                stego = load(fileSet2{row,col});
                
                if isfield(settings,'preprocessing')
                    cover.(settings.feaName) = settings.preprocessing.fun(cover.(settings.feaName),settings.preprocessing.arg{:});
                    stego.(settings.feaName) = settings.preprocessing.fun(stego.(settings.feaName),settings.preprocessing.arg{:});
                end
                
                coverName = ['inst_' getName(fileSet1{row})];
                stegoName = ['inst_' getName(fileSet2{row,col})];
                modelName = ['model_' getName(fileSet1{row}) '_vs_' getName(fileSet2{row,col})];
                
                
                % Division into training/testing set (preserving pairs)
                if isfield(settings,'randomPerm')
                    random_permutation = settings.randomPerm;
                    if size(cover.(settings.feaName),1)~=length(random_permutation);
                        error('"random permutation" and "cover.(settings.feaName)" are with different lengths.');
                    end                    
                else
                    % RandStream.setGlobalStream(RandStream('mt19937ar','Seed',1));
                    random_permutation = randperm(size(cover.(settings.feaName),1));
                end
                training_set = random_permutation(1:round(size(cover.(settings.feaName),1)*settings.trnRate));
                testing_set = random_permutation(round(size(cover.(settings.feaName),1)*settings.trnRate)+1:end);
                
                % Prepare training features
                TRN_cover = cover.(settings.feaName)(training_set,:);
                TRN_stego = stego.(settings.feaName)(training_set,:);
                
                % Prepare testing features
                TST_cover = cover.(settings.feaName)(testing_set,:);
                TST_stego = stego.(settings.feaName)(testing_set,:);
                
                clear cover stego
                
                % Train ensemble
                ensembleTrnSettings.verbose = settings.verbose;
                if ensembleTrnSettings.verbose > 2; ensembleTrnSettings.verbose=0; end
                [trained_ensemble,trn_results] = ensemble_training(TRN_cover,TRN_stego,ensembleTrnSettings);
                trained_ensemble_cell{row,col} = trained_ensemble;
                if settings.saveModel; save(fullfile(settings.saveModelPath, [modelName '.mat']),'trained_ensemble','trn_results'); end
                if settings.verbose ~= 0
                    fprintf([coverName ' - ' stegoName ':  Eoob: %.4f\n'],trn_results.optimal_OOB);
                end
                
                if settings.mode == 3
                    % Testing phase. Predictions: -1 stands for cover, +1 for stego
                    tst_results = ensemble_testing(TST_cover,trained_ensemble);
                    tst_results_cover{row,col} = tst_results;
                    false_alarms = sum(tst_results.predictions~=-1);
                    if settings.saveResult; save(fullfile(settings.saveResultPath, [coverName '_' modelName '.mat']),'tst_results'); end
                    
                    tst_results = ensemble_testing(TST_stego,trained_ensemble);
                    tst_results_stego{row,col} = tst_results;
                    missed_detections = sum(tst_results.predictions~=+1);
                    if settings.saveResult; save(fullfile(settings.saveResultPath, [stegoName '_' modelName '.mat']),'tst_results'); end
                    
                    tst_results_rate{row,col} = [false_alarms/size(TST_cover,1),missed_detections/size(TST_stego,1),...
                                (false_alarms + missed_detections)/(size(TST_cover,1)+size(TST_stego,1))];
                    if settings.verbose ~= 0
                        fprintf('\b  err: %.4f %.4f %.4f  acc: %.4f\n',...
                            false_alarms/size(TST_cover,1),missed_detections/size(TST_stego,1),...
                            (false_alarms + missed_detections)/(size(TST_cover,1)+size(TST_stego,1)),...
                            1-(false_alarms + missed_detections)/(size(TST_cover,1)+size(TST_stego,1)));
                    end
                end
            end
        end
        if nargout == 0; varargout={}; end
        if nargout >= 1; varargout{1} = trained_ensemble_cell; end
        if nargout >= 2; varargout{2} = tst_results_cover; end
        if nargout >= 3; varargout{3} = tst_results_stego; end
        if nargout >= 4; varargout{4} = tst_results_rate; end
    case 2
        tst_results_inst = cell(size(fileSet1,1),size(fileSet2,2));
        tst_results_rate = cell(size(fileSet1,1),size(fileSet2,2));
        for row = 1:size(fileSet1,1)
            for col = 1:size(fileSet2,2)
                if isempty(fileSet2{row,col})
                    continue;
                end
                
                model = load(fileSet1{row});
                inst = load(fileSet2{row,col});
                
                if isfield(settings,'preprocessing')
                    inst.(settings.feaName) = settings.preprocessing.fun(inst.(settings.feaName),settings.preprocessing.arg{:});
                end
                
                modelName = getName(fileSet1{row});
                instName = ['inst_' getName(fileSet2{row,col})];
                
                
                % Division into testing set
                if isfield(settings,'randomPerm')
                    random_permutation = settings.randomPerm;
                    if size(inst.(settings.feaName),1)~=length(random_permutation);
                        error('"random permutation" and "cover.(settings.feaName)" are with different lengths.');
                    end                    
                else
                    % RandStream.setGlobalStream(RandStream('mt19937ar','Seed',1));
                    random_permutation = randperm(size(inst.(settings.feaName),1));
                end
                testing_set = random_permutation(round(size(inst.(settings.feaName),1)*settings.trnRate)+1:end);
                
                % Prepare testing features
                TST_inst = inst.(settings.feaName)(testing_set,:);
                
                clear inst
                
                % Testing phase
                tst_results = ensemble_testing(TST_inst,model.trained_ensemble);
                tst_results_inst{row,col} = tst_results;
                if settings.saveResult; save(fullfile(settings.saveResultPath, [instName '_' modelName '.mat']),'tst_results'); end
                
                % Predictions: -1 stands for cover, +1 for stego
                negative = sum(tst_results.predictions==-1);
                positive = sum(tst_results.predictions==+1);
                tst_results_rate{row,col} = [negative,positive]/size(TST_inst,1);
                if settings.verbose ~= 0
                    fprintf([instName ' - P_negative: %.4f  P_positive: %.4f\n'],...
                        negative/size(TST_inst,1),positive/size(TST_inst,1));
                end
            end
        end
        if nargout == 0; varargout={}; end
        if nargout >= 1; varargout{1} = tst_results_inst; end
        if nargout >= 2; varargout{2} = tst_results_rate; end
end
%--------------------------------------------------------------------------

function namestr = getName(fullfileSet)

% inx = find(fullfileSet=='\');
% if isempty(inx); inx = 0; end
% namestr = fullfileSet(inx(end)+1:end-4);
[~,namestr,~] = fileparts(fullfileSet);
%--------------------------------------------------------------------------

function [fileSet1, fileSet2] = parseFilePair(fileSet1,fileSet2,settings)

[R,C] = size(fileSet1);
if C>1 && R>1
    error('fileSet1 must be a vector.');
elseif R < C
    fileSet1 = transpose(fileSet1);
    [R,C] = size(fileSet1);
end
switch settings.mode
    case {1,3} % % training is included
        if isempty(fileSet2)
            fileSet2 = cell(R,R);
            for k = 1:R
                fileSet2(k,k+1:R) = fileSet1(k+1:R);
            end
        elseif settings.isShuffle && isvector(fileSet2)
            fileSet2 = repmat(transpose(fileSet2(:)),R,1);
        elseif size(fileSet2,1)~=R
            error('fileSet2 must has the same rows as fileSet1.');
        end
    case 2 % testing only
        if settings.isShuffle && isvector(fileSet2)
            fileSet2 = repmat(transpose(fileSet2(:)),R,1);
        elseif size(fileSet2,1)~=R
            error('fileSet2 must has the same rows as fileSet1.');
        end        
end
            
%--------------------------------------------------------------------------

function settings = parseSettings(settings)

if ~isfield(settings,'mode')
    settings.mode = 3;
end

if ~any(settings.mode == [1 2 3])
    error('Invalid mode.');
end

if ~isfield(settings,'trnRate')
    settings.trnRate = 0.5;
end

if ~isfield(settings,'saveModel')
    settings.saveModel = true;
end

if ~isfield(settings,'saveModelPath')
    settings.saveModelPath = '.\';
end

if ~isfield(settings,'saveResult')
    settings.saveResult = true;
end

if ~isfield(settings,'saveResultPath')
    settings.saveResultPath = '.\';
end

if ~isfield(settings,'isShuffle')
    settings.isShuffle = false;
end

if ~isfield(settings,'verbose')
    settings.verbose = 1;
    % = 0 ... no screen output
    % = 1 ... full screen output
    % = 2 ... screen output of only the last row (results)
end

if ~isfield(settings,'feaName')
    settings.feaName = 'F';
end




