# O-RAN
## Message Type Definition

### E2 Procesure
```c
//E2 Setup 200~299

#define E2_Request 200
#define E2_Response 201
#define E2_Failure 204


//E2 Reset 300~399

#define Reset_Request 300
#define Reset_Response 301
#define Reset_Failure 304


//Subscription 400~499

//Subscription Manager <--> E2 Termination

#define Sub_Request 400
#define Sub_Response 401
#define Sub_Failure 404

//xApp<---> Subscription Manager

#define Sub_Request 410
#define Sub_Response 411
#define Sub_Failure 414


//Subscription Delete 500~519

//Subscription Manager <--> E2 Termination

#define Sub_Del_Request 500
#define Sub_Del_Response 501
#define Sub_Del_Failure 504

//xApp<---> Subscription Manager

#define Sub_Del_Request 510
#define Sub_Del_Response 511
#define Sub_Del_Failure 514


//Control 600~699

//E2 Manager <--> E2 Termination

#define Ctrl_Request 600
#define Ctrl_Response 601
#define Ctrl_Failure 604

//xApp <-->E2 Manager

#define Ctrl_Request 610
#define Ctrl_Response 611
#define Ctrl_Failure 614


//Indication(Report) 700

#define Indication_Report 700

//Indication(Insert) 710

#define Indication_Insert 710

//Error E2 Node Disconnected 720

#define E2_Node_Disconnect 720

//Error Indication 740

#define Indication_Error 740




//Service Update 800

#define Serv_Update_Request 800
#define Serv_Update_Response 801
#define Serv_Update_Failure 804

//Service Query 810

#define Serv_Query 810

//Query E2 Setup Table
#define Indication_Query_Request 820
#define Indication_Query_Response 821
#define Indication_Query_Failure 824

//Query Subscription Table
#define Indication_Query_Request 840
#define Indication_Query_Response 841
#define Indication_Query_Failure 844

//Query Indication Report Table
#define Indication_Query_Request 870
#define Indication_Query_Response 871
#define Indication_Query_Failure 874


//xApp <---> Routing Manager 900~999

#define RT_Request 900
#define RT_Response 901

//Routing Manager send the broadcast to all 1000

#define Broadcast 1000

//xApp Manager <---> Routing Manager 1100~1199

#define ASK_port_Request 1100
#define ASK_port_Response 1101


```

### xApp / For Database  (6000~6999)
```c
//(DB) xApp E2 setup

#define DB_E2_Request 6200
#define DB_E2_Response 6201
#define DB_E2_Failure 6204

//(DB) xApp E2 Reset

#define DB_Reset_Request 6300
#define DB_Reset_Response 6301
#define DB_Reset_Failure 6304

//(DB) xApp Subscription

#define DB_Sub_Request 6400
#define DB_Sub_Response 6401
#define DB_Sub_Failure 6404

//(DB) xApp Subscription Delete

#define DB_Sub_Del_Request 6500
#define DB_Sub_Del_Response 6501
#define DB_Sub_Del_Failure 6504

//(DB) xApp Control

#define DB_Ctrl_Request 6600
#define DB_Ctrl_Response 6601
#define DB_Ctrl_Failure 6604

//(DB) xApp Indication(Report) 

#define DB_Indication_Report_Request 6700
#define DB_Indication_Report_Response 6701
#define DB_Indication_Report_Failure 6704

//(DB) xApp Indication(Insert) 

#define DB_Indication_Insert_Request 6710
#define DB_Indication_Insert_Response 6711
#define DB_Indication_Insert_Failure 6714

//(DB) xApp Error Indication

#define DB_Error_Request 6740
#define DB_Error_Response 6741
#define DB_Error_Failure 6744


```

## Sepcific Port Definition

```c
//Manager_port 4000~4999

#define E2_Termination 4000

#define E2_Mgr 4200

#define Subscription_Mgr 4400

#define Routing_Mgr 4600

#define xApp_Mgr 5000

//Database_port 6000~6999

#define Redis_Mgr_RIC 6000

#define Redis_Influx_DB 6200

#define Redis 6379

#define Redis_Mgr_Routing 6600

//xApp_port 8000~65535

#define Default_xApp 8000~65535

```
