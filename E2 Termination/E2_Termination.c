#include<stdio.h>
#include<pthread.h>
#include<string.h>
#include<sys/types.h>
#include<unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <error.h>
#include <sys/epoll.h>
#include <time.h>

#include "rmr.h"

#include "cJSON.h"
#include "BuildMessage.h"
#include "printMsg.h"

#define MAX_BUFFER 4096
#define MY_PORT_NUM 30860   /* This can be changed to suit the need and should be same in server and client */

typedef struct sctp_params {
    int  connSock[10];

} sctp_params_t;

pthread_t ntid;

void printids(const char *s)
{
    pid_t pid;
    pthread_t tid;

    pid = getpid();
    tid = pthread_self();
    printf("%s pid %u tid %u (0x%x)\n",s,(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);

}

void *thread(void *arg){

	printids("Thread for receive\n");
    int connSock =  *((int*) arg);
    int in, flags, ret;
    struct sctp_sndrcvinfo sndrcvinfo;
    int countE2Node = 0;

    uint8_t buffer[MAX_BUFFER + 1];
    uint8_t buffersend[MAX_BUFFER + 1];
    uint8_t bufferCh[MAX_BUFFER + 1];
	size_t bufferCh_size = MAX_BUFFER;

    int listenSock, i, len;
    struct sockaddr_in servaddr;

    int datalen = 0;
    int datasize = 0;

    void* mrc;						// msg router context
    //rmr_mbuf_t*	sbuf;				// send buffer
    //rmr_mbuf_t*	rbuf;				// received buffer
    char* listen_port = "43087";    // default to what has become the standard RMR port
    int	delay = 1000000;			// mu-sec delay between messages
    int mtype;
	
	//receive parameter
    int messagetype, message1;
	long E2NodeID0, E2NodeID1, E2NodeID2, E2NodeID3;
    long E2NodeID, RICControlStatus, RICRequestorID, RICInstanceID, Cause;
    long RANFunctionID, RANFunctionRevision, RICActionID, RICIndicationSN ,RICIndicationType;
	float bler0,bler1,bler2,bler3,bler[4];
	float BLER;
	long SINR, throughput[4], Throughput, latency[2], Latency;
	int senderror;
	char bufferinmessage[2048];

	double latency_value1, latency_value2, latency_value3;
	double bler_value1, bler_value2, bler_value3;
	double sinr_value1, sinr_value2, sinr_value3;
	
	pid_t childpid0,childpid1,childpid2,childpid3,childpid4,childpid5,childpid6,childpid7,childpid8,childpid9;
	sctp_params_t *sctpParams=(sctp_params_t *)arg;
	
	//------------------------------------------------------------------------------sctp--------------------------------------------------------------------------------//

    listenSock = socket (AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if(listenSock == -1)
    {
        printf("Failed to create socket\n");
        perror("socket()");
        exit(1);
    }

    printf("Socket id = %d\n",listenSock);
    bzero ((void *) &servaddr, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
    servaddr.sin_port = htons (MY_PORT_NUM);
    printf("server ip =%s\n",inet_ntoa(servaddr.sin_addr));//输出服务器ip地址
    memset(servaddr.sin_zero,0,8);

    int s;
    setsockopt(listenSock,SOL_SOCKET,SO_REUSEADDR,&s,sizeof(s));

    ret = bind (listenSock, (struct sockaddr *) &servaddr, sizeof (servaddr));

    if(ret == -1 )
    {
        printf("Bind failed \n");
        perror("bind()");
        close(listenSock);
        exit(1);
    }
	
	if(listen (listenSock, 10) == 0){
        printf("[+]Listening...\n");
    }else{
        printf("Error in binding.\n");
    }
	
	bzero (buffer, MAX_BUFFER + 1);
	
	if( (mrc = rmr_init( listen_port, 2048, RMRFL_NONE )) == NULL ) {
        fprintf( stderr, "<DEMO> unable to initialise RMR\n" );
        exit( 1 );
	}
	
	Connumber:do
	{
		sctpParams->connSock[countE2Node] = accept (listenSock, (struct sockaddr *) NULL,(int *) NULL);
		printf("connSock : %d\n",sctpParams->connSock[countE2Node]);
		if (sctpParams->connSock[countE2Node] == -1){
			printf("accept() failed\n");
			perror("accept()");
			close(sctpParams->connSock[countE2Node]);
		}
		else{
			printf ("New Message....\n");
			if (countE2Node == 0){
				if((childpid0 = fork()) == 0){

					LOOP0:do{
                        rmr_mbuf_t*	sbuf;				// send buffer
                        rmr_mbuf_t*	rbuf;				// received buffer
						bzero (buffer, MAX_BUFFER + 1);
						in = sctp_recvmsg (sctpParams->connSock[0], buffer, sizeof (buffer), (struct sockaddr *) NULL, 0, &sndrcvinfo, &flags);
						if( in == -1){
							printf("Error in sctp_recvmsg\n");
							perror("sctp_recvmsg()");
							close(sctpParams->connSock[0]);
							continue;
						}
						else if(in == 0){
							continue;
						}
						else{
							printf ("Length of Data received: %d\n", in);

							E2AP_PDU_t *ErrorInd = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));
							E2AP_PDU_t *testdc = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));
							asn_decode(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, (void **)&testdc, buffer, in);

							printf ("what message(ini or suc or unsuc): %d\n", testdc->present);//initiatingMessage=1 or successfulOutcome=2 or unsuccessfulOutcome=3
							
                            //initiatingMessage=1
                            if(testdc->present == 1){    
								if(testdc->choice.successfulOutcome->procedureCode == 1){
									//E2 Setup Request
									if(testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.count == 2){
										E2NodeID0 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[0];
										E2NodeID1 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[1];
										E2NodeID2 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[2];
										E2NodeID3 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[3];
										RANfunction_ItemIEs_t * RANfunction_ItemIEs = (RANfunction_ItemIEs_t * ) testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[1]->value.choice.RANfunctions_List.list.array[0];
										RANFunctionID = RANfunction_ItemIEs->value.choice.RANfunction_Item.ranFunctionID;
										RANFunctionRevision = RANfunction_ItemIEs->value.choice.RANfunction_Item.ranFunctionRevision;

										messagetype = testdc->choice.initiatingMessage->procedureCode;
										mtype = 2004200;
										message1 = 1;
                                        printf ("Receive [E2 Setup Request]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 5){
									//indication
									if(testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.count == 7){
										RICRequestorID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;				
										RICInstanceID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICActionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[2]->value.choice.RICactionID;
										RICIndicationSN = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[3]->value.choice.RICindicationSN;
										RICIndicationType = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[4]->value.choice.RICindicationType;
										bzero(bufferinmessage,2049);
										strcpy(bufferinmessage,testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[6]->value.choice.RICindicationMessage.buf);
										
										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 7004200;
										message1 = 2;
                                        printf ("Receive [Indication(Report) no. %ld]\n", RICIndicationSN);
									}
									//insert
									if(testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.count == 8){
										RICRequestorID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICActionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[2]->value.choice.RICactionID;
										RICIndicationSN = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[3]->value.choice.RICindicationSN;
										RICIndicationType = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[4]->value.choice.RICindicationType;

										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 7104200;
										message1 = 5;
                                        printf ("Receive [Indication(Insert) no. %ld]\n", RICIndicationSN);              
									}
								}
                                else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
							else if(testdc->present == 2){   //successfulOutcome=2
								if(testdc->choice.successfulOutcome->procedureCode == 8){
									//Subscription Response
									if(testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICaction_Admitted_ItemIEs_t * RICaction_Admitted_ItemIEs = (RICaction_Admitted_ItemIEs_t * ) testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[2]->value.choice.RICaction_Admitted_List.list.array[0];
										RICActionID = RICaction_Admitted_ItemIEs->value.choice.RICaction_Admitted_Item.ricActionID;

										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 4014400;
										message1 = 3;
                                        printf ("Receive [RIC Subscription Response]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 4){
									//Control
									if(testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID =  testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICControlStatus =  testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[2]->value.choice.RICcontrolStatus;
										
                                        messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 6014200;
										message1 = 4;
                                        printf ("Receive [RIC Control Ack]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 9){
									//subscription delete
									if(testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.count == 2){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID =  testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										
                                        messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 5014400;
										message1 = 6;
                                        printf ("Receive [RIC Subscription Delete Response]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
							else{   //unsuccessfulOutcome=3
								//subscription Failure
								if(testdc->choice.unsuccessfulOutcome->procedureCode == 8){ 
									RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
									RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
									RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
									RICaction_NotAdmitted_ItemIEs_t * RICaction_NotAdmitted_ItemIEs = (RICaction_NotAdmitted_ItemIEs_t * ) testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[2]->value.choice.RICaction_NotAdmitted_List.list.array[0];
									RICActionID = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.ricActionID;
									if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 1){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.ricRequest;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 2){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.ricService;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 3){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.transport;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 4){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.protocol;
									}
									else{
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.misc;
									}
									
									messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
									mtype = 4044400;
									message1 = 7;
                                    printf ("Receive [RIC Subscription Failure]\n");
								}
								else if(testdc->choice.unsuccessfulOutcome->procedureCode == 9){ //subscription delete Failure
									RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
									RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
									RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
									if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 1){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricRequest;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 2){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricService;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 3){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.transport;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 4){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.protocol;
									}
									else{
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.misc;
									}
									
									messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
									mtype = 5044400;
									message1 = 8;
                                    printf ("Receive [RIC Subscription Delete Failure]\n");
								}
								else if(testdc->choice.unsuccessfulOutcome->procedureCode == 4){ //Control Failure
									if(testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 1){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricRequest;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 2){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricService;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 3){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.transport;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 4){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.protocol;
										}
										else{
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.misc;
										}
										
										messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
										mtype = 6044200;
										message1 = 9;
                                        printf ("Receive [RIC Control Failure]\n");
									}
									if(testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.count == 4){
										RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 1){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.ricRequest;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 2){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.ricService;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 3){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.transport;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 4){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.protocol;
										}
										else{
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.misc;
										}
										
										messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
										mtype = 6044200;
										message1 = 10;
                                        printf ("Receive [RIC Control Failure (Insert)]\n");
									}
                                }
								else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
						
							if(senderror == 1){
								asn_enc_rval_t Bytestring = asn_encode_to_buffer(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, ErrorInd, bufferCh, bufferCh_size);

								int j=0 ;
								size_t Bytestring_size = Bytestring.encoded;

								if (Bytestring.encoded == -1){
									printf("encoding of %s failed %s\n" , "test", strerror(errno));
									exit(-1);
								}else if (Bytestring.encoded > (ssize_t) bufferCh_size) {
									printf("Buffer of size %ld is to small for %s\n" , bufferCh_size , "test");
									exit(-1);
								}else{
									memcpy(buffersend, bufferCh, Bytestring_size);
								}
							
								/////////////////////////////////////////////////////////////////////////////////////////////////////////////
								ret = sctp_sendmsg (sctpParams->connSock[0], (void *)buffersend, (size_t) Bytestring_size,
								NULL, 0, 0, 0, 0, 0, 0);
								if(ret == -1 ){
									printf("Error in sctp_sendmsg\n");
									perror("sctp_sendmsg()");
								}
								else
									printf("Successfully sent %d bytes data to E2 Agent\n", ret);
								
								senderror = 0;
								goto LOOP0;
							}

                            free(ErrorInd);
                            free(testdc);
						}

						//--------------------------------------------------------------------------------------rmr------------------------------------------------------------------------------------------//

						fprintf( stderr, "<DEMO> listen port: %s; mtype: %d; delay: %d\n",
							listen_port, mtype, delay );

						sbuf = rmr_alloc_msg( mrc, 2048);	// alloc 1st send buf; subsequent bufs alloc on send
						rbuf = NULL;						// don't need to alloc receive buffer

						while( ! rmr_ready( mrc ) ) {		// must have route table
							sleep( 1 );						// wait til we get one
						}
						fprintf( stderr, "<DEMO> rmr is ready\n" );

						int payloadlength = strlen(sbuf->payload);
						bzero (sbuf->payload, payloadlength);
						if(message1 == 1){         //setup
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"E2 Node ID\": \"%ld %ld %ld %ld\",\"RAN Function Definition\": 0,\"RAN Function ID\": %ld,\"RAN Function Revision\": 1} ",	// create the payload
								sctpParams->connSock[0], messagetype , E2NodeID3, E2NodeID2, E2NodeID1 , E2NodeID0 , RANFunctionID);
						}
						else if(message1 == 2){         //indication
							snprintf( sbuf->payload, 1500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"RIC Indication Type\": %ld,\"RIC Indication Header\": 1,\"RIC Indication Message\": %s} ",	// create the payload
								sctpParams->connSock[0],messagetype , RICRequestorID ,RICInstanceID , RANFunctionID , RICActionID, RICIndicationType,bufferinmessage);
						}
						else if(message1 == 3){         //subscription
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RAN Function Revision\": %ld,\"RIC Action ID\": %ld} ",	// create the payload
								sctpParams->connSock[0],messagetype ,RICRequestorID, RICInstanceID,RANFunctionID, RANFunctionRevision,RICActionID);
						}
						else if(message1 == 4){         //control
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Control Status\": %ld} ",	// create the payload
								sctpParams->connSock[0],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,  RICControlStatus);
						}
						else if(message1 == 5){         //indication (insert)
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"RIC Indication Type\": %ld,\"RIC Indication Header\": 1,\"RIC Indication Message\": 1,\"RIC Call process ID\": 1} ",	// create the payload
								sctpParams->connSock[0],messagetype , RICRequestorID ,RICInstanceID , RANFunctionID , RICActionID, RICIndicationType);
						}
						else if(message1 == 6){         //subscription delete
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld} ",	// create the payload
								sctpParams->connSock[0],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID);
						}
						else if(message1 == 7){         //Subscription Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[0],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,RICActionID,Cause);
						}
						else if(message1 == 8){         //Subscription Delete Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[0],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else if(message1 == 9){         //Control Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[0],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else if(message1 == 10){         //Control Failure(CallProcessID)
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Call process ID\": 1,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[0],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else{
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": nothing} ",sctpParams->connSock[0]);	// create the payload
						}

                        sbuf->mtype = mtype;							// fill in the message bits
                        sbuf->len =  strlen( sbuf->payload ) + 1;		// send full ascii-z string
                        sbuf->state = 0;
                        sbuf = rmr_send_msg( mrc, sbuf );				// send & get next buf to fill in
                        while( sbuf->state == RMR_ERR_RETRY ) {			// soft failure (device busy?) retry
                            sbuf = rmr_send_msg( mrc, sbuf );			// w/ simple spin that doesn't give up
                        }

                        rmr_free_msg(sbuf);
					}while(1);
				}
			}
			else if(countE2Node == 1){
				if((childpid1 = fork()) == 0){
					LOOP1:do{
						rmr_mbuf_t*	sbuf;				// send buffer
                        rmr_mbuf_t*	rbuf;				// received buffer
						bzero (buffer, MAX_BUFFER + 1);
						in = sctp_recvmsg (sctpParams->connSock[1], buffer, sizeof (buffer), (struct sockaddr *) NULL, 0, &sndrcvinfo, &flags);
						if( in == -1){
							printf("Error in sctp_recvmsg\n");
							perror("sctp_recvmsg()");
							close(sctpParams->connSock[1]);
							continue;
						}
						else if(in == 0){
							continue;
						}
						else{
							printf ("Length of Data received: %d\n", in);

							E2AP_PDU_t *ErrorInd = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));
							E2AP_PDU_t *testdc = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));
							asn_decode(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, (void **)&testdc, buffer, in);

							printf ("what message(ini or suc or unsuc): %d\n", testdc->present);//initiatingMessage=1 or successfulOutcome=2 or unsuccessfulOutcome=3
							
                            //initiatingMessage=1
                            if(testdc->present == 1){    
								if(testdc->choice.successfulOutcome->procedureCode == 1){
									//E2 Setup Request
									if(testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.count == 2){
										E2NodeID0 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[0];
										E2NodeID1 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[1];
										E2NodeID2 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[2];
										E2NodeID3 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[3];
										RANfunction_ItemIEs_t * RANfunction_ItemIEs = (RANfunction_ItemIEs_t * ) testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[1]->value.choice.RANfunctions_List.list.array[0];
										RANFunctionID = RANfunction_ItemIEs->value.choice.RANfunction_Item.ranFunctionID;
										RANFunctionRevision = RANfunction_ItemIEs->value.choice.RANfunction_Item.ranFunctionRevision;

										messagetype = testdc->choice.initiatingMessage->procedureCode;
										mtype = 2004200;
										message1 = 1;
                                        printf ("Receive [E2 Setup Request]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 5){
									//indication
									if(testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.count == 7){
										RICRequestorID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;				
										RICInstanceID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICActionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[2]->value.choice.RICactionID;
										RICIndicationSN = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[3]->value.choice.RICindicationSN;
										RICIndicationType = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[4]->value.choice.RICindicationType;
										bzero(bufferinmessage,2049);
										strcpy(bufferinmessage,testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[6]->value.choice.RICindicationMessage.buf);
										
										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 7004200;
										message1 = 2;
                                        printf ("Receive [Indication(Report) no. %ld]\n", RICIndicationSN);
									}
									//insert
									if(testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.count == 8){
										RICRequestorID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICActionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[2]->value.choice.RICactionID;
										RICIndicationSN = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[3]->value.choice.RICindicationSN;
										RICIndicationType = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[4]->value.choice.RICindicationType;

										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 7104200;
										message1 = 5;
                                        printf ("Receive [Indication(Insert) no. %ld]\n", RICIndicationSN);              
									}
								}
                                else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
							else if(testdc->present == 2){   //successfulOutcome=2
								if(testdc->choice.successfulOutcome->procedureCode == 8){
									//Subscription Response
									if(testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICaction_Admitted_ItemIEs_t * RICaction_Admitted_ItemIEs = (RICaction_Admitted_ItemIEs_t * ) testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[2]->value.choice.RICaction_Admitted_List.list.array[0];
										RICActionID = RICaction_Admitted_ItemIEs->value.choice.RICaction_Admitted_Item.ricActionID;

										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 4014400;
										message1 = 3;
                                        printf ("Receive [RIC Subscription Response]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 4){
									//Control
									if(testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID =  testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICControlStatus =  testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[2]->value.choice.RICcontrolStatus;
										
                                        messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 6014200;
										message1 = 4;
                                        printf ("Receive [RIC Control Ack]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 9){
									//subscription delete
									if(testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.count == 2){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID =  testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										
                                        messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 5014400;
										message1 = 6;
                                        printf ("Receive [RIC Subscription Delete Response]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
							else{   //unsuccessfulOutcome=3
								//subscription Failure
								if(testdc->choice.unsuccessfulOutcome->procedureCode == 8){ 
									RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
									RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
									RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
									RICaction_NotAdmitted_ItemIEs_t * RICaction_NotAdmitted_ItemIEs = (RICaction_NotAdmitted_ItemIEs_t * ) testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[2]->value.choice.RICaction_NotAdmitted_List.list.array[0];
									RICActionID = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.ricActionID;
									if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 1){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.ricRequest;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 2){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.ricService;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 3){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.transport;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 4){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.protocol;
									}
									else{
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.misc;
									}
									
									messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
									mtype = 4044400;
									message1 = 7;
                                    printf ("Receive [RIC Subscription Failure]\n");
								}
								else if(testdc->choice.unsuccessfulOutcome->procedureCode == 9){ //subscription delete Failure
									RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
									RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
									RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
									if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 1){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricRequest;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 2){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricService;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 3){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.transport;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 4){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.protocol;
									}
									else{
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.misc;
									}
									
									messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
									mtype = 5044400;
									message1 = 8;
                                    printf ("Receive [RIC Subscription Delete Failure]\n");
								}
								else if(testdc->choice.unsuccessfulOutcome->procedureCode == 4){ //Control Failure
									if(testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 1){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricRequest;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 2){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricService;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 3){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.transport;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 4){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.protocol;
										}
										else{
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.misc;
										}
										
										messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
										mtype = 6044200;
										message1 = 9;
                                        printf ("Receive [RIC Control Failure]\n");
									}
									if(testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.count == 4){
										RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 1){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.ricRequest;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 2){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.ricService;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 3){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.transport;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 4){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.protocol;
										}
										else{
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.misc;
										}
										
										messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
										mtype = 6044200;
										message1 = 10;
                                        printf ("Receive [RIC Control Failure (Insert)]\n");
									}
                                }
								else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
						
							if(senderror == 1){
								asn_enc_rval_t Bytestring = asn_encode_to_buffer(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, ErrorInd, bufferCh, bufferCh_size);

								int j=0 ;
								size_t Bytestring_size = Bytestring.encoded;

								if (Bytestring.encoded == -1){
									printf("encoding of %s failed %s\n" , "test", strerror(errno));
									exit(-1);
								}else if (Bytestring.encoded > (ssize_t) bufferCh_size) {
									printf("Buffer of size %ld is to small for %s\n" , bufferCh_size , "test");
									exit(-1);
								}else{
									memcpy(buffersend, bufferCh, Bytestring_size);
								}
							
								/////////////////////////////////////////////////////////////////////////////////////////////////////////////
								ret = sctp_sendmsg (sctpParams->connSock[1], (void *)buffersend, (size_t) Bytestring_size,
								NULL, 0, 0, 0, 0, 0, 0);
								if(ret == -1 ){
									printf("Error in sctp_sendmsg\n");
									perror("sctp_sendmsg()");
								}
								else
									printf("Successfully sent %d bytes data to E2 Agent\n", ret);
								
								senderror = 0;
								goto LOOP1;
							}

                            free(ErrorInd);
                            free(testdc);
						}

						//--------------------------------------------------------------------------------------rmr------------------------------------------------------------------------------------------//

						fprintf( stderr, "<DEMO> listen port: %s; mtype: %d; delay: %d\n",
							listen_port, mtype, delay );

						sbuf = rmr_alloc_msg( mrc, 2048);	// alloc 1st send buf; subsequent bufs alloc on send
						rbuf = NULL;						// don't need to alloc receive buffer

						while( ! rmr_ready( mrc ) ) {		// must have route table
							sleep( 1 );						// wait til we get one
						}
						fprintf( stderr, "<DEMO> rmr is ready\n" );

						int payloadlength = strlen(sbuf->payload);
						bzero (sbuf->payload, payloadlength);
						if(message1 == 1){         //setup
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"E2 Node ID\": \"%ld %ld %ld %ld\",\"RAN Function Definition\": 0,\"RAN Function ID\": %ld,\"RAN Function Revision\": 1} ",	// create the payload
								sctpParams->connSock[1], messagetype , E2NodeID3, E2NodeID2, E2NodeID1 , E2NodeID0 , RANFunctionID);
						}
						else if(message1 == 2){         //indication
							snprintf( sbuf->payload, 1500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"RIC Indication Type\": %ld,\"RIC Indication Header\": 1,\"RIC Indication Message\": %s} ",	// create the payload
								sctpParams->connSock[1],messagetype , RICRequestorID ,RICInstanceID , RANFunctionID , RICActionID, RICIndicationType,bufferinmessage);
						}
						else if(message1 == 3){         //subscription
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RAN Function Revision\": %ld,\"RIC Action ID\": %ld} ",	// create the payload
								sctpParams->connSock[1],messagetype ,RICRequestorID, RICInstanceID,RANFunctionID, RANFunctionRevision,RICActionID);
						}
						else if(message1 == 4){         //control
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Control Status\": %ld} ",	// create the payload
								sctpParams->connSock[1],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,  RICControlStatus);
						}
						else if(message1 == 5){         //indication (insert)
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"RIC Indication Type\": %ld,\"RIC Indication Header\": 1,\"RIC Indication Message\": 1,\"RIC Call process ID\": 1} ",	// create the payload
								sctpParams->connSock[1],messagetype , RICRequestorID ,RICInstanceID , RANFunctionID , RICActionID, RICIndicationType);
						}
						else if(message1 == 6){         //subscription delete
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld} ",	// create the payload
								sctpParams->connSock[1],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID);
						}
						else if(message1 == 7){         //Subscription Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[1],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,RICActionID,Cause);
						}
						else if(message1 == 8){         //Subscription Delete Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[1],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else if(message1 == 9){         //Control Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[1],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else if(message1 == 10){         //Control Failure(CallProcessID)
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Call process ID\": 1,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[1],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else{
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": nothing} ",sctpParams->connSock[1]);	// create the payload
						}

                        sbuf->mtype = mtype;							// fill in the message bits
                        sbuf->len =  strlen( sbuf->payload ) + 1;		// send full ascii-z string
                        sbuf->state = 0;
                        sbuf = rmr_send_msg( mrc, sbuf );				// send & get next buf to fill in
                        while( sbuf->state == RMR_ERR_RETRY ) {			// soft failure (device busy?) retry
                            sbuf = rmr_send_msg( mrc, sbuf );			// w/ simple spin that doesn't give up
                        }

                        rmr_free_msg(sbuf);
					}while(1);
				}
			}
			else if(countE2Node == 2){
				if((childpid2 = fork()) == 0){
					LOOP2:do{
						rmr_mbuf_t*	sbuf;				// send buffer
                        rmr_mbuf_t*	rbuf;				// received buffer
						bzero (buffer, MAX_BUFFER + 1);
						in = sctp_recvmsg (sctpParams->connSock[2], buffer, sizeof (buffer), (struct sockaddr *) NULL, 0, &sndrcvinfo, &flags);
						if( in == -1){
							printf("Error in sctp_recvmsg\n");
							perror("sctp_recvmsg()");
							close(sctpParams->connSock[2]);
							continue;
						}
						else if(in == 0){
							continue;
						}
						else{
							printf ("Length of Data received: %d\n", in);

							E2AP_PDU_t *ErrorInd = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));
							E2AP_PDU_t *testdc = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));
							asn_decode(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, (void **)&testdc, buffer, in);

							printf ("what message(ini or suc or unsuc): %d\n", testdc->present);//initiatingMessage=1 or successfulOutcome=2 or unsuccessfulOutcome=3
							
                            //initiatingMessage=1
                            if(testdc->present == 1){    
								if(testdc->choice.successfulOutcome->procedureCode == 1){
									//E2 Setup Request
									if(testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.count == 2){
										E2NodeID0 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[0];
										E2NodeID1 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[1];
										E2NodeID2 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[2];
										E2NodeID3 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[3];
										RANfunction_ItemIEs_t * RANfunction_ItemIEs = (RANfunction_ItemIEs_t * ) testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[1]->value.choice.RANfunctions_List.list.array[0];
										RANFunctionID = RANfunction_ItemIEs->value.choice.RANfunction_Item.ranFunctionID;
										RANFunctionRevision = RANfunction_ItemIEs->value.choice.RANfunction_Item.ranFunctionRevision;

										messagetype = testdc->choice.initiatingMessage->procedureCode;
										mtype = 2004200;
										message1 = 1;
                                        printf ("Receive [E2 Setup Request]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 5){
									//indication
									if(testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.count == 7){
										RICRequestorID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;				
										RICInstanceID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICActionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[2]->value.choice.RICactionID;
										RICIndicationSN = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[3]->value.choice.RICindicationSN;
										RICIndicationType = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[4]->value.choice.RICindicationType;
										bzero(bufferinmessage,2049);
										strcpy(bufferinmessage,testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[6]->value.choice.RICindicationMessage.buf);
										
										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 7004200;
										message1 = 2;
                                        printf ("Receive [Indication(Report) no. %ld]\n", RICIndicationSN);
									}
									//insert
									if(testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.count == 8){
										RICRequestorID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICActionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[2]->value.choice.RICactionID;
										RICIndicationSN = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[3]->value.choice.RICindicationSN;
										RICIndicationType = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[4]->value.choice.RICindicationType;

										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 7104200;
										message1 = 5;
                                        printf ("Receive [Indication(Insert) no. %ld]\n", RICIndicationSN);              
									}
								}
                                else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
							else if(testdc->present == 2){   //successfulOutcome=2
								if(testdc->choice.successfulOutcome->procedureCode == 8){
									//Subscription Response
									if(testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICaction_Admitted_ItemIEs_t * RICaction_Admitted_ItemIEs = (RICaction_Admitted_ItemIEs_t * ) testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[2]->value.choice.RICaction_Admitted_List.list.array[0];
										RICActionID = RICaction_Admitted_ItemIEs->value.choice.RICaction_Admitted_Item.ricActionID;

										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 4014400;
										message1 = 3;
                                        printf ("Receive [RIC Subscription Response]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 4){
									//Control
									if(testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID =  testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICControlStatus =  testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[2]->value.choice.RICcontrolStatus;
										
                                        messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 6014200;
										message1 = 4;
                                        printf ("Receive [RIC Control Ack]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 9){
									//subscription delete
									if(testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.count == 2){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID =  testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										
                                        messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 5014400;
										message1 = 6;
                                        printf ("Receive [RIC Subscription Delete Response]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
							else{   //unsuccessfulOutcome=3
								//subscription Failure
								if(testdc->choice.unsuccessfulOutcome->procedureCode == 8){ 
									RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
									RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
									RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
									RICaction_NotAdmitted_ItemIEs_t * RICaction_NotAdmitted_ItemIEs = (RICaction_NotAdmitted_ItemIEs_t * ) testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[2]->value.choice.RICaction_NotAdmitted_List.list.array[0];
									RICActionID = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.ricActionID;
									if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 1){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.ricRequest;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 2){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.ricService;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 3){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.transport;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 4){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.protocol;
									}
									else{
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.misc;
									}
									
									messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
									mtype = 4044400;
									message1 = 7;
                                    printf ("Receive [RIC Subscription Failure]\n");
								}
								else if(testdc->choice.unsuccessfulOutcome->procedureCode == 9){ //subscription delete Failure
									RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
									RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
									RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
									if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 1){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricRequest;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 2){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricService;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 3){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.transport;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 4){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.protocol;
									}
									else{
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.misc;
									}
									
									messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
									mtype = 5044400;
									message1 = 8;
                                    printf ("Receive [RIC Subscription Delete Failure]\n");
								}
								else if(testdc->choice.unsuccessfulOutcome->procedureCode == 4){ //Control Failure
									if(testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 1){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricRequest;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 2){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricService;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 3){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.transport;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 4){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.protocol;
										}
										else{
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.misc;
										}
										
										messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
										mtype = 6044200;
										message1 = 9;
                                        printf ("Receive [RIC Control Failure]\n");
									}
									if(testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.count == 4){
										RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 1){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.ricRequest;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 2){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.ricService;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 3){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.transport;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 4){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.protocol;
										}
										else{
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.misc;
										}
										
										messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
										mtype = 6044200;
										message1 = 10;
                                        printf ("Receive [RIC Control Failure (Insert)]\n");
									}
                                }
								else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
						
							if(senderror == 1){
								asn_enc_rval_t Bytestring = asn_encode_to_buffer(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, ErrorInd, bufferCh, bufferCh_size);

								int j=0 ;
								size_t Bytestring_size = Bytestring.encoded;

								if (Bytestring.encoded == -1){
									printf("encoding of %s failed %s\n" , "test", strerror(errno));
									exit(-1);
								}else if (Bytestring.encoded > (ssize_t) bufferCh_size) {
									printf("Buffer of size %ld is to small for %s\n" , bufferCh_size , "test");
									exit(-1);
								}else{
									memcpy(buffersend, bufferCh, Bytestring_size);
								}
							
								/////////////////////////////////////////////////////////////////////////////////////////////////////////////
								ret = sctp_sendmsg (sctpParams->connSock[2], (void *)buffersend, (size_t) Bytestring_size,
								NULL, 0, 0, 0, 0, 0, 0);
								if(ret == -1 ){
									printf("Error in sctp_sendmsg\n");
									perror("sctp_sendmsg()");
								}
								else
									printf("Successfully sent %d bytes data to E2 Agent\n", ret);
								
								senderror = 0;
								goto LOOP2;
							}

                            free(ErrorInd);
                            free(testdc);
						}

						//--------------------------------------------------------------------------------------rmr------------------------------------------------------------------------------------------//

						fprintf( stderr, "<DEMO> listen port: %s; mtype: %d; delay: %d\n",
							listen_port, mtype, delay );

						sbuf = rmr_alloc_msg( mrc, 2048);	// alloc 1st send buf; subsequent bufs alloc on send
						rbuf = NULL;						// don't need to alloc receive buffer

						while( ! rmr_ready( mrc ) ) {		// must have route table
							sleep( 1 );						// wait til we get one
						}
						fprintf( stderr, "<DEMO> rmr is ready\n" );

						int payloadlength = strlen(sbuf->payload);
						bzero (sbuf->payload, payloadlength);
						if(message1 == 1){         //setup
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"E2 Node ID\": \"%ld %ld %ld %ld\",\"RAN Function Definition\": 0,\"RAN Function ID\": %ld,\"RAN Function Revision\": 1} ",	// create the payload
								sctpParams->connSock[2], messagetype , E2NodeID3, E2NodeID2, E2NodeID1 , E2NodeID0 , RANFunctionID);
						}
						else if(message1 == 2){         //indication
							snprintf( sbuf->payload, 1500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"RIC Indication Type\": %ld,\"RIC Indication Header\": 1,\"RIC Indication Message\": %s} ",	// create the payload
								sctpParams->connSock[2],messagetype , RICRequestorID ,RICInstanceID , RANFunctionID , RICActionID, RICIndicationType,bufferinmessage);
						}
						else if(message1 == 3){         //subscription
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RAN Function Revision\": %ld,\"RIC Action ID\": %ld} ",	// create the payload
								sctpParams->connSock[2],messagetype ,RICRequestorID, RICInstanceID,RANFunctionID, RANFunctionRevision,RICActionID);
						}
						else if(message1 == 4){         //control
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Control Status\": %ld} ",	// create the payload
								sctpParams->connSock[2],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,  RICControlStatus);
						}
						else if(message1 == 5){         //indication (insert)
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"RIC Indication Type\": %ld,\"RIC Indication Header\": 1,\"RIC Indication Message\": 1,\"RIC Call process ID\": 1} ",	// create the payload
								sctpParams->connSock[2],messagetype , RICRequestorID ,RICInstanceID , RANFunctionID , RICActionID, RICIndicationType);
						}
						else if(message1 == 6){         //subscription delete
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld} ",	// create the payload
								sctpParams->connSock[2],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID);
						}
						else if(message1 == 7){         //Subscription Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[2],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,RICActionID,Cause);
						}
						else if(message1 == 8){         //Subscription Delete Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[2],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else if(message1 == 9){         //Control Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[2],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else if(message1 == 10){         //Control Failure(CallProcessID)
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Call process ID\": 1,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[2],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else{
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": nothing} ",sctpParams->connSock[2]);	// create the payload
						}

                        sbuf->mtype = mtype;							// fill in the message bits
                        sbuf->len =  strlen( sbuf->payload ) + 1;		// send full ascii-z string
                        sbuf->state = 0;
                        sbuf = rmr_send_msg( mrc, sbuf );				// send & get next buf to fill in
                        while( sbuf->state == RMR_ERR_RETRY ) {			// soft failure (device busy?) retry
                            sbuf = rmr_send_msg( mrc, sbuf );			// w/ simple spin that doesn't give up
                        }

                        rmr_free_msg(sbuf);
					}while(1);
				}
			}
			else if(countE2Node == 3){
				if((childpid3 = fork()) == 0){
					LOOP3:do{
						rmr_mbuf_t*	sbuf;				// send buffer
                        rmr_mbuf_t*	rbuf;				// received buffer
						bzero (buffer, MAX_BUFFER + 1);
						in = sctp_recvmsg (sctpParams->connSock[3], buffer, sizeof (buffer), (struct sockaddr *) NULL, 0, &sndrcvinfo, &flags);
						if( in == -1){
							printf("Error in sctp_recvmsg\n");
							perror("sctp_recvmsg()");
							close(sctpParams->connSock[3]);
							continue;
						}
						else if(in == 0){
							continue;
						}
						else{
							printf ("Length of Data received: %d\n", in);

							E2AP_PDU_t *ErrorInd = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));
							E2AP_PDU_t *testdc = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));
							asn_decode(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, (void **)&testdc, buffer, in);

							printf ("what message(ini or suc or unsuc): %d\n", testdc->present);//initiatingMessage=1 or successfulOutcome=2 or unsuccessfulOutcome=3
							
                            //initiatingMessage=1
                            if(testdc->present == 1){    
								if(testdc->choice.successfulOutcome->procedureCode == 1){
									//E2 Setup Request
									if(testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.count == 2){
										E2NodeID0 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[0];
										E2NodeID1 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[1];
										E2NodeID2 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[2];
										E2NodeID3 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[3];
										RANfunction_ItemIEs_t * RANfunction_ItemIEs = (RANfunction_ItemIEs_t * ) testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[1]->value.choice.RANfunctions_List.list.array[0];
										RANFunctionID = RANfunction_ItemIEs->value.choice.RANfunction_Item.ranFunctionID;
										RANFunctionRevision = RANfunction_ItemIEs->value.choice.RANfunction_Item.ranFunctionRevision;

										messagetype = testdc->choice.initiatingMessage->procedureCode;
										mtype = 2004200;
										message1 = 1;
                                        printf ("Receive [E2 Setup Request]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 5){
									//indication
									if(testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.count == 7){
										RICRequestorID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;				
										RICInstanceID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICActionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[2]->value.choice.RICactionID;
										RICIndicationSN = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[3]->value.choice.RICindicationSN;
										RICIndicationType = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[4]->value.choice.RICindicationType;
										bzero(bufferinmessage,2049);
										strcpy(bufferinmessage,testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[6]->value.choice.RICindicationMessage.buf);
										
										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 7004200;
										message1 = 2;
                                        printf ("Receive [Indication(Report) no. %ld]\n", RICIndicationSN);
									}
									//insert
									if(testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.count == 8){
										RICRequestorID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICActionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[2]->value.choice.RICactionID;
										RICIndicationSN = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[3]->value.choice.RICindicationSN;
										RICIndicationType = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[4]->value.choice.RICindicationType;

										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 7104200;
										message1 = 5;
                                        printf ("Receive [Indication(Insert) no. %ld]\n", RICIndicationSN);              
									}
								}
                                else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
							else if(testdc->present == 2){   //successfulOutcome=2
								if(testdc->choice.successfulOutcome->procedureCode == 8){
									//Subscription Response
									if(testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICaction_Admitted_ItemIEs_t * RICaction_Admitted_ItemIEs = (RICaction_Admitted_ItemIEs_t * ) testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[2]->value.choice.RICaction_Admitted_List.list.array[0];
										RICActionID = RICaction_Admitted_ItemIEs->value.choice.RICaction_Admitted_Item.ricActionID;

										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 4014400;
										message1 = 3;
                                        printf ("Receive [RIC Subscription Response]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 4){
									//Control
									if(testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID =  testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICControlStatus =  testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[2]->value.choice.RICcontrolStatus;
										
                                        messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 6014200;
										message1 = 4;
                                        printf ("Receive [RIC Control Ack]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 9){
									//subscription delete
									if(testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.count == 2){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID =  testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										
                                        messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 5014400;
										message1 = 6;
                                        printf ("Receive [RIC Subscription Delete Response]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
							else{   //unsuccessfulOutcome=3
								//subscription Failure
								if(testdc->choice.unsuccessfulOutcome->procedureCode == 8){ 
									RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
									RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
									RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
									RICaction_NotAdmitted_ItemIEs_t * RICaction_NotAdmitted_ItemIEs = (RICaction_NotAdmitted_ItemIEs_t * ) testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[2]->value.choice.RICaction_NotAdmitted_List.list.array[0];
									RICActionID = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.ricActionID;
									if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 1){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.ricRequest;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 2){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.ricService;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 3){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.transport;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 4){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.protocol;
									}
									else{
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.misc;
									}
									
									messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
									mtype = 4044400;
									message1 = 7;
                                    printf ("Receive [RIC Subscription Failure]\n");
								}
								else if(testdc->choice.unsuccessfulOutcome->procedureCode == 9){ //subscription delete Failure
									RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
									RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
									RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
									if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 1){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricRequest;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 2){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricService;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 3){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.transport;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 4){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.protocol;
									}
									else{
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.misc;
									}
									
									messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
									mtype = 5044400;
									message1 = 8;
                                    printf ("Receive [RIC Subscription Delete Failure]\n");
								}
								else if(testdc->choice.unsuccessfulOutcome->procedureCode == 4){ //Control Failure
									if(testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 1){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricRequest;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 2){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricService;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 3){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.transport;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 4){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.protocol;
										}
										else{
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.misc;
										}
										
										messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
										mtype = 6044200;
										message1 = 9;
                                        printf ("Receive [RIC Control Failure]\n");
									}
									if(testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.count == 4){
										RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 1){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.ricRequest;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 2){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.ricService;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 3){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.transport;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 4){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.protocol;
										}
										else{
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.misc;
										}
										
										messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
										mtype = 6044200;
										message1 = 10;
                                        printf ("Receive [RIC Control Failure (Insert)]\n");
									}
                                }
								else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
						
							if(senderror == 1){
								asn_enc_rval_t Bytestring = asn_encode_to_buffer(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, ErrorInd, bufferCh, bufferCh_size);

								int j=0 ;
								size_t Bytestring_size = Bytestring.encoded;

								if (Bytestring.encoded == -1){
									printf("encoding of %s failed %s\n" , "test", strerror(errno));
									exit(-1);
								}else if (Bytestring.encoded > (ssize_t) bufferCh_size) {
									printf("Buffer of size %ld is to small for %s\n" , bufferCh_size , "test");
									exit(-1);
								}else{
									memcpy(buffersend, bufferCh, Bytestring_size);
								}
							
								/////////////////////////////////////////////////////////////////////////////////////////////////////////////
								ret = sctp_sendmsg (sctpParams->connSock[3], (void *)buffersend, (size_t) Bytestring_size,
								NULL, 0, 0, 0, 0, 0, 0);
								if(ret == -1 ){
									printf("Error in sctp_sendmsg\n");
									perror("sctp_sendmsg()");
								}
								else
									printf("Successfully sent %d bytes data to E2 Agent\n", ret);
								
								senderror = 0;
								goto LOOP3;
							}

                            free(ErrorInd);
                            free(testdc);
						}

						//--------------------------------------------------------------------------------------rmr------------------------------------------------------------------------------------------//

						fprintf( stderr, "<DEMO> listen port: %s; mtype: %d; delay: %d\n",
							listen_port, mtype, delay );

						sbuf = rmr_alloc_msg( mrc, 2048);	// alloc 1st send buf; subsequent bufs alloc on send
						rbuf = NULL;						// don't need to alloc receive buffer

						while( ! rmr_ready( mrc ) ) {		// must have route table
							sleep( 1 );						// wait til we get one
						}
						fprintf( stderr, "<DEMO> rmr is ready\n" );

						int payloadlength = strlen(sbuf->payload);
						bzero (sbuf->payload, payloadlength);
						if(message1 == 1){         //setup
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"E2 Node ID\": \"%ld %ld %ld %ld\",\"RAN Function Definition\": 0,\"RAN Function ID\": %ld,\"RAN Function Revision\": 1} ",	// create the payload
								sctpParams->connSock[3], messagetype , E2NodeID3, E2NodeID2, E2NodeID1 , E2NodeID0 , RANFunctionID);
						}
						else if(message1 == 2){         //indication
							snprintf( sbuf->payload, 1500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"RIC Indication Type\": %ld,\"RIC Indication Header\": 1,\"RIC Indication Message\": %s} ",	// create the payload
								sctpParams->connSock[3],messagetype , RICRequestorID ,RICInstanceID , RANFunctionID , RICActionID, RICIndicationType,bufferinmessage);
						}
						else if(message1 == 3){         //subscription
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RAN Function Revision\": %ld,\"RIC Action ID\": %ld} ",	// create the payload
								sctpParams->connSock[3],messagetype ,RICRequestorID, RICInstanceID,RANFunctionID, RANFunctionRevision,RICActionID);
						}
						else if(message1 == 4){         //control
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Control Status\": %ld} ",	// create the payload
								sctpParams->connSock[3],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,  RICControlStatus);
						}
						else if(message1 == 5){         //indication (insert)
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"RIC Indication Type\": %ld,\"RIC Indication Header\": 1,\"RIC Indication Message\": 1,\"RIC Call process ID\": 1} ",	// create the payload
								sctpParams->connSock[3],messagetype , RICRequestorID ,RICInstanceID , RANFunctionID , RICActionID, RICIndicationType);
						}
						else if(message1 == 6){         //subscription delete
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld} ",	// create the payload
								sctpParams->connSock[3],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID);
						}
						else if(message1 == 7){         //Subscription Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[3],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,RICActionID,Cause);
						}
						else if(message1 == 8){         //Subscription Delete Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[3],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else if(message1 == 9){         //Control Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[3],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else if(message1 == 10){         //Control Failure(CallProcessID)
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Call process ID\": 1,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[3],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else{
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": nothing} ",sctpParams->connSock[3]);	// create the payload
						}

                        sbuf->mtype = mtype;							// fill in the message bits
                        sbuf->len =  strlen( sbuf->payload ) + 1;		// send full ascii-z string
                        sbuf->state = 0;
                        sbuf = rmr_send_msg( mrc, sbuf );				// send & get next buf to fill in
                        while( sbuf->state == RMR_ERR_RETRY ) {			// soft failure (device busy?) retry
                            sbuf = rmr_send_msg( mrc, sbuf );			// w/ simple spin that doesn't give up
                        }

                        rmr_free_msg(sbuf);
					}while(1);
				}
			}
			else if(countE2Node == 4){
				if((childpid4 = fork()) == 0){
					LOOP4:do{
						rmr_mbuf_t*	sbuf;				// send buffer
                        rmr_mbuf_t*	rbuf;				// received buffer
						bzero (buffer, MAX_BUFFER + 1);
						in = sctp_recvmsg (sctpParams->connSock[4], buffer, sizeof (buffer), (struct sockaddr *) NULL, 0, &sndrcvinfo, &flags);
						if( in == -1){
							printf("Error in sctp_recvmsg\n");
							perror("sctp_recvmsg()");
							close(sctpParams->connSock[4]);
							continue;
						}
						else if(in == 0){
							continue;
						}
						else{
							printf ("Length of Data received: %d\n", in);

							E2AP_PDU_t *ErrorInd = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));
							E2AP_PDU_t *testdc = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));
							asn_decode(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, (void **)&testdc, buffer, in);

							printf ("what message(ini or suc or unsuc): %d\n", testdc->present);//initiatingMessage=1 or successfulOutcome=2 or unsuccessfulOutcome=3
							
                            //initiatingMessage=1
                            if(testdc->present == 1){    
								if(testdc->choice.successfulOutcome->procedureCode == 1){
									//E2 Setup Request
									if(testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.count == 2){
										E2NodeID0 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[0];
										E2NodeID1 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[1];
										E2NodeID2 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[2];
										E2NodeID3 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[3];
										RANfunction_ItemIEs_t * RANfunction_ItemIEs = (RANfunction_ItemIEs_t * ) testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[1]->value.choice.RANfunctions_List.list.array[0];
										RANFunctionID = RANfunction_ItemIEs->value.choice.RANfunction_Item.ranFunctionID;
										RANFunctionRevision = RANfunction_ItemIEs->value.choice.RANfunction_Item.ranFunctionRevision;

										messagetype = testdc->choice.initiatingMessage->procedureCode;
										mtype = 2004200;
										message1 = 1;
                                        printf ("Receive [E2 Setup Request]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 5){
									//indication
									if(testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.count == 7){
										RICRequestorID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;				
										RICInstanceID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICActionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[2]->value.choice.RICactionID;
										RICIndicationSN = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[3]->value.choice.RICindicationSN;
										RICIndicationType = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[4]->value.choice.RICindicationType;
										bzero(bufferinmessage,2049);
										strcpy(bufferinmessage,testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[6]->value.choice.RICindicationMessage.buf);
										
										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 7004200;
										message1 = 2;
                                        printf ("Receive [Indication(Report) no. %ld]\n", RICIndicationSN);
									}
									//insert
									if(testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.count == 8){
										RICRequestorID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICActionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[2]->value.choice.RICactionID;
										RICIndicationSN = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[3]->value.choice.RICindicationSN;
										RICIndicationType = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[4]->value.choice.RICindicationType;

										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 7104200;
										message1 = 5;
                                        printf ("Receive [Indication(Insert) no. %ld]\n", RICIndicationSN);              
									}
								}
                                else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
							else if(testdc->present == 2){   //successfulOutcome=2
								if(testdc->choice.successfulOutcome->procedureCode == 8){
									//Subscription Response
									if(testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICaction_Admitted_ItemIEs_t * RICaction_Admitted_ItemIEs = (RICaction_Admitted_ItemIEs_t * ) testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[2]->value.choice.RICaction_Admitted_List.list.array[0];
										RICActionID = RICaction_Admitted_ItemIEs->value.choice.RICaction_Admitted_Item.ricActionID;

										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 4014400;
										message1 = 3;
                                        printf ("Receive [RIC Subscription Response]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 4){
									//Control
									if(testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID =  testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICControlStatus =  testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[2]->value.choice.RICcontrolStatus;
										
                                        messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 6014200;
										message1 = 4;
                                        printf ("Receive [RIC Control Ack]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 9){
									//subscription delete
									if(testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.count == 2){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID =  testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										
                                        messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 5014400;
										message1 = 6;
                                        printf ("Receive [RIC Subscription Delete Response]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
							else{   //unsuccessfulOutcome=3
								//subscription Failure
								if(testdc->choice.unsuccessfulOutcome->procedureCode == 8){ 
									RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
									RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
									RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
									RICaction_NotAdmitted_ItemIEs_t * RICaction_NotAdmitted_ItemIEs = (RICaction_NotAdmitted_ItemIEs_t * ) testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[2]->value.choice.RICaction_NotAdmitted_List.list.array[0];
									RICActionID = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.ricActionID;
									if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 1){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.ricRequest;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 2){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.ricService;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 3){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.transport;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 4){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.protocol;
									}
									else{
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.misc;
									}
									
									messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
									mtype = 4044400;
									message1 = 7;
                                    printf ("Receive [RIC Subscription Failure]\n");
								}
								else if(testdc->choice.unsuccessfulOutcome->procedureCode == 9){ //subscription delete Failure
									RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
									RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
									RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
									if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 1){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricRequest;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 2){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricService;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 3){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.transport;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 4){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.protocol;
									}
									else{
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.misc;
									}
									
									messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
									mtype = 5044400;
									message1 = 8;
                                    printf ("Receive [RIC Subscription Delete Failure]\n");
								}
								else if(testdc->choice.unsuccessfulOutcome->procedureCode == 4){ //Control Failure
									if(testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 1){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricRequest;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 2){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricService;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 3){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.transport;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 4){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.protocol;
										}
										else{
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.misc;
										}
										
										messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
										mtype = 6044200;
										message1 = 9;
                                        printf ("Receive [RIC Control Failure]\n");
									}
									if(testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.count == 4){
										RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 1){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.ricRequest;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 2){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.ricService;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 3){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.transport;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 4){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.protocol;
										}
										else{
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.misc;
										}
										
										messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
										mtype = 6044200;
										message1 = 10;
                                        printf ("Receive [RIC Control Failure (Insert)]\n");
									}
                                }
								else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
						
							if(senderror == 1){
								asn_enc_rval_t Bytestring = asn_encode_to_buffer(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, ErrorInd, bufferCh, bufferCh_size);

								int j=0 ;
								size_t Bytestring_size = Bytestring.encoded;

								if (Bytestring.encoded == -1){
									printf("encoding of %s failed %s\n" , "test", strerror(errno));
									exit(-1);
								}else if (Bytestring.encoded > (ssize_t) bufferCh_size) {
									printf("Buffer of size %ld is to small for %s\n" , bufferCh_size , "test");
									exit(-1);
								}else{
									memcpy(buffersend, bufferCh, Bytestring_size);
								}
							
								/////////////////////////////////////////////////////////////////////////////////////////////////////////////
								ret = sctp_sendmsg (sctpParams->connSock[4], (void *)buffersend, (size_t) Bytestring_size,
								NULL, 0, 0, 0, 0, 0, 0);
								if(ret == -1 ){
									printf("Error in sctp_sendmsg\n");
									perror("sctp_sendmsg()");
								}
								else
									printf("Successfully sent %d bytes data to E2 Agent\n", ret);
								
								senderror = 0;
								goto LOOP4;
							}

                            free(ErrorInd);
                            free(testdc);
						}

						//--------------------------------------------------------------------------------------rmr------------------------------------------------------------------------------------------//

						fprintf( stderr, "<DEMO> listen port: %s; mtype: %d; delay: %d\n",
							listen_port, mtype, delay );

						sbuf = rmr_alloc_msg( mrc, 2048);	// alloc 1st send buf; subsequent bufs alloc on send
						rbuf = NULL;						// don't need to alloc receive buffer

						while( ! rmr_ready( mrc ) ) {		// must have route table
							sleep( 1 );						// wait til we get one
						}
						fprintf( stderr, "<DEMO> rmr is ready\n" );

						int payloadlength = strlen(sbuf->payload);
						bzero (sbuf->payload, payloadlength);
						if(message1 == 1){         //setup
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"E2 Node ID\": \"%ld %ld %ld %ld\",\"RAN Function Definition\": 0,\"RAN Function ID\": %ld,\"RAN Function Revision\": 1} ",	// create the payload
								sctpParams->connSock[4], messagetype , E2NodeID3, E2NodeID2, E2NodeID1 , E2NodeID0 , RANFunctionID);
						}
						else if(message1 == 2){         //indication
							snprintf( sbuf->payload, 1500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"RIC Indication Type\": %ld,\"RIC Indication Header\": 1,\"RIC Indication Message\": %s} ",	// create the payload
								sctpParams->connSock[4],messagetype , RICRequestorID ,RICInstanceID , RANFunctionID , RICActionID, RICIndicationType,bufferinmessage);
						}
						else if(message1 == 3){         //subscription
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RAN Function Revision\": %ld,\"RIC Action ID\": %ld} ",	// create the payload
								sctpParams->connSock[4],messagetype ,RICRequestorID, RICInstanceID,RANFunctionID, RANFunctionRevision,RICActionID);
						}
						else if(message1 == 4){         //control
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Control Status\": %ld} ",	// create the payload
								sctpParams->connSock[4],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,  RICControlStatus);
						}
						else if(message1 == 5){         //indication (insert)
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"RIC Indication Type\": %ld,\"RIC Indication Header\": 1,\"RIC Indication Message\": 1,\"RIC Call process ID\": 1} ",	// create the payload
								sctpParams->connSock[4],messagetype , RICRequestorID ,RICInstanceID , RANFunctionID , RICActionID, RICIndicationType);
						}
						else if(message1 == 6){         //subscription delete
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld} ",	// create the payload
								sctpParams->connSock[4],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID);
						}
						else if(message1 == 7){         //Subscription Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[4],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,RICActionID,Cause);
						}
						else if(message1 == 8){         //Subscription Delete Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[4],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else if(message1 == 9){         //Control Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[4],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else if(message1 == 10){         //Control Failure(CallProcessID)
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Call process ID\": 1,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[4],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else{
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": nothing} ",sctpParams->connSock[4]);	// create the payload
						}

                        sbuf->mtype = mtype;							// fill in the message bits
                        sbuf->len =  strlen( sbuf->payload ) + 1;		// send full ascii-z string
                        sbuf->state = 0;
                        sbuf = rmr_send_msg( mrc, sbuf );				// send & get next buf to fill in
                        while( sbuf->state == RMR_ERR_RETRY ) {			// soft failure (device busy?) retry
                            sbuf = rmr_send_msg( mrc, sbuf );			// w/ simple spin that doesn't give up
                        }

                        rmr_free_msg(sbuf);
					}while(1);
				}
			}
			else if(countE2Node == 5){
				if((childpid5 = fork()) == 0){
					LOOP5:do{
						rmr_mbuf_t*	sbuf;				// send buffer
                        rmr_mbuf_t*	rbuf;				// received buffer
						bzero (buffer, MAX_BUFFER + 1);
						in = sctp_recvmsg (sctpParams->connSock[5], buffer, sizeof (buffer), (struct sockaddr *) NULL, 0, &sndrcvinfo, &flags);
						if( in == -1){
							printf("Error in sctp_recvmsg\n");
							perror("sctp_recvmsg()");
							close(sctpParams->connSock[5]);
							continue;
						}
						else if(in == 0){
							continue;
						}
						else{
							printf ("Length of Data received: %d\n", in);

							E2AP_PDU_t *ErrorInd = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));
							E2AP_PDU_t *testdc = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));
							asn_decode(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, (void **)&testdc, buffer, in);

							printf ("what message(ini or suc or unsuc): %d\n", testdc->present);//initiatingMessage=1 or successfulOutcome=2 or unsuccessfulOutcome=3
							
                            //initiatingMessage=1
                            if(testdc->present == 1){    
								if(testdc->choice.successfulOutcome->procedureCode == 1){
									//E2 Setup Request
									if(testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.count == 2){
										E2NodeID0 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[0];
										E2NodeID1 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[1];
										E2NodeID2 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[2];
										E2NodeID3 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[3];
										RANfunction_ItemIEs_t * RANfunction_ItemIEs = (RANfunction_ItemIEs_t * ) testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[1]->value.choice.RANfunctions_List.list.array[0];
										RANFunctionID = RANfunction_ItemIEs->value.choice.RANfunction_Item.ranFunctionID;
										RANFunctionRevision = RANfunction_ItemIEs->value.choice.RANfunction_Item.ranFunctionRevision;

										messagetype = testdc->choice.initiatingMessage->procedureCode;
										mtype = 2004200;
										message1 = 1;
                                        printf ("Receive [E2 Setup Request]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 5){
									//indication
									if(testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.count == 7){
										RICRequestorID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;				
										RICInstanceID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICActionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[2]->value.choice.RICactionID;
										RICIndicationSN = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[3]->value.choice.RICindicationSN;
										RICIndicationType = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[4]->value.choice.RICindicationType;
										bzero(bufferinmessage,2049);
										strcpy(bufferinmessage,testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[6]->value.choice.RICindicationMessage.buf);
										
										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 7004200;
										message1 = 2;
                                        printf ("Receive [Indication(Report) no. %ld]\n", RICIndicationSN);
									}
									//insert
									if(testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.count == 8){
										RICRequestorID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICActionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[2]->value.choice.RICactionID;
										RICIndicationSN = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[3]->value.choice.RICindicationSN;
										RICIndicationType = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[4]->value.choice.RICindicationType;

										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 7104200;
										message1 = 5;
                                        printf ("Receive [Indication(Insert) no. %ld]\n", RICIndicationSN);              
									}
								}
                                else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
							else if(testdc->present == 2){   //successfulOutcome=2
								if(testdc->choice.successfulOutcome->procedureCode == 8){
									//Subscription Response
									if(testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICaction_Admitted_ItemIEs_t * RICaction_Admitted_ItemIEs = (RICaction_Admitted_ItemIEs_t * ) testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[2]->value.choice.RICaction_Admitted_List.list.array[0];
										RICActionID = RICaction_Admitted_ItemIEs->value.choice.RICaction_Admitted_Item.ricActionID;

										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 4014400;
										message1 = 3;
                                        printf ("Receive [RIC Subscription Response]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 4){
									//Control
									if(testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID =  testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICControlStatus =  testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[2]->value.choice.RICcontrolStatus;
										
                                        messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 6014200;
										message1 = 4;
                                        printf ("Receive [RIC Control Ack]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 9){
									//subscription delete
									if(testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.count == 2){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID =  testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										
                                        messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 5014400;
										message1 = 6;
                                        printf ("Receive [RIC Subscription Delete Response]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
							else{   //unsuccessfulOutcome=3
								//subscription Failure
								if(testdc->choice.unsuccessfulOutcome->procedureCode == 8){ 
									RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
									RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
									RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
									RICaction_NotAdmitted_ItemIEs_t * RICaction_NotAdmitted_ItemIEs = (RICaction_NotAdmitted_ItemIEs_t * ) testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[2]->value.choice.RICaction_NotAdmitted_List.list.array[0];
									RICActionID = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.ricActionID;
									if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 1){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.ricRequest;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 2){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.ricService;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 3){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.transport;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 4){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.protocol;
									}
									else{
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.misc;
									}
									
									messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
									mtype = 4044400;
									message1 = 7;
                                    printf ("Receive [RIC Subscription Failure]\n");
								}
								else if(testdc->choice.unsuccessfulOutcome->procedureCode == 9){ //subscription delete Failure
									RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
									RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
									RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
									if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 1){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricRequest;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 2){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricService;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 3){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.transport;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 4){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.protocol;
									}
									else{
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.misc;
									}
									
									messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
									mtype = 5044400;
									message1 = 8;
                                    printf ("Receive [RIC Subscription Delete Failure]\n");
								}
								else if(testdc->choice.unsuccessfulOutcome->procedureCode == 4){ //Control Failure
									if(testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 1){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricRequest;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 2){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricService;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 3){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.transport;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 4){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.protocol;
										}
										else{
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.misc;
										}
										
										messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
										mtype = 6044200;
										message1 = 9;
                                        printf ("Receive [RIC Control Failure]\n");
									}
									if(testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.count == 4){
										RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 1){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.ricRequest;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 2){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.ricService;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 3){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.transport;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 4){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.protocol;
										}
										else{
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.misc;
										}
										
										messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
										mtype = 6044200;
										message1 = 10;
                                        printf ("Receive [RIC Control Failure (Insert)]\n");
									}
                                }
								else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
						
							if(senderror == 1){
								asn_enc_rval_t Bytestring = asn_encode_to_buffer(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, ErrorInd, bufferCh, bufferCh_size);

								int j=0 ;
								size_t Bytestring_size = Bytestring.encoded;

								if (Bytestring.encoded == -1){
									printf("encoding of %s failed %s\n" , "test", strerror(errno));
									exit(-1);
								}else if (Bytestring.encoded > (ssize_t) bufferCh_size) {
									printf("Buffer of size %ld is to small for %s\n" , bufferCh_size , "test");
									exit(-1);
								}else{
									memcpy(buffersend, bufferCh, Bytestring_size);
								}
							
								/////////////////////////////////////////////////////////////////////////////////////////////////////////////
								ret = sctp_sendmsg (sctpParams->connSock[5], (void *)buffersend, (size_t) Bytestring_size,
								NULL, 0, 0, 0, 0, 0, 0);
								if(ret == -1 ){
									printf("Error in sctp_sendmsg\n");
									perror("sctp_sendmsg()");
								}
								else
									printf("Successfully sent %d bytes data to E2 Agent\n", ret);
								
								senderror = 0;
								goto LOOP5;
							}

                            free(ErrorInd);
                            free(testdc);
						}

						//--------------------------------------------------------------------------------------rmr------------------------------------------------------------------------------------------//

						fprintf( stderr, "<DEMO> listen port: %s; mtype: %d; delay: %d\n",
							listen_port, mtype, delay );

						sbuf = rmr_alloc_msg( mrc, 2048);	// alloc 1st send buf; subsequent bufs alloc on send
						rbuf = NULL;						// don't need to alloc receive buffer

						while( ! rmr_ready( mrc ) ) {		// must have route table
							sleep( 1 );						// wait til we get one
						}
						fprintf( stderr, "<DEMO> rmr is ready\n" );

						int payloadlength = strlen(sbuf->payload);
						bzero (sbuf->payload, payloadlength);
						if(message1 == 1){         //setup
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"E2 Node ID\": \"%ld %ld %ld %ld\",\"RAN Function Definition\": 0,\"RAN Function ID\": %ld,\"RAN Function Revision\": 1} ",	// create the payload
								sctpParams->connSock[5], messagetype , E2NodeID3, E2NodeID2, E2NodeID1 , E2NodeID0 , RANFunctionID);
						}
						else if(message1 == 2){         //indication
							snprintf( sbuf->payload, 1500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"RIC Indication Type\": %ld,\"RIC Indication Header\": 1,\"RIC Indication Message\": %s} ",	// create the payload
								sctpParams->connSock[5],messagetype , RICRequestorID ,RICInstanceID , RANFunctionID , RICActionID, RICIndicationType,bufferinmessage);
						}
						else if(message1 == 3){         //subscription
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RAN Function Revision\": %ld,\"RIC Action ID\": %ld} ",	// create the payload
								sctpParams->connSock[5],messagetype ,RICRequestorID, RICInstanceID,RANFunctionID, RANFunctionRevision,RICActionID);
						}
						else if(message1 == 4){         //control
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Control Status\": %ld} ",	// create the payload
								sctpParams->connSock[5],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,  RICControlStatus);
						}
						else if(message1 == 5){         //indication (insert)
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"RIC Indication Type\": %ld,\"RIC Indication Header\": 1,\"RIC Indication Message\": 1,\"RIC Call process ID\": 1} ",	// create the payload
								sctpParams->connSock[5],messagetype , RICRequestorID ,RICInstanceID , RANFunctionID , RICActionID, RICIndicationType);
						}
						else if(message1 == 6){         //subscription delete
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld} ",	// create the payload
								sctpParams->connSock[5],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID);
						}
						else if(message1 == 7){         //Subscription Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[5],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,RICActionID,Cause);
						}
						else if(message1 == 8){         //Subscription Delete Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[5],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else if(message1 == 9){         //Control Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[5],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else if(message1 == 10){         //Control Failure(CallProcessID)
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Call process ID\": 1,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[5],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else{
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": nothing} ",sctpParams->connSock[5]);	// create the payload
						}

                        sbuf->mtype = mtype;							// fill in the message bits
                        sbuf->len =  strlen( sbuf->payload ) + 1;		// send full ascii-z string
                        sbuf->state = 0;
                        sbuf = rmr_send_msg( mrc, sbuf );				// send & get next buf to fill in
                        while( sbuf->state == RMR_ERR_RETRY ) {			// soft failure (device busy?) retry
                            sbuf = rmr_send_msg( mrc, sbuf );			// w/ simple spin that doesn't give up
                        }

                        rmr_free_msg(sbuf);
					}while(1);
				}
			}
			else if(countE2Node == 6){
				if((childpid6 = fork()) == 0){
					LOOP6:do{
						rmr_mbuf_t*	sbuf;				// send buffer
                        rmr_mbuf_t*	rbuf;				// received buffer
						bzero (buffer, MAX_BUFFER + 1);
						in = sctp_recvmsg (sctpParams->connSock[6], buffer, sizeof (buffer), (struct sockaddr *) NULL, 0, &sndrcvinfo, &flags);
						if( in == -1){
							printf("Error in sctp_recvmsg\n");
							perror("sctp_recvmsg()");
							close(sctpParams->connSock[6]);
							continue;
						}
						else if(in == 0){
							continue;
						}
						else{
							printf ("Length of Data received: %d\n", in);

							E2AP_PDU_t *ErrorInd = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));
							E2AP_PDU_t *testdc = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));
							asn_decode(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, (void **)&testdc, buffer, in);

							printf ("what message(ini or suc or unsuc): %d\n", testdc->present);//initiatingMessage=1 or successfulOutcome=2 or unsuccessfulOutcome=3
							
                            //initiatingMessage=1
                            if(testdc->present == 1){    
								if(testdc->choice.successfulOutcome->procedureCode == 1){
									//E2 Setup Request
									if(testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.count == 2){
										E2NodeID0 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[0];
										E2NodeID1 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[1];
										E2NodeID2 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[2];
										E2NodeID3 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[3];
										RANfunction_ItemIEs_t * RANfunction_ItemIEs = (RANfunction_ItemIEs_t * ) testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[1]->value.choice.RANfunctions_List.list.array[0];
										RANFunctionID = RANfunction_ItemIEs->value.choice.RANfunction_Item.ranFunctionID;
										RANFunctionRevision = RANfunction_ItemIEs->value.choice.RANfunction_Item.ranFunctionRevision;

										messagetype = testdc->choice.initiatingMessage->procedureCode;
										mtype = 2004200;
										message1 = 1;
                                        printf ("Receive [E2 Setup Request]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 5){
									//indication
									if(testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.count == 7){
										RICRequestorID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;				
										RICInstanceID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICActionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[2]->value.choice.RICactionID;
										RICIndicationSN = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[3]->value.choice.RICindicationSN;
										RICIndicationType = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[4]->value.choice.RICindicationType;
										bzero(bufferinmessage,2049);
										strcpy(bufferinmessage,testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[6]->value.choice.RICindicationMessage.buf);
										
										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 7004200;
										message1 = 2;
                                        printf ("Receive [Indication(Report) no. %ld]\n", RICIndicationSN);
									}
									//insert
									if(testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.count == 8){
										RICRequestorID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICActionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[2]->value.choice.RICactionID;
										RICIndicationSN = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[3]->value.choice.RICindicationSN;
										RICIndicationType = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[4]->value.choice.RICindicationType;

										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 7104200;
										message1 = 5;
                                        printf ("Receive [Indication(Insert) no. %ld]\n", RICIndicationSN);              
									}
								}
                                else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
							else if(testdc->present == 2){   //successfulOutcome=2
								if(testdc->choice.successfulOutcome->procedureCode == 8){
									//Subscription Response
									if(testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICaction_Admitted_ItemIEs_t * RICaction_Admitted_ItemIEs = (RICaction_Admitted_ItemIEs_t * ) testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[2]->value.choice.RICaction_Admitted_List.list.array[0];
										RICActionID = RICaction_Admitted_ItemIEs->value.choice.RICaction_Admitted_Item.ricActionID;

										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 4014400;
										message1 = 3;
                                        printf ("Receive [RIC Subscription Response]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 4){
									//Control
									if(testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID =  testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICControlStatus =  testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[2]->value.choice.RICcontrolStatus;
										
                                        messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 6014200;
										message1 = 4;
                                        printf ("Receive [RIC Control Ack]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 9){
									//subscription delete
									if(testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.count == 2){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID =  testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										
                                        messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 5014400;
										message1 = 6;
                                        printf ("Receive [RIC Subscription Delete Response]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
							else{   //unsuccessfulOutcome=3
								//subscription Failure
								if(testdc->choice.unsuccessfulOutcome->procedureCode == 8){ 
									RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
									RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
									RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
									RICaction_NotAdmitted_ItemIEs_t * RICaction_NotAdmitted_ItemIEs = (RICaction_NotAdmitted_ItemIEs_t * ) testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[2]->value.choice.RICaction_NotAdmitted_List.list.array[0];
									RICActionID = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.ricActionID;
									if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 1){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.ricRequest;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 2){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.ricService;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 3){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.transport;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 4){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.protocol;
									}
									else{
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.misc;
									}
									
									messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
									mtype = 4044400;
									message1 = 7;
                                    printf ("Receive [RIC Subscription Failure]\n");
								}
								else if(testdc->choice.unsuccessfulOutcome->procedureCode == 9){ //subscription delete Failure
									RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
									RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
									RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
									if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 1){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricRequest;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 2){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricService;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 3){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.transport;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 4){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.protocol;
									}
									else{
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.misc;
									}
									
									messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
									mtype = 5044400;
									message1 = 8;
                                    printf ("Receive [RIC Subscription Delete Failure]\n");
								}
								else if(testdc->choice.unsuccessfulOutcome->procedureCode == 4){ //Control Failure
									if(testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 1){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricRequest;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 2){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricService;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 3){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.transport;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 4){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.protocol;
										}
										else{
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.misc;
										}
										
										messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
										mtype = 6044200;
										message1 = 9;
                                        printf ("Receive [RIC Control Failure]\n");
									}
									if(testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.count == 4){
										RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 1){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.ricRequest;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 2){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.ricService;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 3){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.transport;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 4){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.protocol;
										}
										else{
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.misc;
										}
										
										messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
										mtype = 6044200;
										message1 = 10;
                                        printf ("Receive [RIC Control Failure (Insert)]\n");
									}
                                }
								else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
						
							if(senderror == 1){
								asn_enc_rval_t Bytestring = asn_encode_to_buffer(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, ErrorInd, bufferCh, bufferCh_size);

								int j=0 ;
								size_t Bytestring_size = Bytestring.encoded;

								if (Bytestring.encoded == -1){
									printf("encoding of %s failed %s\n" , "test", strerror(errno));
									exit(-1);
								}else if (Bytestring.encoded > (ssize_t) bufferCh_size) {
									printf("Buffer of size %ld is to small for %s\n" , bufferCh_size , "test");
									exit(-1);
								}else{
									memcpy(buffersend, bufferCh, Bytestring_size);
								}
							
								/////////////////////////////////////////////////////////////////////////////////////////////////////////////
								ret = sctp_sendmsg (sctpParams->connSock[6], (void *)buffersend, (size_t) Bytestring_size,
								NULL, 0, 0, 0, 0, 0, 0);
								if(ret == -1 ){
									printf("Error in sctp_sendmsg\n");
									perror("sctp_sendmsg()");
								}
								else
									printf("Successfully sent %d bytes data to E2 Agent\n", ret);
								
								senderror = 0;
								goto LOOP6;
							}

                            free(ErrorInd);
                            free(testdc);
						}

						//--------------------------------------------------------------------------------------rmr------------------------------------------------------------------------------------------//

						fprintf( stderr, "<DEMO> listen port: %s; mtype: %d; delay: %d\n",
							listen_port, mtype, delay );

						sbuf = rmr_alloc_msg( mrc, 2048);	// alloc 1st send buf; subsequent bufs alloc on send
						rbuf = NULL;						// don't need to alloc receive buffer

						while( ! rmr_ready( mrc ) ) {		// must have route table
							sleep( 1 );						// wait til we get one
						}
						fprintf( stderr, "<DEMO> rmr is ready\n" );

						int payloadlength = strlen(sbuf->payload);
						bzero (sbuf->payload, payloadlength);
						if(message1 == 1){         //setup
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"E2 Node ID\": \"%ld %ld %ld %ld\",\"RAN Function Definition\": 0,\"RAN Function ID\": %ld,\"RAN Function Revision\": 1} ",	// create the payload
								sctpParams->connSock[6], messagetype , E2NodeID3, E2NodeID2, E2NodeID1 , E2NodeID0 , RANFunctionID);
						}
						else if(message1 == 2){         //indication
							snprintf( sbuf->payload, 1500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"RIC Indication Type\": %ld,\"RIC Indication Header\": 1,\"RIC Indication Message\": %s} ",	// create the payload
								sctpParams->connSock[6],messagetype , RICRequestorID ,RICInstanceID , RANFunctionID , RICActionID, RICIndicationType,bufferinmessage);
						}
						else if(message1 == 3){         //subscription
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RAN Function Revision\": %ld,\"RIC Action ID\": %ld} ",	// create the payload
								sctpParams->connSock[6],messagetype ,RICRequestorID, RICInstanceID,RANFunctionID, RANFunctionRevision,RICActionID);
						}
						else if(message1 == 4){         //control
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Control Status\": %ld} ",	// create the payload
								sctpParams->connSock[6],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,  RICControlStatus);
						}
						else if(message1 == 5){         //indication (insert)
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"RIC Indication Type\": %ld,\"RIC Indication Header\": 1,\"RIC Indication Message\": 1,\"RIC Call process ID\": 1} ",	// create the payload
								sctpParams->connSock[6],messagetype , RICRequestorID ,RICInstanceID , RANFunctionID , RICActionID, RICIndicationType);
						}
						else if(message1 == 6){         //subscription delete
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld} ",	// create the payload
								sctpParams->connSock[6],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID);
						}
						else if(message1 == 7){         //Subscription Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[6],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,RICActionID,Cause);
						}
						else if(message1 == 8){         //Subscription Delete Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[6],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else if(message1 == 9){         //Control Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[6],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else if(message1 == 10){         //Control Failure(CallProcessID)
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Call process ID\": 1,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[6],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else{
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": nothing} ",sctpParams->connSock[6]);	// create the payload
						}

                        sbuf->mtype = mtype;							// fill in the message bits
                        sbuf->len =  strlen( sbuf->payload ) + 1;		// send full ascii-z string
                        sbuf->state = 0;
                        sbuf = rmr_send_msg( mrc, sbuf );				// send & get next buf to fill in
                        while( sbuf->state == RMR_ERR_RETRY ) {			// soft failure (device busy?) retry
                            sbuf = rmr_send_msg( mrc, sbuf );			// w/ simple spin that doesn't give up
                        }

                        rmr_free_msg(sbuf);
					}while(1);
				}
			}
			else if(countE2Node == 7){
				if((childpid7 = fork()) == 0){
					LOOP7:do{
						rmr_mbuf_t*	sbuf;				// send buffer
                        rmr_mbuf_t*	rbuf;				// received buffer
						bzero (buffer, MAX_BUFFER + 1);
						in = sctp_recvmsg (sctpParams->connSock[7], buffer, sizeof (buffer), (struct sockaddr *) NULL, 0, &sndrcvinfo, &flags);
						if( in == -1){
							printf("Error in sctp_recvmsg\n");
							perror("sctp_recvmsg()");
							close(sctpParams->connSock[7]);
							continue;
						}
						else if(in == 0){
							continue;
						}
						else{
							printf ("Length of Data received: %d\n", in);

							E2AP_PDU_t *ErrorInd = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));
							E2AP_PDU_t *testdc = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));
							asn_decode(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, (void **)&testdc, buffer, in);

							printf ("what message(ini or suc or unsuc): %d\n", testdc->present);//initiatingMessage=1 or successfulOutcome=2 or unsuccessfulOutcome=3
							
                            //initiatingMessage=1
                            if(testdc->present == 1){    
								if(testdc->choice.successfulOutcome->procedureCode == 1){
									//E2 Setup Request
									if(testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.count == 2){
										E2NodeID0 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[0];
										E2NodeID1 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[1];
										E2NodeID2 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[2];
										E2NodeID3 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[3];
										RANfunction_ItemIEs_t * RANfunction_ItemIEs = (RANfunction_ItemIEs_t * ) testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[1]->value.choice.RANfunctions_List.list.array[0];
										RANFunctionID = RANfunction_ItemIEs->value.choice.RANfunction_Item.ranFunctionID;
										RANFunctionRevision = RANfunction_ItemIEs->value.choice.RANfunction_Item.ranFunctionRevision;

										messagetype = testdc->choice.initiatingMessage->procedureCode;
										mtype = 2004200;
										message1 = 1;
                                        printf ("Receive [E2 Setup Request]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 5){
									//indication
									if(testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.count == 7){
										RICRequestorID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;				
										RICInstanceID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICActionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[2]->value.choice.RICactionID;
										RICIndicationSN = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[3]->value.choice.RICindicationSN;
										RICIndicationType = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[4]->value.choice.RICindicationType;
										bzero(bufferinmessage,2049);
										strcpy(bufferinmessage,testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[6]->value.choice.RICindicationMessage.buf);
										
										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 7004200;
										message1 = 2;
                                        printf ("Receive [Indication(Report) no. %ld]\n", RICIndicationSN);
									}
									//insert
									if(testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.count == 8){
										RICRequestorID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICActionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[2]->value.choice.RICactionID;
										RICIndicationSN = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[3]->value.choice.RICindicationSN;
										RICIndicationType = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[4]->value.choice.RICindicationType;

										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 7104200;
										message1 = 5;
                                        printf ("Receive [Indication(Insert) no. %ld]\n", RICIndicationSN);              
									}
								}
                                else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
							else if(testdc->present == 2){   //successfulOutcome=2
								if(testdc->choice.successfulOutcome->procedureCode == 8){
									//Subscription Response
									if(testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICaction_Admitted_ItemIEs_t * RICaction_Admitted_ItemIEs = (RICaction_Admitted_ItemIEs_t * ) testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[2]->value.choice.RICaction_Admitted_List.list.array[0];
										RICActionID = RICaction_Admitted_ItemIEs->value.choice.RICaction_Admitted_Item.ricActionID;

										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 4014400;
										message1 = 3;
                                        printf ("Receive [RIC Subscription Response]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 4){
									//Control
									if(testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID =  testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICControlStatus =  testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[2]->value.choice.RICcontrolStatus;
										
                                        messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 6014200;
										message1 = 4;
                                        printf ("Receive [RIC Control Ack]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 9){
									//subscription delete
									if(testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.count == 2){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID =  testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										
                                        messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 5014400;
										message1 = 6;
                                        printf ("Receive [RIC Subscription Delete Response]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
							else{   //unsuccessfulOutcome=3
								//subscription Failure
								if(testdc->choice.unsuccessfulOutcome->procedureCode == 8){ 
									RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
									RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
									RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
									RICaction_NotAdmitted_ItemIEs_t * RICaction_NotAdmitted_ItemIEs = (RICaction_NotAdmitted_ItemIEs_t * ) testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[2]->value.choice.RICaction_NotAdmitted_List.list.array[0];
									RICActionID = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.ricActionID;
									if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 1){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.ricRequest;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 2){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.ricService;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 3){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.transport;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 4){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.protocol;
									}
									else{
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.misc;
									}
									
									messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
									mtype = 4044400;
									message1 = 7;
                                    printf ("Receive [RIC Subscription Failure]\n");
								}
								else if(testdc->choice.unsuccessfulOutcome->procedureCode == 9){ //subscription delete Failure
									RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
									RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
									RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
									if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 1){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricRequest;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 2){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricService;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 3){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.transport;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 4){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.protocol;
									}
									else{
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.misc;
									}
									
									messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
									mtype = 5044400;
									message1 = 8;
                                    printf ("Receive [RIC Subscription Delete Failure]\n");
								}
								else if(testdc->choice.unsuccessfulOutcome->procedureCode == 4){ //Control Failure
									if(testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 1){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricRequest;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 2){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricService;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 3){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.transport;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 4){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.protocol;
										}
										else{
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.misc;
										}
										
										messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
										mtype = 6044200;
										message1 = 9;
                                        printf ("Receive [RIC Control Failure]\n");
									}
									if(testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.count == 4){
										RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 1){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.ricRequest;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 2){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.ricService;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 3){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.transport;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 4){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.protocol;
										}
										else{
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.misc;
										}
										
										messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
										mtype = 6044200;
										message1 = 10;
                                        printf ("Receive [RIC Control Failure (Insert)]\n");
									}
                                }
								else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
						
							if(senderror == 1){
								asn_enc_rval_t Bytestring = asn_encode_to_buffer(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, ErrorInd, bufferCh, bufferCh_size);

								int j=0 ;
								size_t Bytestring_size = Bytestring.encoded;

								if (Bytestring.encoded == -1){
									printf("encoding of %s failed %s\n" , "test", strerror(errno));
									exit(-1);
								}else if (Bytestring.encoded > (ssize_t) bufferCh_size) {
									printf("Buffer of size %ld is to small for %s\n" , bufferCh_size , "test");
									exit(-1);
								}else{
									memcpy(buffersend, bufferCh, Bytestring_size);
								}
							
								/////////////////////////////////////////////////////////////////////////////////////////////////////////////
								ret = sctp_sendmsg (sctpParams->connSock[7], (void *)buffersend, (size_t) Bytestring_size,
								NULL, 0, 0, 0, 0, 0, 0);
								if(ret == -1 ){
									printf("Error in sctp_sendmsg\n");
									perror("sctp_sendmsg()");
								}
								else
									printf("Successfully sent %d bytes data to E2 Agent\n", ret);
								
								senderror = 0;
								goto LOOP7;
							}

                            free(ErrorInd);
                            free(testdc);
						}

						//--------------------------------------------------------------------------------------rmr------------------------------------------------------------------------------------------//

						fprintf( stderr, "<DEMO> listen port: %s; mtype: %d; delay: %d\n",
							listen_port, mtype, delay );

						sbuf = rmr_alloc_msg( mrc, 2048);	// alloc 1st send buf; subsequent bufs alloc on send
						rbuf = NULL;						// don't need to alloc receive buffer

						while( ! rmr_ready( mrc ) ) {		// must have route table
							sleep( 1 );						// wait til we get one
						}
						fprintf( stderr, "<DEMO> rmr is ready\n" );

						int payloadlength = strlen(sbuf->payload);
						bzero (sbuf->payload, payloadlength);
						if(message1 == 1){         //setup
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"E2 Node ID\": \"%ld %ld %ld %ld\",\"RAN Function Definition\": 0,\"RAN Function ID\": %ld,\"RAN Function Revision\": 1} ",	// create the payload
								sctpParams->connSock[7], messagetype , E2NodeID3, E2NodeID2, E2NodeID1 , E2NodeID0 , RANFunctionID);
						}
						else if(message1 == 2){         //indication
							snprintf( sbuf->payload, 1500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"RIC Indication Type\": %ld,\"RIC Indication Header\": 1,\"RIC Indication Message\": %s} ",	// create the payload
								sctpParams->connSock[7],messagetype , RICRequestorID ,RICInstanceID , RANFunctionID , RICActionID, RICIndicationType,bufferinmessage);
						}
						else if(message1 == 3){         //subscription
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RAN Function Revision\": %ld,\"RIC Action ID\": %ld} ",	// create the payload
								sctpParams->connSock[7],messagetype ,RICRequestorID, RICInstanceID,RANFunctionID, RANFunctionRevision,RICActionID);
						}
						else if(message1 == 4){         //control
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Control Status\": %ld} ",	// create the payload
								sctpParams->connSock[7],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,  RICControlStatus);
						}
						else if(message1 == 5){         //indication (insert)
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"RIC Indication Type\": %ld,\"RIC Indication Header\": 1,\"RIC Indication Message\": 1,\"RIC Call process ID\": 1} ",	// create the payload
								sctpParams->connSock[7],messagetype , RICRequestorID ,RICInstanceID , RANFunctionID , RICActionID, RICIndicationType);
						}
						else if(message1 == 6){         //subscription delete
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld} ",	// create the payload
								sctpParams->connSock[7],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID);
						}
						else if(message1 == 7){         //Subscription Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[7],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,RICActionID,Cause);
						}
						else if(message1 == 8){         //Subscription Delete Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[7],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else if(message1 == 9){         //Control Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[7],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else if(message1 == 10){         //Control Failure(CallProcessID)
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Call process ID\": 1,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[7],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else{
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": nothing} ",sctpParams->connSock[7]);	// create the payload
						}

                        sbuf->mtype = mtype;							// fill in the message bits
                        sbuf->len =  strlen( sbuf->payload ) + 1;		// send full ascii-z string
                        sbuf->state = 0;
                        sbuf = rmr_send_msg( mrc, sbuf );				// send & get next buf to fill in
                        while( sbuf->state == RMR_ERR_RETRY ) {			// soft failure (device busy?) retry
                            sbuf = rmr_send_msg( mrc, sbuf );			// w/ simple spin that doesn't give up
                        }

                        rmr_free_msg(sbuf);
					}while(1);
				}
			}
			else if(countE2Node == 8){
				if((childpid8 = fork()) == 0){
					LOOP8:do{
						rmr_mbuf_t*	sbuf;				// send buffer
                        rmr_mbuf_t*	rbuf;				// received buffer
						bzero (buffer, MAX_BUFFER + 1);
						in = sctp_recvmsg (sctpParams->connSock[8], buffer, sizeof (buffer), (struct sockaddr *) NULL, 0, &sndrcvinfo, &flags);
						if( in == -1){
							printf("Error in sctp_recvmsg\n");
							perror("sctp_recvmsg()");
							close(sctpParams->connSock[8]);
							continue;
						}
						else if(in == 0){
							continue;
						}
						else{
							printf ("Length of Data received: %d\n", in);

							E2AP_PDU_t *ErrorInd = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));
							E2AP_PDU_t *testdc = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));
							asn_decode(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, (void **)&testdc, buffer, in);

							printf ("what message(ini or suc or unsuc): %d\n", testdc->present);//initiatingMessage=1 or successfulOutcome=2 or unsuccessfulOutcome=3
							
                            //initiatingMessage=1
                            if(testdc->present == 1){    
								if(testdc->choice.successfulOutcome->procedureCode == 1){
									//E2 Setup Request
									if(testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.count == 2){
										E2NodeID0 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[0];
										E2NodeID1 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[1];
										E2NodeID2 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[2];
										E2NodeID3 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[3];
										RANfunction_ItemIEs_t * RANfunction_ItemIEs = (RANfunction_ItemIEs_t * ) testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[1]->value.choice.RANfunctions_List.list.array[0];
										RANFunctionID = RANfunction_ItemIEs->value.choice.RANfunction_Item.ranFunctionID;
										RANFunctionRevision = RANfunction_ItemIEs->value.choice.RANfunction_Item.ranFunctionRevision;

										messagetype = testdc->choice.initiatingMessage->procedureCode;
										mtype = 2004200;
										message1 = 1;
                                        printf ("Receive [E2 Setup Request]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 5){
									//indication
									if(testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.count == 7){
										RICRequestorID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;				
										RICInstanceID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICActionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[2]->value.choice.RICactionID;
										RICIndicationSN = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[3]->value.choice.RICindicationSN;
										RICIndicationType = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[4]->value.choice.RICindicationType;
										bzero(bufferinmessage,2049);
										strcpy(bufferinmessage,testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[6]->value.choice.RICindicationMessage.buf);
										
										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 7004200;
										message1 = 2;
                                        printf ("Receive [Indication(Report) no. %ld]\n", RICIndicationSN);
									}
									//insert
									if(testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.count == 8){
										RICRequestorID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICActionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[2]->value.choice.RICactionID;
										RICIndicationSN = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[3]->value.choice.RICindicationSN;
										RICIndicationType = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[4]->value.choice.RICindicationType;

										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 7104200;
										message1 = 5;
                                        printf ("Receive [Indication(Insert) no. %ld]\n", RICIndicationSN);              
									}
								}
                                else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
							else if(testdc->present == 2){   //successfulOutcome=2
								if(testdc->choice.successfulOutcome->procedureCode == 8){
									//Subscription Response
									if(testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICaction_Admitted_ItemIEs_t * RICaction_Admitted_ItemIEs = (RICaction_Admitted_ItemIEs_t * ) testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[2]->value.choice.RICaction_Admitted_List.list.array[0];
										RICActionID = RICaction_Admitted_ItemIEs->value.choice.RICaction_Admitted_Item.ricActionID;

										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 4014400;
										message1 = 3;
                                        printf ("Receive [RIC Subscription Response]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 4){
									//Control
									if(testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID =  testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICControlStatus =  testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[2]->value.choice.RICcontrolStatus;
										
                                        messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 6014200;
										message1 = 4;
                                        printf ("Receive [RIC Control Ack]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 9){
									//subscription delete
									if(testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.count == 2){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID =  testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										
                                        messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 5014400;
										message1 = 6;
                                        printf ("Receive [RIC Subscription Delete Response]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
							else{   //unsuccessfulOutcome=3
								//subscription Failure
								if(testdc->choice.unsuccessfulOutcome->procedureCode == 8){ 
									RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
									RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
									RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
									RICaction_NotAdmitted_ItemIEs_t * RICaction_NotAdmitted_ItemIEs = (RICaction_NotAdmitted_ItemIEs_t * ) testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[2]->value.choice.RICaction_NotAdmitted_List.list.array[0];
									RICActionID = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.ricActionID;
									if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 1){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.ricRequest;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 2){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.ricService;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 3){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.transport;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 4){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.protocol;
									}
									else{
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.misc;
									}
									
									messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
									mtype = 4044400;
									message1 = 7;
                                    printf ("Receive [RIC Subscription Failure]\n");
								}
								else if(testdc->choice.unsuccessfulOutcome->procedureCode == 9){ //subscription delete Failure
									RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
									RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
									RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
									if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 1){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricRequest;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 2){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricService;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 3){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.transport;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 4){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.protocol;
									}
									else{
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.misc;
									}
									
									messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
									mtype = 5044400;
									message1 = 8;
                                    printf ("Receive [RIC Subscription Delete Failure]\n");
								}
								else if(testdc->choice.unsuccessfulOutcome->procedureCode == 4){ //Control Failure
									if(testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 1){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricRequest;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 2){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricService;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 3){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.transport;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 4){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.protocol;
										}
										else{
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.misc;
										}
										
										messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
										mtype = 6044200;
										message1 = 9;
                                        printf ("Receive [RIC Control Failure]\n");
									}
									if(testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.count == 4){
										RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 1){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.ricRequest;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 2){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.ricService;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 3){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.transport;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 4){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.protocol;
										}
										else{
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.misc;
										}
										
										messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
										mtype = 6044200;
										message1 = 10;
                                        printf ("Receive [RIC Control Failure (Insert)]\n");
									}
                                }
								else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
						
							if(senderror == 1){
								asn_enc_rval_t Bytestring = asn_encode_to_buffer(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, ErrorInd, bufferCh, bufferCh_size);

								int j=0 ;
								size_t Bytestring_size = Bytestring.encoded;

								if (Bytestring.encoded == -1){
									printf("encoding of %s failed %s\n" , "test", strerror(errno));
									exit(-1);
								}else if (Bytestring.encoded > (ssize_t) bufferCh_size) {
									printf("Buffer of size %ld is to small for %s\n" , bufferCh_size , "test");
									exit(-1);
								}else{
									memcpy(buffersend, bufferCh, Bytestring_size);
								}
							
								/////////////////////////////////////////////////////////////////////////////////////////////////////////////
								ret = sctp_sendmsg (sctpParams->connSock[8], (void *)buffersend, (size_t) Bytestring_size,
								NULL, 0, 0, 0, 0, 0, 0);
								if(ret == -1 ){
									printf("Error in sctp_sendmsg\n");
									perror("sctp_sendmsg()");
								}
								else
									printf("Successfully sent %d bytes data to E2 Agent\n", ret);
								
								senderror = 0;
								goto LOOP8;
							}

                            free(ErrorInd);
                            free(testdc);
						}

						//--------------------------------------------------------------------------------------rmr------------------------------------------------------------------------------------------//

						fprintf( stderr, "<DEMO> listen port: %s; mtype: %d; delay: %d\n",
							listen_port, mtype, delay );

						sbuf = rmr_alloc_msg( mrc, 2048);	// alloc 1st send buf; subsequent bufs alloc on send
						rbuf = NULL;						// don't need to alloc receive buffer

						while( ! rmr_ready( mrc ) ) {		// must have route table
							sleep( 1 );						// wait til we get one
						}
						fprintf( stderr, "<DEMO> rmr is ready\n" );

						int payloadlength = strlen(sbuf->payload);
						bzero (sbuf->payload, payloadlength);
						if(message1 == 1){         //setup
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"E2 Node ID\": \"%ld %ld %ld %ld\",\"RAN Function Definition\": 0,\"RAN Function ID\": %ld,\"RAN Function Revision\": 1} ",	// create the payload
								sctpParams->connSock[8], messagetype , E2NodeID3, E2NodeID2, E2NodeID1 , E2NodeID0 , RANFunctionID);
						}
						else if(message1 == 2){         //indication
							snprintf( sbuf->payload, 1500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"RIC Indication Type\": %ld,\"RIC Indication Header\": 1,\"RIC Indication Message\": %s} ",	// create the payload
								sctpParams->connSock[8],messagetype , RICRequestorID ,RICInstanceID , RANFunctionID , RICActionID, RICIndicationType,bufferinmessage);
						}
						else if(message1 == 3){         //subscription
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RAN Function Revision\": %ld,\"RIC Action ID\": %ld} ",	// create the payload
								sctpParams->connSock[8],messagetype ,RICRequestorID, RICInstanceID,RANFunctionID, RANFunctionRevision,RICActionID);
						}
						else if(message1 == 4){         //control
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Control Status\": %ld} ",	// create the payload
								sctpParams->connSock[8],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,  RICControlStatus);
						}
						else if(message1 == 5){         //indication (insert)
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"RIC Indication Type\": %ld,\"RIC Indication Header\": 1,\"RIC Indication Message\": 1,\"RIC Call process ID\": 1} ",	// create the payload
								sctpParams->connSock[8],messagetype , RICRequestorID ,RICInstanceID , RANFunctionID , RICActionID, RICIndicationType);
						}
						else if(message1 == 6){         //subscription delete
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld} ",	// create the payload
								sctpParams->connSock[8],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID);
						}
						else if(message1 == 7){         //Subscription Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[8],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,RICActionID,Cause);
						}
						else if(message1 == 8){         //Subscription Delete Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[8],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else if(message1 == 9){         //Control Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[8],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else if(message1 == 10){         //Control Failure(CallProcessID)
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Call process ID\": 1,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[8],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else{
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": nothing} ",sctpParams->connSock[8]);	// create the payload
						}

                        sbuf->mtype = mtype;							// fill in the message bits
                        sbuf->len =  strlen( sbuf->payload ) + 1;		// send full ascii-z string
                        sbuf->state = 0;
                        sbuf = rmr_send_msg( mrc, sbuf );				// send & get next buf to fill in
                        while( sbuf->state == RMR_ERR_RETRY ) {			// soft failure (device busy?) retry
                            sbuf = rmr_send_msg( mrc, sbuf );			// w/ simple spin that doesn't give up
                        }

                        rmr_free_msg(sbuf);
					}while(1);
				}
			}
			else if(countE2Node == 9){
				if((childpid9 = fork()) == 0){
					LOOP9:do{
						rmr_mbuf_t*	sbuf;				// send buffer
                        rmr_mbuf_t*	rbuf;				// received buffer
						bzero (buffer, MAX_BUFFER + 1);
						in = sctp_recvmsg (sctpParams->connSock[9], buffer, sizeof (buffer), (struct sockaddr *) NULL, 0, &sndrcvinfo, &flags);
						if( in == -1){
							printf("Error in sctp_recvmsg\n");
							perror("sctp_recvmsg()");
							close(sctpParams->connSock[9]);
							continue;
						}
						else if(in == 0){
							continue;
						}
						else{
							printf ("Length of Data received: %d\n", in);

							E2AP_PDU_t *ErrorInd = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));
							E2AP_PDU_t *testdc = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));
							asn_decode(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, (void **)&testdc, buffer, in);

							printf ("what message(ini or suc or unsuc): %d\n", testdc->present);//initiatingMessage=1 or successfulOutcome=2 or unsuccessfulOutcome=3
							
                            //initiatingMessage=1
                            if(testdc->present == 1){    
								if(testdc->choice.successfulOutcome->procedureCode == 1){
									//E2 Setup Request
									if(testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.count == 2){
										E2NodeID0 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[0];
										E2NodeID1 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[1];
										E2NodeID2 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[2];
										E2NodeID3 = testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[0]->value.choice.GlobalE2node_ID.choice.gNB->global_gNB_ID.gnb_id.choice.gnb_ID.buf[3];
										RANfunction_ItemIEs_t * RANfunction_ItemIEs = (RANfunction_ItemIEs_t * ) testdc->choice.initiatingMessage->value.choice.E2setupRequest.protocolIEs.list.array[1]->value.choice.RANfunctions_List.list.array[0];
										RANFunctionID = RANfunction_ItemIEs->value.choice.RANfunction_Item.ranFunctionID;
										RANFunctionRevision = RANfunction_ItemIEs->value.choice.RANfunction_Item.ranFunctionRevision;

										messagetype = testdc->choice.initiatingMessage->procedureCode;
										mtype = 2004200;
										message1 = 1;
                                        printf ("Receive [E2 Setup Request]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 5){
									//indication
									if(testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.count == 7){
										RICRequestorID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;				
										RICInstanceID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICActionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[2]->value.choice.RICactionID;
										RICIndicationSN = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[3]->value.choice.RICindicationSN;
										RICIndicationType = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[4]->value.choice.RICindicationType;
										bzero(bufferinmessage,2049);
										strcpy(bufferinmessage,testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[6]->value.choice.RICindicationMessage.buf);
										
										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 7004200;
										message1 = 2;
                                        printf ("Receive [Indication(Report) no. %ld]\n", RICIndicationSN);
									}
									//insert
									if(testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.count == 8){
										RICRequestorID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICActionID = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[2]->value.choice.RICactionID;
										RICIndicationSN = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[3]->value.choice.RICindicationSN;
										RICIndicationType = testdc->choice.initiatingMessage->value.choice.RICindication.protocolIEs.list.array[4]->value.choice.RICindicationType;

										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 7104200;
										message1 = 5;
                                        printf ("Receive [Indication(Insert) no. %ld]\n", RICIndicationSN);              
									}
								}
                                else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
							else if(testdc->present == 2){   //successfulOutcome=2
								if(testdc->choice.successfulOutcome->procedureCode == 8){
									//Subscription Response
									if(testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICaction_Admitted_ItemIEs_t * RICaction_Admitted_ItemIEs = (RICaction_Admitted_ItemIEs_t * ) testdc->choice.successfulOutcome->value.choice.RICsubscriptionResponse.protocolIEs.list.array[2]->value.choice.RICaction_Admitted_List.list.array[0];
										RICActionID = RICaction_Admitted_ItemIEs->value.choice.RICaction_Admitted_Item.ricActionID;

										messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 4014400;
										message1 = 3;
                                        printf ("Receive [RIC Subscription Response]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 4){
									//Control
									if(testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID =  testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										RICControlStatus =  testdc->choice.successfulOutcome->value.choice.RICcontrolAcknowledge.protocolIEs.list.array[2]->value.choice.RICcontrolStatus;
										
                                        messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 6014200;
										message1 = 4;
                                        printf ("Receive [RIC Control Ack]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else if(testdc->choice.successfulOutcome->procedureCode == 9){
									//subscription delete
									if(testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.count == 2){
										RICRequestorID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID =  testdc->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										
                                        messagetype = testdc->choice.successfulOutcome->procedureCode;
										mtype = 5014400;
										message1 = 6;
                                        printf ("Receive [RIC Subscription Delete Response]\n");
									}
									else{
										buildErrorIndication(ErrorInd);
										printf ("Send [Error Indication]\n");
										senderror = 1;
									}
								}
								else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
							else{   //unsuccessfulOutcome=3
								//subscription Failure
								if(testdc->choice.unsuccessfulOutcome->procedureCode == 8){ 
									RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
									RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
									RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
									RICaction_NotAdmitted_ItemIEs_t * RICaction_NotAdmitted_ItemIEs = (RICaction_NotAdmitted_ItemIEs_t * ) testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure.protocolIEs.list.array[2]->value.choice.RICaction_NotAdmitted_List.list.array[0];
									RICActionID = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.ricActionID;
									if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 1){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.ricRequest;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 2){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.ricService;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 3){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.transport;
									}
									else if(RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.present == 4){
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.protocol;
									}
									else{
										Cause = RICaction_NotAdmitted_ItemIEs->value.choice.RICaction_NotAdmitted_Item.cause.choice.misc;
									}
									
									messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
									mtype = 4044400;
									message1 = 7;
                                    printf ("Receive [RIC Subscription Failure]\n");
								}
								else if(testdc->choice.unsuccessfulOutcome->procedureCode == 9){ //subscription delete Failure
									RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
									RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
									RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
									if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 1){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricRequest;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 2){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricService;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 3){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.transport;
									}
									else if( testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 4){
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.protocol;
									}
									else{
										Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.misc;
									}
									
									messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
									mtype = 5044400;
									message1 = 8;
                                    printf ("Receive [RIC Subscription Delete Failure]\n");
								}
								else if(testdc->choice.unsuccessfulOutcome->procedureCode == 4){ //Control Failure
									if(testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.count == 3){
										RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 1){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricRequest;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 2){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.ricService;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 3){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.transport;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.present == 4){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.protocol;
										}
										else{
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[2]->value.choice.Cause.choice.misc;
										}
										
										messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
										mtype = 6044200;
										message1 = 9;
                                        printf ("Receive [RIC Control Failure]\n");
									}
									if(testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.count == 4){
										RICRequestorID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricRequestorID;
										RICInstanceID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[0]->value.choice.RICrequestID.ricInstanceID;
										RANFunctionID = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[1]->value.choice.RANfunctionID;
										if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 1){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.ricRequest;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 2){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.ricService;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 3){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.transport;
										}
										else if( testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.present == 4){
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.protocol;
										}
										else{
											Cause = testdc->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure.protocolIEs.list.array[3]->value.choice.Cause.choice.misc;
										}
										
										messagetype = testdc->choice.unsuccessfulOutcome->procedureCode;
										mtype = 6044200;
										message1 = 10;
                                        printf ("Receive [RIC Control Failure (Insert)]\n");
									}
                                }
								else{    //fail
									buildErrorIndication(ErrorInd);
									printf ("Send [Error Indication]\n");
								}
							}
						
							if(senderror == 1){
								asn_enc_rval_t Bytestring = asn_encode_to_buffer(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, ErrorInd, bufferCh, bufferCh_size);

								int j=0 ;
								size_t Bytestring_size = Bytestring.encoded;

								if (Bytestring.encoded == -1){
									printf("encoding of %s failed %s\n" , "test", strerror(errno));
									exit(-1);
								}else if (Bytestring.encoded > (ssize_t) bufferCh_size) {
									printf("Buffer of size %ld is to small for %s\n" , bufferCh_size , "test");
									exit(-1);
								}else{
									memcpy(buffersend, bufferCh, Bytestring_size);
								}
							
								/////////////////////////////////////////////////////////////////////////////////////////////////////////////
								ret = sctp_sendmsg (sctpParams->connSock[9], (void *)buffersend, (size_t) Bytestring_size,
								NULL, 0, 0, 0, 0, 0, 0);
								if(ret == -1 ){
									printf("Error in sctp_sendmsg\n");
									perror("sctp_sendmsg()");
								}
								else
									printf("Successfully sent %d bytes data to E2 Agent\n", ret);
								
								senderror = 0;
								goto LOOP9;
							}

                            free(ErrorInd);
                            free(testdc);
						}

						//--------------------------------------------------------------------------------------rmr------------------------------------------------------------------------------------------//

						fprintf( stderr, "<DEMO> listen port: %s; mtype: %d; delay: %d\n",
							listen_port, mtype, delay );

						sbuf = rmr_alloc_msg( mrc, 2048);	// alloc 1st send buf; subsequent bufs alloc on send
						rbuf = NULL;						// don't need to alloc receive buffer

						while( ! rmr_ready( mrc ) ) {		// must have route table
							sleep( 1 );						// wait til we get one
						}
						fprintf( stderr, "<DEMO> rmr is ready\n" );

						int payloadlength = strlen(sbuf->payload);
						bzero (sbuf->payload, payloadlength);
						if(message1 == 1){         //setup
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"E2 Node ID\": \"%ld %ld %ld %ld\",\"RAN Function Definition\": 0,\"RAN Function ID\": %ld,\"RAN Function Revision\": 1} ",	// create the payload
								sctpParams->connSock[9], messagetype , E2NodeID3, E2NodeID2, E2NodeID1 , E2NodeID0 , RANFunctionID);
						}
						else if(message1 == 2){         //indication
							snprintf( sbuf->payload, 1500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"RIC Indication Type\": %ld,\"RIC Indication Header\": 1,\"RIC Indication Message\": %s} ",	// create the payload
								sctpParams->connSock[9],messagetype , RICRequestorID ,RICInstanceID , RANFunctionID , RICActionID, RICIndicationType,bufferinmessage);
						}
						else if(message1 == 3){         //subscription
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RAN Function Revision\": %ld,\"RIC Action ID\": %ld} ",	// create the payload
								sctpParams->connSock[9],messagetype ,RICRequestorID, RICInstanceID,RANFunctionID, RANFunctionRevision,RICActionID);
						}
						else if(message1 == 4){         //control
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Control Status\": %ld} ",	// create the payload
								sctpParams->connSock[9],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,  RICControlStatus);
						}
						else if(message1 == 5){         //indication (insert)
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"RIC Indication Type\": %ld,\"RIC Indication Header\": 1,\"RIC Indication Message\": 1,\"RIC Call process ID\": 1} ",	// create the payload
								sctpParams->connSock[9],messagetype , RICRequestorID ,RICInstanceID , RANFunctionID , RICActionID, RICIndicationType);
						}
						else if(message1 == 6){         //subscription delete
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld} ",	// create the payload
								sctpParams->connSock[9],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID);
						}
						else if(message1 == 7){         //Subscription Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Action ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[9],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,RICActionID,Cause);
						}
						else if(message1 == 8){         //Subscription Delete Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[9],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else if(message1 == 9){         //Control Failure
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[9],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else if(message1 == 10){         //Control Failure(CallProcessID)
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": %d,\"RIC Request ID\":{\"RIC Requestor ID\": %ld,\"RIC Instance ID\": %ld},\"RAN Function ID\": %ld,\"RIC Call process ID\": 1,\"Cause\": %ld} ",	// create the payload
								sctpParams->connSock[9],messagetype ,RICRequestorID , RICInstanceID,RANFunctionID,Cause);
						}
						else{
							snprintf( sbuf->payload, 500,
								"{\"SocketFD\": %d,\"Procedure Code\": nothing} ",sctpParams->connSock[9]);	// create the payload
						}

                        sbuf->mtype = mtype;							// fill in the message bits
                        sbuf->len =  strlen( sbuf->payload ) + 1;		// send full ascii-z string
                        sbuf->state = 0;
                        sbuf = rmr_send_msg( mrc, sbuf );				// send & get next buf to fill in
                        while( sbuf->state == RMR_ERR_RETRY ) {			// soft failure (device busy?) retry
                            sbuf = rmr_send_msg( mrc, sbuf );			// w/ simple spin that doesn't give up
                        }

                        rmr_free_msg(sbuf);
					}while(1);
				}
			}
			
			countE2Node++;
			goto Connumber;
		}
	}while(countE2Node == 10);
}

int main( int argc, char** argv){

    sctp_params_t sctpParams;
	uint8_t ricid[] = "RIC-ID";
  
    //create thread
    int temp ;
    if(temp=pthread_create(&ntid,NULL,thread, (void*) &sctpParams)!= 0){
            printf("can't create thread: %s\n",strerror(temp));
            return 1;
    }

	int ret_main;
    uint8_t buffersend_main[MAX_BUFFER + 1];
    void* mrc2;                     // msg router context
    int stat_freq2 = 10;            // write stats after reciving this many messages
    char* listen_port = "4000";     // default to what has become the standard RMR port
    long long count = 0;
    long long bad2 = 0;
    long long empty2 = 0;
    int number = 0;

	//JSON parameter
	int value_ProcedureCode,value_SocketFD;
	int value_RANFunctionID, value_RICRequestorID, value_RICInstanceID, value_RICActionID, value_RICActionType, value_RICIndicationType,value_TypeofMessage;

	//control parameter
	int value_Repetition1, value_Repetition2, value_Repetition3, value_Repetition4, value_Repetition5;
    int MCS1, MCS2, MCS3, MCS4, MCS5;
    int HARQ1, HARQ2, HARQ3, HARQ4, HARQ5;
    
    //rmr rec
	if( argc > 1 ) {
	listen_port = argv[1];
	}
	if( argc > 2 ) {
		stat_freq2 = atoi( argv[2] );
	}
	fprintf( stderr, "<DEMO> listening on port: %s\n", listen_port );
	fprintf( stderr, "<DEMO> stats will be reported every %d messages\n", stat_freq2 );

	mrc2 = rmr_init( listen_port, RMR_MAX_RCV_BYTES, RMRFL_NONE );
	if( mrc2 == NULL ) {
		fprintf( stderr, "<DEMO> ABORT:  unable to initialise RMr\n" );
		exit( 1 );
	}

	while( ! rmr_ready( mrc2 ) ) {	// wait for RMR to get a route table
		fprintf( stderr, "<DEMO> waiting for ready\n" );
		sleep( 3 );
	}
	fprintf( stderr, "<DEMO> rmr now shows ready\n" );

	while( 1 ) {
		rmr_mbuf_t* msg2 = NULL;        // message received
		E2AP_PDU_t *SendProcedure = (E2AP_PDU_t *)calloc(1, sizeof(E2AP_PDU_t));

		msg2 = rmr_rcv_msg( mrc2, msg2);        // block until one arrives
		
		if( msg2 ) {
			fprintf(stderr, "Received: %s\n", msg2->payload);
			cJSON * root;
			root = cJSON_Parse(msg2->payload); 
			value_ProcedureCode = cJSON_GetObjectItem(root,"Procedure Code")->valueint; 
			printf( "%d ", value_ProcedureCode );


			if( value_ProcedureCode == 1 ) {
				printf( "Setup " );
				value_SocketFD = cJSON_GetObjectItem(root,"SocketFD")->valueint;
				value_TypeofMessage= cJSON_GetObjectItem(root,"Type of Message")->valueint;
				if( value_TypeofMessage == 1 ) {
					printf("Response\n");
					value_RANFunctionID = cJSON_GetObjectItem(root,"RAN Function ID")->valueint; 

					buildSetupSuccsessfulResponse(SendProcedure,466,92, ricid, value_RANFunctionID);
				}
				else { 
					printf("Failure\n");

					buildSetupUnSuccsessfulResponse(SendProcedure);
				}
			}
			else if( value_ProcedureCode == 4 ) {
				printf( "Control Request\n" );
				cJSON * format;
				cJSON * format1;
				value_SocketFD = cJSON_GetObjectItem(root,"SocketFD")->valueint; 
				format = cJSON_GetObjectItem(root,"RIC Request ID");
				value_RICRequestorID= cJSON_GetObjectItem(format,"RIC Requestor ID")->valueint;
				value_RICInstanceID= cJSON_GetObjectItem(format,"RIC Instance ID")->valueint;
				value_RANFunctionID = cJSON_GetObjectItem(root,"RAN Function ID")->valueint; 

				format1 = cJSON_GetObjectItem(root,"RIC Control Message");
				MCS1= cJSON_GetObjectItem(format1,"MCS1")->valueint;
                MCS2= cJSON_GetObjectItem(format1,"MCS2")->valueint;
                MCS3= cJSON_GetObjectItem(format1,"MCS3")->valueint;
                MCS4= cJSON_GetObjectItem(format1,"MCS4")->valueint;
                MCS5= cJSON_GetObjectItem(format1,"MCS5")->valueint;

				HARQ1= cJSON_GetObjectItem(format1,"HARQ1")->valueint;
                HARQ2= cJSON_GetObjectItem(format1,"HARQ2")->valueint;
                HARQ3= cJSON_GetObjectItem(format1,"HARQ3")->valueint;
                HARQ4= cJSON_GetObjectItem(format1,"HARQ4")->valueint;
                HARQ5= cJSON_GetObjectItem(format1,"HARQ5")->valueint;

				value_Repetition1= cJSON_GetObjectItem(format1,"Repetition1")->valuedouble;
                value_Repetition2= cJSON_GetObjectItem(format1,"Repetition2")->valuedouble;
                value_Repetition3= cJSON_GetObjectItem(format1,"Repetition3")->valuedouble;
                value_Repetition4= cJSON_GetObjectItem(format1,"Repetition4")->valuedouble;
                value_Repetition5= cJSON_GetObjectItem(format1,"Repetition5")->valuedouble;

                buildRICControlRequest(SendProcedure, value_RICRequestorID, value_RICInstanceID, value_RANFunctionID, 
                                                    MCS1, MCS2, MCS3, MCS4, MCS4, 
                                                    HARQ1, HARQ2, HARQ3, HARQ4, HARQ5, 
                                                    value_Repetition1, value_Repetition2, value_Repetition3, value_Repetition4, value_Repetition5);
			}
			else if( value_ProcedureCode == 8 ) {
				printf( "Subscription Request\n" );
				cJSON * format;
				cJSON * format1;
				cJSON * format2;
				value_SocketFD = cJSON_GetObjectItem(root,"SocketFD")->valueint; 
				format = cJSON_GetObjectItem(root,"RIC Request ID");
				value_RICRequestorID= cJSON_GetObjectItem(format,"RIC Requestor ID")->valueint;
				value_RICInstanceID= cJSON_GetObjectItem(format,"RIC Instance ID")->valueint;
				value_RANFunctionID = cJSON_GetObjectItem(root,"RAN Function ID")->valueint; 

				format1 = cJSON_GetObjectItem(root,"RIC Subscription Details");
				format2 = cJSON_GetObjectItem(format1,"Sequence of Actions");
				value_RICActionID= cJSON_GetObjectItem(format2,"RIC Action ID")->valueint;
				value_RICActionType= cJSON_GetObjectItem(format2,"RIC Action Type")->valueint;
				
				buildRICSubscriptionRequest(SendProcedure, value_RICRequestorID, value_RICInstanceID, value_RANFunctionID, value_RICActionID, value_RICActionType);
			}
			else if(value_ProcedureCode == 9){
				printf( "Subscription Delete Request\n" );
				cJSON * format;
				value_SocketFD = cJSON_GetObjectItem(root,"SocketFD")->valueint; 
				format = cJSON_GetObjectItem(root,"RIC Request ID");
				value_RICRequestorID= cJSON_GetObjectItem(format,"RIC Requestor ID")->valueint;
				value_RICInstanceID= cJSON_GetObjectItem(format,"RIC Instance ID")->valueint;
				value_RANFunctionID = cJSON_GetObjectItem(root,"RAN Function ID")->valueint; 

				buildRICSubscriptionDeleteRequest(SendProcedure, value_RICRequestorID, value_RICInstanceID, value_RANFunctionID);
			}

			cJSON_Delete(root);
		}

		//-----------------------------------------------------------------build message & sctp send------------------------------------------------------------------//

		uint8_t buffermain[MAX_BUFFER + 1];
		size_t buffermain_size = MAX_BUFFER;
		asn_enc_rval_t Bytestring_main = asn_encode_to_buffer(NULL, ATS_ALIGNED_BASIC_PER,&asn_DEF_E2AP_PDU, SendProcedure, buffermain, buffermain_size);

		size_t Bytestring_main_size = Bytestring_main.encoded;
		printf("sizetest: %ld \n", Bytestring_main_size);

		if (Bytestring_main.encoded == -1) {
			printf("encoding of %s failed %s\n" , "test", strerror(errno));
			exit(-1);
		} else if (Bytestring_main.encoded > (ssize_t) buffermain_size) {
			printf("Buffer of size %ld is to small for %s\n" , buffermain_size , "test");
			exit(-1);
		} else{
			memcpy(buffersend_main, buffermain, Bytestring_main_size);
		}

		//send sctp message
		ret_main = sctp_sendmsg (value_SocketFD, (void *)buffersend_main, (size_t) Bytestring_main_size,
		NULL, 0, 0, 0, 0, 0, 0);
		if(ret_main == -1 )
		{
			printf("Error in sctp_sendmsg\n");
			perror("sctp_sendmsg()");
		}
		else
		{
			printf("Successfully sent %d bytes data to E2 Agent\n", ret_main);
		}

		//free
        rmr_free_msg(msg2);
        free(SendProcedure);
	}
}