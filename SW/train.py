from ultralytics import YOLO
import torch


model = YOLO('yolov8m.pt')  # build a new model from YAML

model.train(data=r"C:\Users\User\Desktop\school\YOLO_img\cat-car.v2i.yolov8\data.yaml", epochs=50, imgsz=640)
model.export(format="onnx")

