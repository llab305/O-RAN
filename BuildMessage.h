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

#include "E2/E2SM-gNB-NRT-RANfunction-Definition.h"
#include "E2/RIC-InsertStyle-List.h"
#include "E2/RANparameterDef-Item.h"
#include "E2/GlobalE2node-en-gNB-ID.h"
#include "E2/RICsubsequentAction.h"
#include "E2/E2AP-PDU.h"
#include "E2/InitiatingMessage.h"
#include "E2/SuccessfulOutcome.h"
#include "E2/UnsuccessfulOutcome.h"
#include "E2/ProtocolIE-Field.h"
#include "E2/ENB-ID.h"
#include "E2/GlobalENB-ID.h"
#include "E2/GlobalE2node-gNB-ID.h"
#include "E2/constr_TYPE.h"
#include "E2/asn_constant.h"
#include "E2/RIC-EventTriggerStyle-List.h"


static void checkAndPrint(asn_TYPE_descriptor_t *typeDescriptor, void *data, char *dataType, const char *function) {
    char errbuf[128]; /* Buffer for error message */
    size_t errlen = sizeof(errbuf); /* Size of the buffer */
    if (asn_check_constraints(typeDescriptor, data, errbuf, &errlen) != 0) {
        fprintf(stderr, "%s Constraint validation failed: %s", dataType, errbuf);
    }
    fprintf(stdout, "%s successes function %s \n", dataType, function);
}

void createPLMN_IDByMCCandMNC(PLMN_Identity_t *plmnId, int mcc, int mnc) {

    //printf("PLMN_Identity_t %s \n", __func__);

    ASN_STRUCT_RESET(asn_DEF_PLMN_Identity, plmnId);
    plmnId->size = 3;
    plmnId->buf = (uint8_t *) calloc(1, 3);
    volatile int mcc1 = (unsigned int) (mcc / 100);
    volatile int mcc2 = (unsigned int) (mcc / 10 % 10);
    volatile int mcc3 = (unsigned int) (mcc % 10);
    plmnId->buf[0] = mcc2 << 4 | mcc1;

    volatile int mnc1 = (unsigned int)0;
    volatile int mnc2 = (unsigned int)0;
    volatile int mnc3 = (unsigned int)0;

    if (mnc >= 100) {
        mnc1 = (unsigned int) (mnc / 100);
        mnc2 = (unsigned int) (mnc / 10 % 10);
        mnc3 = (unsigned int) (mnc % 10);
    } else {
        mnc1 = (unsigned int) (mnc / 10);
        mnc2 = (unsigned int) (mnc % 10);
        mnc3 = 15;
    }
    plmnId->buf[1] = mcc3 << 4 | mnc3 ;
    plmnId->buf[2] = mnc2 << 4 | mnc1 ;

    //checkAndPrint(&asn_DEF_PLMN_Identity, plmnId, (char *) "PLMN_Identity_t", __func__);
}

void buildSetupSuccsessfulResponse(E2AP_PDU_t *pdu, int mcc, int mnc, uint8_t *data, int ranfunid) {
    ASN_STRUCT_RESET(asn_DEF_E2AP_PDU, pdu);

    //message type
    pdu->choice.successfulOutcome = (SuccessfulOutcome_t *)calloc(1, sizeof(SuccessfulOutcome_t));
    SuccessfulOutcome_t *successfulOutcome = pdu->choice.successfulOutcome;
    ASN_STRUCT_RESET(asn_DEF_E2setupResponse, &successfulOutcome->value.choice.E2setupResponse);
    successfulOutcome->procedureCode = ProcedureCode_id_E2setup;
    successfulOutcome->criticality = Criticality_reject;
    successfulOutcome->value.present = SuccessfulOutcome__value_PR_E2setupResponse;

    //ric-id
    E2setupResponseIEs_t *globalRicidIE = (E2setupResponseIEs_t *)calloc(1, sizeof(E2setupResponseIEs_t));
    ASN_STRUCT_RESET(asn_DEF_E2setupResponseIEs, globalRicidIE);

    globalRicidIE->criticality = Criticality_reject;
    globalRicidIE->id = ProtocolIE_ID_id_GlobalRIC_ID;
    globalRicidIE->value.present = E2setupResponseIEs__value_PR_GlobalRIC_ID;
    createPLMN_IDByMCCandMNC(&globalRicidIE->value.choice.GlobalRIC_ID.pLMN_Identity, mcc, mnc);

    globalRicidIE->value.choice.GlobalRIC_ID.ric_ID.size = 3;
    globalRicidIE->value.choice.GlobalRIC_ID.ric_ID.bits_unused = 4;
    globalRicidIE->value.choice.GlobalRIC_ID.ric_ID.buf = (uint8_t *)calloc(1, globalRicidIE->value.choice.GlobalRIC_ID.ric_ID.size);
    memcpy(globalRicidIE->value.choice.GlobalRIC_ID.ric_ID.buf, data, globalRicidIE->value.choice.GlobalRIC_ID.ric_ID.size);
    globalRicidIE->value.choice.GlobalRIC_ID.ric_ID.buf[2] &= (unsigned)0xF0;

    ASN_SEQUENCE_ADD(&successfulOutcome->value.choice.E2setupResponse.protocolIEs.list, globalRicidIE);

    //RAN function accepted
    E2setupResponseIEs_t *ranFunctionAdd = (E2setupResponseIEs_t *)calloc(1, sizeof(E2setupResponseIEs_t));
    ASN_STRUCT_RESET(asn_DEF_E2setupResponseIEs, ranFunctionAdd);
    ranFunctionAdd->criticality = Criticality_reject;
    ranFunctionAdd->id = ProtocolIE_ID_id_RANfunctionsAccepted;
    ranFunctionAdd->value.present = E2setupResponseIEs__value_PR_RANfunctionsID_List;

    RANfunctionID_ItemIEs_t *ranFuncIdItemIEs = (RANfunctionID_ItemIEs_t *)calloc(1, sizeof(RANfunctionID_ItemIEs_t));

    ranFuncIdItemIEs->criticality = Criticality_ignore;
    ranFuncIdItemIEs->id = ProtocolIE_ID_id_RANfunctionID_Item;
    ranFuncIdItemIEs->value.present = RANfunctionID_ItemIEs__value_PR_RANfunctionID_Item;
    ranFuncIdItemIEs->value.choice.RANfunctionID_Item.ranFunctionID = ranfunid;
    ranFuncIdItemIEs->value.choice.RANfunctionID_Item.ranFunctionRevision = 1;

    ASN_SEQUENCE_ADD(&ranFunctionAdd->value.choice.RANfunctionsID_List.list, ranFuncIdItemIEs);
    ASN_SEQUENCE_ADD(&successfulOutcome->value.choice.E2setupResponse.protocolIEs.list, ranFunctionAdd);

    pdu->present = E2AP_PDU_PR_successfulOutcome;
}

void buildRICSubscriptionRequest(E2AP_PDU_t *pdu, int requestorid, int instanceid,int ranfuctionid, int actionid, int actiontype) {
    ASN_STRUCT_RESET(asn_DEF_E2AP_PDU, pdu);

    pdu->choice.initiatingMessage = (InitiatingMessage_t *)calloc(1, sizeof(InitiatingMessage_t));
    InitiatingMessage_t *initiatingMessage = pdu->choice.initiatingMessage;
    ASN_STRUCT_RESET(asn_DEF_InitiatingMessage, initiatingMessage);
    initiatingMessage->procedureCode = ProcedureCode_id_RICsubscription;
    initiatingMessage->criticality = Criticality_reject;
    initiatingMessage->value.present = InitiatingMessage__value_PR_RICsubscriptionRequest;

    ASN_STRUCT_RESET(asn_DEF_RICsubscriptionRequest, &initiatingMessage->value.choice.RICsubscriptionRequest);

    {
        RICsubscriptionRequest_IEs_t  *rICsubscriptionRequest_IEs = (RICsubscriptionRequest_IEs_t *)calloc(1, sizeof(RICsubscriptionRequest_IEs_t));
        ASN_STRUCT_RESET(asn_DEF_RICsubscriptionRequest_IEs, rICsubscriptionRequest_IEs);

        rICsubscriptionRequest_IEs->criticality = Criticality_reject;
        rICsubscriptionRequest_IEs->id = ProtocolIE_ID_id_RICrequestID;
        rICsubscriptionRequest_IEs->value.present = RICsubscriptionRequest_IEs__value_PR_RICrequestID;
        rICsubscriptionRequest_IEs->value.choice.RICrequestID.ricRequestorID = requestorid;
        rICsubscriptionRequest_IEs->value.choice.RICrequestID.ricInstanceID = instanceid;

        ASN_SEQUENCE_ADD(&initiatingMessage->value.choice.RICsubscriptionRequest.protocolIEs.list, rICsubscriptionRequest_IEs);
    }
    
    {
        RICsubscriptionRequest_IEs_t  *rICsubscriptionRequest_IEs = (RICsubscriptionRequest_IEs_t *)calloc(1, sizeof(RICsubscriptionRequest_IEs_t));
        ASN_STRUCT_RESET(asn_DEF_RICsubscriptionRequest_IEs, rICsubscriptionRequest_IEs);

        rICsubscriptionRequest_IEs->criticality = Criticality_reject;
        rICsubscriptionRequest_IEs->id = ProtocolIE_ID_id_RANfunctionID;
        rICsubscriptionRequest_IEs->value.present = RICsubscriptionRequest_IEs__value_PR_RANfunctionID;
        rICsubscriptionRequest_IEs->value.choice.RANfunctionID = ranfuctionid;

        ASN_SEQUENCE_ADD(&initiatingMessage->value.choice.RICsubscriptionRequest.protocolIEs.list, rICsubscriptionRequest_IEs);
    }

    {
        RICsubscriptionRequest_IEs_t  *rICsubscriptionRequest_IEs = (RICsubscriptionRequest_IEs_t *)calloc(1, sizeof(RICsubscriptionRequest_IEs_t));
        ASN_STRUCT_RESET(asn_DEF_RICsubscriptionRequest_IEs, rICsubscriptionRequest_IEs);

        rICsubscriptionRequest_IEs->criticality = Criticality_reject;
        rICsubscriptionRequest_IEs->id = ProtocolIE_ID_id_RICsubscriptionDetails;
        rICsubscriptionRequest_IEs->value.present = RICsubscriptionRequest_IEs__value_PR_RICsubscriptionDetails;
        {
            RICaction_ToBeSetup_ItemIEs_t *ricactionToBeSetupItemIEs = (RICaction_ToBeSetup_ItemIEs_t *)calloc(1, sizeof(RICaction_ToBeSetup_ItemIEs_t));
            ricactionToBeSetupItemIEs->criticality = Criticality_reject;
            ricactionToBeSetupItemIEs->id = ProtocolIE_ID_id_RICaction_ToBeSetup_Item;
            ricactionToBeSetupItemIEs->value.present = RICaction_ToBeSetup_ItemIEs__value_PR_RICaction_ToBeSetup_Item;
            ricactionToBeSetupItemIEs->value.choice.RICaction_ToBeSetup_Item.ricActionID = actionid;
            ricactionToBeSetupItemIEs->value.choice.RICaction_ToBeSetup_Item.ricActionType = actiontype;

            RICactionDefinition_t *ad = (RICactionDefinition_t *)calloc(1, sizeof(RICactionDefinition_t));
            ASN_STRUCT_RESET(asn_DEF_RICactionDefinition, ad);
            uint8_t RICActionDef[] = "RICActionDef1" ;
            ad->size = strlen((char *)RICActionDef);
            ad->buf = (uint8_t *)calloc(1, ad->size);
            memcpy(ad->buf, RICActionDef, ad->size);
            ricactionToBeSetupItemIEs->value.choice.RICaction_ToBeSetup_Item.ricActionDefinition = ad;
            
            RICsubsequentAction_t *sa = (RICsubsequentAction_t *) calloc(1, sizeof(RICsubsequentAction_t));
            ASN_STRUCT_RESET(asn_DEF_RICsubsequentAction, sa);

            sa->ricTimeToWait = RICtimeToWait_w500ms;
            sa->ricSubsequentActionType = RICsubsequentActionType_continue;
            ricactionToBeSetupItemIEs->value.choice.RICaction_ToBeSetup_Item.ricSubsequentAction = sa;

            ASN_SEQUENCE_ADD(&rICsubscriptionRequest_IEs->value.choice.RICsubscriptionDetails.ricAction_ToBeSetup_List.list, ricactionToBeSetupItemIEs);
        }

        {   
            uint8_t eventTrigger[] = "EventTrigger1";
            rICsubscriptionRequest_IEs->value.choice.RICsubscriptionDetails.ricEventTriggerDefinition.buf = (uint8_t *)calloc(1, strlen((char *)eventTrigger));
            rICsubscriptionRequest_IEs->value.choice.RICsubscriptionDetails.ricEventTriggerDefinition.size = strlen((char *)eventTrigger);
            memcpy(rICsubscriptionRequest_IEs->value.choice.RICsubscriptionDetails.ricEventTriggerDefinition.buf, eventTrigger, 
                rICsubscriptionRequest_IEs->value.choice.RICsubscriptionDetails.ricEventTriggerDefinition.size);

        }

        ASN_SEQUENCE_ADD(&initiatingMessage->value.choice.RICsubscriptionRequest.protocolIEs.list, rICsubscriptionRequest_IEs);
    }

    pdu->present = E2AP_PDU_PR_initiatingMessage;
}

void buildRICSubscriptionDeleteRequest(E2AP_PDU_t *pdu, int requestorid, int instanceid,int ranfuctionid) {
    ASN_STRUCT_RESET(asn_DEF_E2AP_PDU, pdu);

    pdu->choice.initiatingMessage = (InitiatingMessage_t *)calloc(1, sizeof(InitiatingMessage_t));
    InitiatingMessage_t *initiatingMessage = pdu->choice.initiatingMessage;
    ASN_STRUCT_RESET(asn_DEF_InitiatingMessage, initiatingMessage);
    initiatingMessage->procedureCode = ProcedureCode_id_RICsubscriptionDelete;
    initiatingMessage->criticality = Criticality_reject;
    initiatingMessage->value.present = InitiatingMessage__value_PR_RICsubscriptionDeleteRequest;
    ASN_STRUCT_RESET(asn_DEF_RICsubscriptionDeleteRequest, &initiatingMessage->value.choice.RICsubscriptionDeleteRequest);

    {
        RICsubscriptionDeleteRequest_IEs_t  *ricsubscriptionDeleteRequest_IEs = (RICsubscriptionDeleteRequest_IEs_t *)calloc(1, sizeof(RICsubscriptionDeleteRequest_IEs_t));
        ASN_STRUCT_RESET(asn_DEF_RICsubscriptionDeleteRequest_IEs, ricsubscriptionDeleteRequest_IEs);

        ricsubscriptionDeleteRequest_IEs->criticality = Criticality_reject;
        ricsubscriptionDeleteRequest_IEs->id = ProtocolIE_ID_id_RICrequestID;
        ricsubscriptionDeleteRequest_IEs->value.present = RICsubscriptionDeleteRequest_IEs__value_PR_RICrequestID;
        ricsubscriptionDeleteRequest_IEs->value.choice.RICrequestID.ricRequestorID = requestorid;
        ricsubscriptionDeleteRequest_IEs->value.choice.RICrequestID.ricInstanceID = instanceid;

        ASN_SEQUENCE_ADD(&initiatingMessage->value.choice.RICsubscriptionDeleteRequest.protocolIEs.list, ricsubscriptionDeleteRequest_IEs);
    }

    {
        RICsubscriptionDeleteRequest_IEs_t  *ricsubscriptionDeleteRequest_IEs = (RICsubscriptionDeleteRequest_IEs_t *)calloc(1, sizeof(RICsubscriptionDeleteRequest_IEs_t));
        ASN_STRUCT_RESET(asn_DEF_RICsubscriptionDeleteRequest_IEs, ricsubscriptionDeleteRequest_IEs);

        ricsubscriptionDeleteRequest_IEs->criticality = Criticality_reject;
        ricsubscriptionDeleteRequest_IEs->id = ProtocolIE_ID_id_RANfunctionID;
        ricsubscriptionDeleteRequest_IEs->value.present = RICsubscriptionDeleteRequest_IEs__value_PR_RANfunctionID;
        ricsubscriptionDeleteRequest_IEs->value.choice.RANfunctionID = 33;
        
        ASN_SEQUENCE_ADD(&initiatingMessage->value.choice.RICsubscriptionDeleteRequest.protocolIEs.list, ricsubscriptionDeleteRequest_IEs);
    }

    pdu->present = E2AP_PDU_PR_initiatingMessage;
}
//char inmessage[100]
void buildRICControlRequest(E2AP_PDU_t *pdu, int requestorid, int instanceid,int ranfuctionid, int mcs1 , int mcs2, int mcs3, int mcs4 , int mcs5, int harq1, int harq2, int harq3, int harq4, int harq5, double repetition1, double repetition2, double repetition3, double repetition4, double repetition5) {
    ASN_STRUCT_RESET(asn_DEF_E2AP_PDU, pdu);

    pdu->choice.initiatingMessage = (InitiatingMessage_t *)calloc(1, sizeof(InitiatingMessage_t));
    InitiatingMessage_t *initiatingMessage = pdu->choice.initiatingMessage;
    initiatingMessage = pdu->choice.initiatingMessage;
    ASN_STRUCT_RESET(asn_DEF_InitiatingMessage, initiatingMessage);
    initiatingMessage->procedureCode = ProcedureCode_id_RICcontrol;
    initiatingMessage->criticality = Criticality_reject;
    initiatingMessage->value.present = InitiatingMessage__value_PR_RICcontrolRequest;
    ASN_STRUCT_RESET(asn_DEF_RICcontrolRequest, &initiatingMessage->value.choice.RICcontrolRequest);

    {
        RICcontrolRequest_IEs_t  *riccontrolRequest_IEs = (RICcontrolRequest_IEs_t *)calloc(1, sizeof(RICcontrolRequest_IEs_t));
        ASN_STRUCT_RESET(asn_DEF_RICcontrolRequest_IEs, riccontrolRequest_IEs);

        riccontrolRequest_IEs->criticality = Criticality_reject;
        riccontrolRequest_IEs->id = ProtocolIE_ID_id_RICrequestID;
        riccontrolRequest_IEs->value.present = RICcontrolRequest_IEs__value_PR_RICrequestID;
        riccontrolRequest_IEs->value.choice.RICrequestID.ricRequestorID = requestorid;
        riccontrolRequest_IEs->value.choice.RICrequestID.ricInstanceID = instanceid;

        ASN_SEQUENCE_ADD(&initiatingMessage->value.choice.RICcontrolRequest.protocolIEs.list, riccontrolRequest_IEs);
    }

    {
        RICcontrolRequest_IEs_t  *riccontrolRequest_IEs = (RICcontrolRequest_IEs_t *)calloc(1, sizeof(RICcontrolRequest_IEs_t));
        ASN_STRUCT_RESET(asn_DEF_RICcontrolRequest_IEs, riccontrolRequest_IEs);

        riccontrolRequest_IEs->criticality = Criticality_reject;
        riccontrolRequest_IEs->id = ProtocolIE_ID_id_RANfunctionID;
        riccontrolRequest_IEs->value.present = RICcontrolRequest_IEs__value_PR_RANfunctionID;
        riccontrolRequest_IEs->value.choice.RANfunctionID = ranfuctionid;
        
        ASN_SEQUENCE_ADD(&initiatingMessage->value.choice.RICcontrolRequest.protocolIEs.list, riccontrolRequest_IEs);
    }

    {   
        RICcontrolRequest_IEs_t  *riccontrolRequest_IEs = (RICcontrolRequest_IEs_t *)calloc(1, sizeof(RICcontrolRequest_IEs_t));
        ASN_STRUCT_RESET(asn_DEF_RICcontrolRequest_IEs, riccontrolRequest_IEs);

        riccontrolRequest_IEs->criticality = Criticality_reject;
        riccontrolRequest_IEs->id = ProtocolIE_ID_id_RICcontrolHeader;
        riccontrolRequest_IEs->value.present = RICcontrolRequest_IEs__value_PR_RICcontrolHeader;

        uint8_t controlheader[] = "ControlHeader";
        riccontrolRequest_IEs->value.choice.RICcontrolHeader.buf = (uint8_t *)calloc(1, strlen((char *)controlheader));
        riccontrolRequest_IEs->value.choice.RICcontrolHeader.size = strlen((char *)controlheader);
        memcpy(riccontrolRequest_IEs->value.choice.RICcontrolHeader.buf, controlheader, riccontrolRequest_IEs->value.choice.RICcontrolHeader.size);

        ASN_SEQUENCE_ADD(&initiatingMessage->value.choice.RICcontrolRequest.protocolIEs.list, riccontrolRequest_IEs);
    }

    {   
        RICcontrolRequest_IEs_t  *riccontrolRequest_IEs = (RICcontrolRequest_IEs_t *)calloc(1, sizeof(RICcontrolRequest_IEs_t));
        ASN_STRUCT_RESET(asn_DEF_RICcontrolRequest_IEs, riccontrolRequest_IEs);

        riccontrolRequest_IEs->criticality = Criticality_reject;
        riccontrolRequest_IEs->id = ProtocolIE_ID_id_RICcontrolMessage;
        riccontrolRequest_IEs->value.present = RICcontrolRequest_IEs__value_PR_RICcontrolMessage;


        uint8_t controlmessage[500];
        snprintf( controlmessage, 500,
								"{\'Repetition1\': %f,\'Repetition2\': %f,\'Repetition3\':%f,\'Repetition4\': %f,\'Repetition5\': %f,\'MCS1\': %d,\'MCS2\': %d,\'MCS3\': %d,\'MCS4\': %d,\'MCS5\': %d,\'HARQ1\': %d,\'HARQ2\': %d,\'HARQ3\': %d,\'HARQ4\': %d,\'HARQ5\': %d} ",	// create the payload
								repetition1, repetition2, repetition3, repetition4, repetition5, mcs1 ,mcs2 , mcs3, mcs4, mcs5, harq1, harq2, harq3, harq4, harq5);
        //uint8_t controlmessage[100];
        //{'MCS_DL':19, 'MCS_UL':19, 'max_DL_HARQ':2, 'max_UL_HARQ':2, 'Repetition':2.0, 'Grantfree_Ind':0}
        //int ulmcs , int dlharq, int ulharq, int repetition, int maxnumue, int grantfreeInd

        printf("ControlMessage:%s\n",controlmessage);
        //strcpy(controlmessage,inmessage);
        riccontrolRequest_IEs->value.choice.RICcontrolMessage.buf = (uint8_t *)calloc(1, strlen((char *)controlmessage));
        riccontrolRequest_IEs->value.choice.RICcontrolMessage.size = strlen((char *)controlmessage);
        memcpy(riccontrolRequest_IEs->value.choice.RICcontrolMessage.buf, controlmessage, riccontrolRequest_IEs->value.choice.RICcontrolMessage.size);


        ASN_SEQUENCE_ADD(&initiatingMessage->value.choice.RICcontrolRequest.protocolIEs.list, riccontrolRequest_IEs);
    }

    {
        RICcontrolRequest_IEs_t  *riccontrolRequest_IEs = (RICcontrolRequest_IEs_t *)calloc(1, sizeof(RICcontrolRequest_IEs_t));
        ASN_STRUCT_RESET(asn_DEF_RICcontrolRequest_IEs, riccontrolRequest_IEs);

        riccontrolRequest_IEs->criticality = Criticality_notify;
        riccontrolRequest_IEs->id = ProtocolIE_ID_id_RICcontrolAckRequest;
        riccontrolRequest_IEs->value.present = RICcontrolRequest_IEs__value_PR_RICcontrolAckRequest;
        riccontrolRequest_IEs->value.choice.RICcontrolAckRequest = RICcontrolAckRequest_ack;
        
        ASN_SEQUENCE_ADD(&initiatingMessage->value.choice.RICcontrolRequest.protocolIEs.list, riccontrolRequest_IEs);
    }

    pdu->present = E2AP_PDU_PR_initiatingMessage;
}

void buildErrorIndication(E2AP_PDU_t *pdu) {
    ASN_STRUCT_RESET(asn_DEF_E2AP_PDU, pdu);

    pdu->choice.initiatingMessage = (InitiatingMessage_t *)calloc(1, sizeof(InitiatingMessage_t));
    InitiatingMessage_t *initiatingMessage = pdu->choice.initiatingMessage;
    ASN_STRUCT_RESET(asn_DEF_InitiatingMessage, initiatingMessage);
    initiatingMessage->procedureCode = ProcedureCode_id_ErrorIndication;
    initiatingMessage->criticality = Criticality_reject;
    initiatingMessage->value.present = InitiatingMessage__value_PR_ErrorIndication;
    ASN_STRUCT_RESET(asn_DEF_ErrorIndication, &initiatingMessage->value.choice.ErrorIndication);

    {
        ErrorIndication_IEs_t  *errorIndication_IEs = (ErrorIndication_IEs_t *)calloc(1, sizeof(ErrorIndication_IEs_t));
        ASN_STRUCT_RESET(asn_DEF_ErrorIndication_IEs, errorIndication_IEs);

        errorIndication_IEs->criticality = Criticality_reject;
        errorIndication_IEs->id = ProtocolIE_ID_id_RANfunctionID;
        errorIndication_IEs->value.present = ErrorIndication_IEs__value_PR_RANfunctionID;
        errorIndication_IEs->value.choice.RANfunctionID = 52;
        
        ASN_SEQUENCE_ADD(&initiatingMessage->value.choice.ErrorIndication.protocolIEs.list, errorIndication_IEs);
    }

    {
        ErrorIndication_IEs_t  *errorIndication_IEs = (ErrorIndication_IEs_t *)calloc(1, sizeof(ErrorIndication_IEs_t));
        ASN_STRUCT_RESET(asn_DEF_ErrorIndication_IEs, errorIndication_IEs);

        errorIndication_IEs->criticality = Criticality_reject;
        errorIndication_IEs->id = ProtocolIE_ID_id_RICrequestID;
        errorIndication_IEs->value.present = ErrorIndication_IEs__value_PR_RICrequestID;
        errorIndication_IEs->value.choice.RICrequestID.ricRequestorID = 61;
        errorIndication_IEs->value.choice.RICrequestID.ricInstanceID = 31;

        ASN_SEQUENCE_ADD(&initiatingMessage->value.choice.ErrorIndication.protocolIEs.list, errorIndication_IEs);
    }

    {
        ErrorIndication_IEs_t  *errorIndication_IEs = (ErrorIndication_IEs_t *)calloc(1, sizeof(ErrorIndication_IEs_t));
        ASN_STRUCT_RESET(asn_DEF_ErrorIndication_IEs, errorIndication_IEs);

        errorIndication_IEs->criticality = Criticality_ignore;
        errorIndication_IEs->id = ProtocolIE_ID_id_Cause;
        errorIndication_IEs->value.present = ErrorIndication_IEs__value_PR_Cause;
        errorIndication_IEs->value.choice.Cause.present = Cause_PR_transport;
        errorIndication_IEs->value.choice.Cause.choice.transport = CauseTransport_transport_resource_unavailable;

        ASN_SEQUENCE_ADD(&initiatingMessage->value.choice.ErrorIndication.protocolIEs.list, errorIndication_IEs);
    }

    pdu->present = E2AP_PDU_PR_initiatingMessage;
}

//-----------------------------------------------------------------------------------------Failure---------------------------------------------------------------------------------------------------//

void buildSetupUnSuccsessfulResponse(E2AP_PDU_t *pdu) {
    ASN_STRUCT_RESET(asn_DEF_E2AP_PDU, pdu);

    pdu->choice.unsuccessfulOutcome = (UnsuccessfulOutcome_t *)calloc(1, sizeof(UnsuccessfulOutcome_t));
    UnsuccessfulOutcome_t *uns = pdu->choice.unsuccessfulOutcome;
    uns->procedureCode = ProcedureCode_id_E2setup;
    uns->criticality = Criticality_reject;
    uns->value.present = UnsuccessfulOutcome__value_PR_E2setupFailure;

    ASN_STRUCT_RESET(asn_DEF_E2setupFailure, &uns->value.choice.E2setupFailure);


    {
        E2setupFailureIEs_t *e2SetupFIE = (E2setupFailureIEs_t *) calloc(1, sizeof(E2setupFailureIEs_t));
        ASN_STRUCT_RESET(asn_DEF_E2setupFailureIEs, e2SetupFIE);

        e2SetupFIE->criticality = Criticality_ignore;
        e2SetupFIE->id = ProtocolIE_ID_id_Cause;
        e2SetupFIE->value.present = E2setupFailureIEs__value_PR_Cause;
        e2SetupFIE->value.choice.Cause.present = Cause_PR_transport;
        e2SetupFIE->value.choice.Cause.choice.transport = CauseTransport_transport_resource_unavailable;

        ASN_SEQUENCE_ADD(&uns->value.choice.E2setupFailure.protocolIEs.list, e2SetupFIE);
    }

    {
        E2setupFailureIEs_t *e2SetupFIE = (E2setupFailureIEs_t *) calloc(1, sizeof(E2setupFailureIEs_t));
        ASN_STRUCT_RESET(asn_DEF_E2setupFailureIEs, e2SetupFIE);

        e2SetupFIE->criticality = Criticality_ignore;
        e2SetupFIE->id = ProtocolIE_ID_id_TimeToWait;
        e2SetupFIE->value.present = E2setupFailureIEs__value_PR_TimeToWait;
        e2SetupFIE->value.choice.TimeToWait = TimeToWait_v60s;

        ASN_SEQUENCE_ADD(&uns->value.choice.E2setupFailure.protocolIEs.list, e2SetupFIE);
    }

    pdu->present = E2AP_PDU_PR_unsuccessfulOutcome;
}