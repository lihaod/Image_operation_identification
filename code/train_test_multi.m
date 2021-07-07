function [meanConfuseMat,acc] = train_test_multi(feamatList, modelPath, resultPath, confuseMatName, times, instNum)

L = length(feamatList);

jm = parcluster();

randp = zeros(instNum,times);
for k = 1:times
randp(:,k) = randperm(instNum);
end
save(['randp.mat'],'randp');
load('randp.mat','randp');

settings.trnRate = 0.5;
settings.verbose = 0;

%%
disp('training...')

for k = 1:times        
    settings.mode = 1;
    settings.randomPerm = randp(:,k);
    settings.saveModel = true;
    settings.saveModelPath = [modelPath num2str(k)];
    settings.saveResult = false;
    mkdir(settings.saveModelPath);
    if isunix; cmdStatus = system(['chmod a+w ' settings.saveModelPath]); end
    
    job = createJob(jm);
    for i = 1:L
        for j = i+1:L
            tasks = createTask(job, @ensembleTrnTst, 0, {feamatList(i),feamatList(j),settings});
        end
    end
    submit(job)
    wait(job, 'finished')
end

%%
disp('testing...')
modelSet = cell(L*(L-1)/2,1);
confuseMat = zeros(L,L,times);
rsta = zeros(L,L*(L-1)/2,round(instNum*(1-settings.trnRate)));
for k = 1:times    
    settings.mode = 2;
    settings.randomPerm = randp(:,k);
    settings.saveModel = false;
    settings.saveResultPath = [resultPath num2str(k)];
    settings.saveResult = true;
    mkdir(settings.saveResultPath);
    if isunix; cmdStatus = system(['chmod a+w ' settings.saveResultPath]); end
    
    count = 1;
    for i = 1:L
        for j = i+1:L
            [~,namei,~] = fileparts(feamatList{i});
            [~,namej,~] = fileparts(feamatList{j});
            modelSet{count} = [modelPath num2str(k) '/' 'model_' namei '_vs_' namej '.mat'];
            count = count+1;
        end
    end
    
    for i = 1:L
        job = createJob(jm);
        for j = 1:length(modelSet)
            tasks = createTask(job, @ensembleTrnTst, 1, {modelSet(j),feamatList(i),settings});
        end
        submit(job)
        wait(job, 'finished')
        tst_results = fetchOutputs(job);
        for j = 1:length(tst_results)
            rsta(i,j,:) = tst_results{j}{1}.predictions;
        end
    end
    count = 1;
    for i = 1:L
        for j = i+1:L
            rsta(:,count,:) = rsta(:,count,:)/2*(j-i)+(j+i)/2;
            count = count+1;
        end
    end
    rstb = permute(mode(rsta,2),[1,3,2]);
    for i = 1:L
        for j = 1:L
            confuseMat(i,j,k) = sum(rstb(i,:)==j)/numel(rstb(i,:));
        end
    end
    
end
meanConfuseMat = mean(confuseMat,3);
acc = mean(diag(mean(confuseMat,3)));
save(confuseMatName,'confuseMat')