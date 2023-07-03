import requests
import time
import threading

data1 = {
    "A1":"1",
    "A2":"1",
    "A3":"1",
    "A4":"1",
    "A5":"1",
    "A6":"1",
    "A7":"1",
    "A8":"1",
}

data0 = {
    "A1":"0",
    "A2":"0",
    "A3":"0",
    "A4":"0",
    "A5":"0",
    "A6":"0",
    "A7":"0",
    "A8":"0",
}



"""
requests.get("http://192.168.10.120/lines", params=data1)
"""


r = requests.get("http://192.168.10.120/board?s")
#r = requests.get("http://192.168.10.120/board?warn")
"""
r = requests.get("http://192.168.10.120/board?safe")
"""



    





    


    
    


