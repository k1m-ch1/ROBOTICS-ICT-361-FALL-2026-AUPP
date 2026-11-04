First of all, we must create a new note book and connect to the instance with the GPU.

# Importing dataset

Upload the dataset to your drive.

Next, run:

```python
from google.colab import drive
drive.mount('/content/drive')
```

Now locate your dataset and copy it to the `content` in the root directory.

Now we have to install the `ultralytics` library which I guess implements the yolo stuff.

```
pip install ultralytics
```

Now check:

```
yolo version
```

Now, to train it:

```
yolo detect train model=yolov8n.pt data=data.yaml imgsz=320 epochs=10 batch=16 device=0
```

`data.yaml` probably contains metadata on the dataset:

```
train: ../train/images
val: ../valid/images
test: ../test/images

nc: 3
names: ['greenbox', 'redbox', 'xparking']

roboflow:
  workspace: danish-cq5li
  project: wro-detection-47xs2
  version: 1
  license: CC BY 4.0
  url: https://universe.roboflow.com/danish-cq5li/wro-detection-47xs2/dataset/1
```

Each images has a very particular format:

```
redbox_995_jpg.rf.ec6688fd0b19d85e7c8657726e24a444.jpg
```

and its labels:

```
redbox_995_jpg.rf.ec6688fd0b19d85e7c8657726e24a444.txt
```

It seems like we essentially have the name, then dash, image number, then dash, image format or something.

Each image is of size `640 x 640`:

```
❯ exiftool ./redbox_10_jpg.rf.f993a66905520b09451c18eb461692d9.jpg 
ExifTool Version Number         : 13.55
File Name                       : redbox_10_jpg.rf.f993a66905520b09451c18eb461692d9.jpg
Directory                       : .
File Size                       : 19 kB
File Modification Date/Time     : 2026:10:28 14:49:25+07:00
File Access Date/Time           : 2026:11:04 15:52:18+07:00
File Inode Change Date/Time     : 2026:11:04 15:51:19+07:00
File Permissions                : -rw-rw-rw-
File Type                       : JPEG
File Type Extension             : jpg
MIME Type                       : image/jpeg
JFIF Version                    : 1.01
Resolution Unit                 : None
X Resolution                    : 1
Y Resolution                    : 1
Image Width                     : 640
Image Height                    : 640
Encoding Process                : Baseline DCT, Huffman coding
Bits Per Sample                 : 8
Color Components                : 3
Y Cb Cr Sub Sampling            : YCbCr4:2:0 (2 2)
Image Size                      : 640x640
Megapixels                      : 0.410
```

So it seems like we can use a smaller `imgsz` parameter (as long as it's divisible by 64).

The Epoch is just jargon for how many times we run through the dataset.

The batch is probably like the number of images we run through such that we do one update using some kind of gradient descent 

We have `best.pt` as our weights.

We now need to export it to another file format called `onnx`

```
yolo export model=best.pt format=onnx imgsz=320 opset=12 dynamic=False simplify=False nms=True
```
