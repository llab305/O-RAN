
import time
import json,os
from ricxappframe.xapp_frame import Xapp

os.environ['RMR_SEED_RT'] = 'test_route.rt'
os.environ['RMR_LOG_VLEVEL'] = '0'
os.environ['RMR_RTG_SVC']='-1'

def entry(self):
    my_ns = "myxapp"
    number = 0
    while True:
        number+=1
        jpay={'SocketFD': 7, 'Procedure Code': 5, 'RIC Request ID': {'RIC Requestor ID': 8000, 'RIC Instance ID': 1}, 'RAN Function ID': 1, 'RIC Action ID': number, 'RIC Indication Type': 0, 'RIC Indication Header': 1, 'RIC Indication Message': '{"BLER":{"UE1":0.0048,"UE2":0.01,"UE3":0.6,"UE4":0.12,"UE5":0.16,},"SINR":{"UE1":16.238,"UE2":15.278,"UE3":2.542,"UE4":9.667,"UE5":8.852,},"Latency":{"UE1":11.0,"UE2":5.0,"UE3":20.0,"UE4":9.0,"UE5":17.0,},"Throughput":768.0,"DL or UL":UL}'}
        # rmr send to default handler
        self.rmr_send(json.dumps(jpay).encode(), 7004200)
        print(number)
        """
        # rmr send 60000, should trigger registered callback
        val = json.dumps({"test_send": number}).encode()
        self.rmr_send(val, 60000)
        number += 1
        """
        # rmr receive
        for (summary, sbuf) in self.rmr_get_messages():
            # summary is a dict that contains bytes so we can't use json.dumps on it
            # so we have no good way to turn this into a string to use the logger unfortunately
            # print is more "verbose" than the ric logger
            # if you try to log this you will get: TypeError: Object of type bytes is not JSON serializable
            print("ping: {0}".format(summary))
            self.rmr_free(sbuf)

        time.sleep(1)


xapp = Xapp(entrypoint=entry, rmr_port=4564, use_fake_sdl=True)
xapp.run()
