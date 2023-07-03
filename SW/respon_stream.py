import requests
import yaml

class respon_stream:
    def __init__(self,
                 config_file="D:\main_code\city-yolov8\config_dataset\config-1\config-1.yaml"):
        with open(config_file, "r") as fr:
            self.data = yaml.load(fr, Loader=yaml.CLoader)

        self.http = self.data["nodemcu"]

        self.lines_name = "/lines"
        self.board_name = "/board?"

    def send_lines(self, data:dict):
        r = requests.get(self.http+self.lines_name, params=data)
    def send_board(self, data:str):
        r = requests.get(self.http+self.board_name+data)

if "__main__" == __name__:
    rs = respon_stream()
    """
    rs.send_board("warn")
    data = {
        "A1": "1",
        "A2": "1",
        "A3": "1",
        "A4": "1",
        "A5": "1",
        "A6": "1",
        "A7": "1",
        "A8": "1",
    }
    rs.send_lines(data)
    """
