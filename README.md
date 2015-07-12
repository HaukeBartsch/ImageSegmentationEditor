# ImageSegmentationEditor

Program for manual segmentation of volumetric images with support for color (DEC)

The Image Segmentation Editor allows you to draw on volumetric images loaded as either mgz or DICOM. Be aware that it is a very
crude tool. Development was stopped after it became tolerably bad judged by people with a high pain threshold. It is modeled after
the Amira Segmentation Editor (best segmentation tool ever) and can do one thing more (manually segment color images) and everything
else less. Each region outlined by the user can be assigned a label name and a color. A list of these labels is stored in a label
file (mgh format with a secondary json file) and can be loaded at a later time.

![screenshot of the application](https://raw.github.com/HaukeBartsch/ImageSegmentationEditor/master/images/screenshot.png)
