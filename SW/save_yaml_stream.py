import argparse
import cv2
import yaml
import requests
from io import BytesIO
import numpy as np


class Config:
    def __init__(self, name:str):
        self.name = name
        self.data = { self.name:[] }

    def cmd_xy(self, event, x, y, flags, userdata):
        if event == cv2.EVENT_LBUTTONDOWN:
            self.data[self.name].append([x,y])

    def output_config(self):
        return self.data

""" args
model  : 1 | 2
yaml   : file
camera : url
"""
def add_val(model, file, camera):
    name = "crosswalk" if model == "1" else "double-yellow-line"
    try:
        res = requests.get(camera, stream=True)
    except Exception as e:
        print(str(e))
        print("!! - close camera and after open - !!")
        exit(0)

    config = Config(name)

    for chunk in res.iter_content(chunk_size=120000):
        if len(chunk) > 100:
           try:
               frame_data = BytesIO(chunk)
               frame = cv2.imdecode(np.frombuffer(frame_data.read(), np.uint8), 1)
               frame = cv2.rotate(frame, cv2.ROTATE_90_CLOCKWISE)
           except Exception as e:
               print(str(e))
               continue
        else:   continue

        for xxyy in config.output_config()[name]:
            cv2.circle(frame, xxyy, 2, (0,0,255), 2)

        cv2.imshow("f", frame)
        cv2.setMouseCallback("f", config.cmd_xy)

        if cv2.waitKey(1) & 0xFF == ord("q"):
            assert len(config.output_config()[name]) > 2, "!:  len(data)>2  , your %s"%(len(config.output_config()[name]))

            m_n = 0

            with open(file, "r") as fr:
                data = yaml.load(fr, Loader=yaml.CLoader)

                if data is not None:
                    for i in data.keys():
                        if name in i:
                            n = i.split("_")
                            num = int(n[1])
                            m_n = num if num > m_n else m_n

            with open(file, "w") as fw:
                if data is None:
                    data = {}
                    data[name+"_"+str(m_n+1)] = config.output_config()[name]
                else:
                    new_data = {}
                    new_data[name + "_" + str(m_n + 1)] = config.output_config()[name]
                    data.update(new_data)

                yaml.dump(data, fw)

            print(name+"_"+str(m_n+1)+" it OK!!")
            break


    cv2.destroyAllWindows()
    exit(0)

""" args
yaml   : file
delete : name
"""
def del_val(name, file):
    with open(file, "r") as fr:
        data = yaml.load(fr, Loader=yaml.CLoader)

    assert name in data.keys(), "NOT VAL"

    del data[name]
    n = name.split("_")
    m = int(n[1])
    new_data = {}

    for i in data.keys():
        j = i.split("_")

        if not ((n[0] in i) & (m < int(j[1]))):
            new_data[i] = data[i]
        else:
            new_data[j[0] + "_" + str(int(j[1]) - 1)] = data[i]

    with open(file, "w") as fw:
        yaml.dump(new_data, fw)

    print("del "+name+" it OK!!")



if "__main__" == __name__:

    """ test
        model:    1     ("crosswalk")
        yaml:    "D:\main_code\city-yolov8\config_masks.yaml"
        camera:  "https://cctv.bote.gov.taipei:8501/mjpeg/243"
    """

    parser = argparse.ArgumentParser()
    parser.add_argument("--camera", help="http")
    parser.add_argument("--model", help="1:crosswalk | 2:double_yellow_line")
    parser.add_argument("--yaml")
    parser.add_argument("--delete")

    args = parser.parse_args()

    #assert args.model , "--model ,1:crosswalk | 2:double_yellow_line"
    #assert args.yaml  , "file"
    #assert args.camera, "http"

    if args.delete is not None:
        assert args.yaml, "file"

        del_val(args.delete, args.yaml)
    else:
        assert args.model, "--model ,1:crosswalk | 2:double_yellow_line"
        assert args.yaml  , "file"
        assert args.camera, "http"

        add_val(args.model, args.yaml, args.camera)

