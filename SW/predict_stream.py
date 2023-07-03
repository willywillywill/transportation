import cv2
from ultralytics import YOLO
import numpy as np
import yaml
from io import BytesIO
import requests


class predict_stream:
    def __init__(self,
                 config_file="D:\main_code\city-yolov8\config_dataset\config-1\config-1.yaml",
                 show_stream=False,
                 ):
        self.is_open_camerea = {"camera1":False, "camera2":False}
        self.config_file = config_file
        self.show_stream = show_stream
        self.return_predict_dataset = {"camera1":[], "camera2":[]}

        with open(self.config_file, "r") as fr:
            self.data = yaml.load(fr, Loader=yaml.CLoader)

        self.masks_data = {}
        for i in range(1,3):
            with open(self.data["config_masks_camera%d"%(i)], "r") as fr:
                self.masks_data["camera%d"%(i)] = yaml.load(fr, Loader=yaml.CLoader)

        self.mask_name = self.data["obj"]
        self.model = YOLO(self.data["model"])

    def show(self, name, frame):
        if self.show_stream:
            cv2.imshow(name, frame)

    def run(self, camera_num):
        try:
            res = requests.get(self.data["camera%s"%(camera_num)], stream=True)
        except Exception as e:
            print(str(e))
            print("!! - close camera and after open - !!")
            exit(0)

        self.is_open_camerea["camera%s"%(camera_num)] = True

        for chunk in res.iter_content(chunk_size=120000):
            if len(chunk) > 100:
                try:
                    frame_data = BytesIO(chunk)
                    frame = cv2.imdecode(np.frombuffer(frame_data.read(), np.uint8), 1)
                    frame = cv2.rotate(frame, cv2.ROTATE_90_CLOCKWISE)
                except Exception as e:
                    print(str(e))
                    continue
            else:
                continue

            results = self.model.predict(frame)
            results_set = {}
            for result in results:
                results_set["confs"] = result.boxes.conf.numpy()
                results_set["names"] = result.boxes.cls.numpy().astype(int)
                results_set["boxes"] = result.boxes.xyxy.numpy().astype(int)

            masks_data_key = self.masks_data["camera%d"%(camera_num)].keys()

            pts_masks_data_set = {}
            for mask_key in masks_data_key:
                pts_mask_z = np.zeros(np.shape(frame))
                pts_mask = np.array(self.masks_data["camera%d"%(camera_num)][mask_key], np.int32)

                z = np.zeros(np.shape(frame), np.uint8)
                cv2.fillPoly(z, [pts_mask], color=(0, 255, 255))
                frame = cv2.addWeighted(frame, 1.0, z, 0.3, 1)

                cv2.fillPoly(pts_mask_z, [pts_mask], color=(0, 255, 255))
                pts_masks_data_set[mask_key] = pts_mask_z

            for box, name, conf in zip(results_set["boxes"], results_set["names"], results_set["confs"]):

                if conf < 0.5: continue

                pts_box_z = np.zeros(np.shape(frame))

                c = int((box[2]-box[0]) / 2)
                xy = (box[0]+c,box[3])
                dy = box[3] - box[1]
                dy = int(dy/5)

                dx = box[2] - box[0]
                dx = int(dx/3)

                z = np.zeros(np.shape(frame), np.uint8)
                cv2.rectangle(z, (xy[0]-dx, xy[1]-dy), (xy[0]+dx, xy[1]), (255, 255, 0), -1)
                frame = cv2.addWeighted(frame, 1.0, z, 0.3, 1)
                cv2.rectangle(pts_box_z, (xy[0]-dx, xy[1]-dy), (xy[0]+dx, xy[1]), (255, 255, 0), -1)

                #self.show(self.mask_name[name] + " camera%s" % (camera_num), pts_box_z)

                for mask_key in masks_data_key:
                    k = cv2.bitwise_and(pts_masks_data_set[mask_key], pts_box_z)
                    th1 = k.astype(np.uint8)
                    #self.show(mask_key + "camera%s th1" % (camera_num), th1)
                    th1 = cv2.cvtColor(th1, cv2.COLOR_BGR2GRAY)
                    b = th1.flatten()

                    if max(b) > 5:
                        self.return_predict_dataset["camera%s"%(camera_num)].append({"obj":self.mask_name[name], "mask":mask_key})

            self.show("f2"+"camera%s"%(camera_num), frame)

            if cv2.waitKey(1) & 0xFF == ord("q"):
                break

        self.is_open_camerea["camera%s"%(camera_num)] = False
        cv2.destroyAllWindows()
        exit(0)

    def return_isOpenCamera(self):
        return self.is_open_camerea

    def return_predict(self,camera_num):
        assert camera_num, "camera_num err"

        data = self.return_predict_dataset["camera%s"%(camera_num)]
        self.return_predict_dataset["camera%s"%(camera_num)] = []
        return data



if "__main__" == __name__:
    ps = predict_stream(show_stream=True)
    ps.run(camera_num=2)

