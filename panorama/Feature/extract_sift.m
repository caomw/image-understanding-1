datasetRootPath = 'D:\yunfeng\Documents\Visual Studio 2012\Projects\iamge-understanding\Panorama\Panorama\pano1';
imageIdRange = 8:25;
imageIdCell = num2cell(imageIdRange);

for i = 1: size(imageIdCell,2)
    jpgFileName = strcat(datasetRootPath, '\jpg\pano1_00',sprintf('%02d',imageIdCell{i}), '.JPG');
    I = imread(jpgFileName);
    I = single(rgb2gray(I));
    [f, d] = vl_sift(I);

    %normalize 
  
    d = normc(double(d));
    [dimOfFeat,numOfFeat]  = size(d);
    
    %write out metedata
    siftFileName = strcat(datasetRootPath, '\sift\pano1_00', sprintf('%02d', imageIdCell{i}), '.f');
    siftFileId = fopen(siftFileName, 'w');
    fprintf(siftFileId, '%d\n', numOfFeat);
    fprintf(siftFileId, '%d\n', dimOfFeat);
    fclose(siftFileId);
    
    %write out parameters and data
    para_data = [f', d'];
    dlmwrite(siftFileName, para_data, '-append', 'delimiter', ' ');
    
end
