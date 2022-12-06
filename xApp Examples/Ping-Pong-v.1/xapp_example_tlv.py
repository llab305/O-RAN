# xApp Version 1.0
# © 2021 III All Rights Reserved

import logging,json,os,sys, threading
from ricxappframe.xapp_frame import Xapp
from xapp.xapp_api3 import xApp
import time
import gc
import tracemalloc


fileName=sys.argv[0]
fileName=os.path.splitext(fileName)[0]


xapp_port=8003
xAppAPI = xApp(fileName,xapp_port)
#xAppAPI.initials(fileName)
#xapp_port= xAppAPI.get_xApp_port()

def added_thread_job():
  print('新增加的執行續: ', threading.current_thread())
  print('新增加的執行續名稱: ', threading.current_thread().name)
  print('活動中的執行續數量: ', threading.active_count())
#send to other component
def entry(self):
    global xapp_port,check_in
    
    print("*******************************\n")
    print("xApp is ready for works!!!!!\n")
    print("*******************************\n")
    logging.info("xApp is ready for works!!!!!\n")
    
    
    

    #tracemalloc.start()
    flg=True
    Ask = xAppAPI.do_xApp_initial(xapp_port)
    #xAppAPI.createTimer()    
    self.rmr_send(Ask, xAppAPI.RT_REQUEST)
    report_period = time.time()
    control_before = time.time()
    #del Ask
    #gc.collect()
    #print("time = ",i)
    time.sleep(2)
    """
    snapshot = tracemalloc.take_snapshot()
    top_stats = snapshot.statistics('lineno')

    for stat in top_stats:
        print(stat)

    tracemalloc.stop()
    """
    #report_flag = 0
    #sub_flag = 0
        
    while True:
        
        # rmr receive
        for (summary, sbuf) in self.rmr_get_messages():

            #tracemalloc.start()
            message= xAppAPI.get_mtype(summary)
            jpay = xAppAPI.get_payload(summary)
            s=sys.getsizeof(summary)
            #s2=sys.getsizeof(jpay)
            print("message:",summary)
            #print("jpay:",jpay)
            #print("message memory is:",s)
            #print("payload memory is:",s2)
            
            #Routing Ack
            if message == xAppAPI.RT_RESPONSE:
                val = xAppAPI.sub_request()
                #print("inital response val:",val)
                if val != None:
                    self.rmr_send(val, xAppAPI.SUB_REQUEST)
                #sub_flag = 1
            
            #Subscription Response
            if message== xAppAPI.SUB_RESPONSE:
                print("Recieved Subscription Response\n")
                logging.info("Recieved Subscription Response")
                print("Subscription Response Payload:\n",jpay,"\n")
                SocketFD = jpay['SocketFD']
                #sub_flag = 1

            #Subscription Failure
            if message== xAppAPI.SUB_FAILURE:
                SocketFD = xAppAPI.get_SocketFD
                val = xAppAPI.report_query()
                self.rmr_send(val, xAppAPI.REPORT_QUERY_REQUEST)

            if message== xAppAPI.SUB_DELETE_RESPONSE:
                print("Recieved Subscription Delete Response\n")
                print("Subscription Delete Response Payload:\n",jpay,"\n")
                val = xAppAPI.report_query()
                self.rmr_send(val, xAppAPI.REPORT_QUERY_REQUEST)

            #Control Response
            if message== xAppAPI.CTL_RESPONSE:
                print("Recieved Control Response\n")
                #print("Control Response Payload:\n",jpay,"\n")
                flg=True
                control_ack = time.time()
                print("control ACK interval: ", control_ack - control_start)

            #Indiction Report
            if message == xAppAPI.INDI_REPORT:
                #start_ = time.time()
                get_report_time = time.time()
                print("-------------------report-----------------------")
                print("Recieved Indication Report\nReport period: ", get_report_time - report_period)
                report_period = get_report_time
                #print("=== Recieved Indication Report ===\nIndication Report Payload:\n",jpay,"\n")
                """為將時間搓記註解"""
                #print("inbound time: ", start_ - jpay['Time'])
                if flg:
                    start_ = time.time()
                    datalist=[17,17,1,1,1,1,0]
                    controlmessage = json.dumps({
                        "RIC Request ID":{"RIC Requestor ID":xAppAPI.get_xAppPort(),"RIC Instance ID":1},
                        "RAN Function ID":1,
                        "RAN Function Revision":1,
                        "RIC Control Header":0x00,
                        "RIC Control Message":{'DL_MCS':datalist[0],#DL_MCS,
                                            'UL_MCS':datalist[1],#UL_MCS,
                                            'max DL HARQ':datalist[2],#DL_maxHARQ,
                                            'max UL HARQ':datalist[3],#UL_maxHARQ,
                                            'Repetition':float(datalist[4]),#float(repetition),
                                            'max Num UE':datalist[5],#max_Num_UE,
                                            'Grant-free Ind':datalist[6]#Grandfree
                        }, 
                        "RIC Control Ack Request":1,
                        "Procedure Code":4,
                        "SocketFD":xAppAPI.get_SocketFD()
                    }).encode()
                    #end_ = time.time()
                    #print("encode time: ", end_-start_)
                    #print("controlmessage:",controlmessage)


                    #controlmessage = xAppAPI.control_request(xapp_port,xAppAPI.get_SocketFD(), 1, 1, 1, 1, 1, 17, 17, 17, 17, 17, 1, 1, 1, 1, 1)
                    #controlmessage = xAppAPI.control_request(xapp_port,xAppAPI.get_SocketFD(),17,17,1,1,1,1,0)
                    #start_ = time.time()
                    self.rmr_send(controlmessage, 6104200)
                    #end_ = time.time()
                    control_start = time.time()
                    print("control peroid: ", control_start - control_before)
                    control_before = control_start
                    #print("outbound time: ", end_-start_)
                    flg=False
                    #added_thread_job()
                #report_flag = 1
                #SocketFD = xAppAPI.get_SocketFD
                #val = xAppAPI.report_query()
                #self.rmr_send(val, xAppAPI.REPORT_QUERY_REQUEST)

            #Indiction insert
            if message == xAppAPI.INDI_INSERT:
                print("Recieved Indication Insert\n")
                print("Indication Insert Payload:",jpay,"\n")
                
                DelMessage = xAppAPI.sub_del_request()
                self.rmr_send(DelMessage, xAppAPI.SUB_DELETE_REQUEST)
                #control_request(self, summary, sbuf)

            if message == xAppAPI.REPORT_QUERY_RESPONSE:
                print("Recieved Report Query Response\n")
                print("Report Query Payload:",len(str(jpay)),"\n")

            """
            snapshot = tracemalloc.take_snapshot()
            top_stats = snapshot.statistics('lineno')

            for stat in top_stats:
                print(stat)

            tracemalloc.stop()
            """
            self.rmr_free(sbuf)
        """
        #print(report_flag)
        if report_flag == 1:
            break
        if sub_flag == 1:
            break
        """

        

xapp = Xapp(entrypoint=entry, rmr_port=xapp_port)
xapp.run()

