datasetRootPath = 'D:\yunfeng\Documents\Visual Studio 2012\Projects\iamge-understanding\Panorama\Panorama\pano1';
imageIdRange = 8:25;
imageIdCell = num2cell(imageIdRange);

for i = 1: size(imageIdCell,2) - 1
    img1 = strcat(datasetRootPath, '\jpg\pano1_00',sprintf('%02d',imageIdCell{i}), '.JPG');
    img2 = strcat(datasetRootPath, '\jpg\pano1_00',sprintf('%02d',imageIdCell{i+1}), '.JPG');
    Ia = imread(img1);
    Ib = imread(img2);
    Ia = single(rgb2gray(Ia));
    Ib = single(rgb2gray(Ib));
    [fa, da] = vl_sift(Ia);
    [fb, db] = vl_sift(Ib);
    da = normc(double(da));
    db = normc(double(db));
    [matches, scores] = vl_ubcmatch(da, db, 10);
    %normalize 
  
   matchFileName = strcat(datasetRootPath, '\match\match-', sprintf('%02d', imageIdCell{i}),'-', sprintf('%02d', imageIdCell{i+1}), 'txt');
   matchFileId = fopen(matchFileName, 'w');
   fprintf(matchFileId, '%d\n', size(matches, 2));
   fclose(matchFileId);
   mixData = [matches', scores'];
   
    dlmwrite(matchFileName, mixData, '-append', 'delimiter', ' ');

    
end
