function acc = train_test_binary(fileSet1,fileSet2, times)
% fileSet1 - the path of the orginal image feature mat file
% fileSet2 - the path of the processed image feature mat file.

fileSet2 = repmat(fileSet2,1,times);
fileSet1 = repmat(fileSet1,size(fileSet2,1),1);
        
settings.mode = 3;
settings.trnRate = 0.5;
settings.isShuffle = true;
settings.verbose = 3;
settings.saveModel = false;
settings.saveResult = false;

[~,~,~,rate] = ensembleTrnTst(fileSet1,fileSet2,settings);
err = cell2mat(rate);
err = err(:,3:3:end);
acc = (1-mean(err,2))';