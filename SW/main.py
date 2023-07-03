import predict_stream
import threading
import time
import respon_stream


def read_predict_return():
    time.sleep(3)

    while ps.return_isOpenCamera()["camera2"]:
        data = []
        lines_cmd = {
            "A1": "0",
            "A2": "0",
            "A3": "0",
            "A4": "0",
            "A5": "0",
            "A6": "0",
            "A7": "0",
            "A8": "0",
        }
        board_cmd = "safe"

        if data1 := ps.return_predict(1):
            data.append(data1)
        if data2 := ps.return_predict(2):
            data.append(data2)
        print(data)
        for i in data:

            if (i[0]["obj"] == "car") and ("crosswalk" in i[0]["mask"]):

                if i[0]["camera"] == 2:
                    lines_cmd["A5"] = "1"
                elif i[0]["camera"] == 1:
                    lines_cmd["A7"] = "1"

                board_cmd = "warn"

            if (i[0]["obj"] == "cat") and ("double-yellow-line" in i[0]["mask"]):
                if i[0]["camera"] == 2:
                    lines_cmd["A1"] = "1"
                elif i[0]["camera"] == 1:
                    lines_cmd["A3"] = "1"

                board_cmd = "warn"

        th_lines = threading.Thread(target=rs.send_lines, args=(lines_cmd,))
        th_board = threading.Thread(target=rs.send_board, args=(board_cmd,))
        th_lines.start()
        th_board.start()

        print(board_cmd)
        print(lines_cmd)


        time.sleep(1)

    exit(0)

print("run")

config_file = "D:\main_code\city-yolov8\config_dataset\config-1\config-1.yaml"

rs = respon_stream.respon_stream(config_file=config_file)
ps = predict_stream.predict_stream(
    config_file=config_file,
    show_stream=True
    )


th1 = threading.Thread(target=ps.run, args=(1,))
th2 = threading.Thread(target=ps.run, args=(2,))
th3 = threading.Thread(target=read_predict_return)



#th1.start()
th2.start()
th3.start()



