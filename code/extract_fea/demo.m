%%
clear
clc
addpath('jpegtbx\', 'utilis\');
I = imread('00002.pgm');
F = feature_extraction_demo(I);