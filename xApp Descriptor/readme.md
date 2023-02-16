# 1. xApp Descriptor
The xApp Descriptor is a description file that must be submitted for the deployment of an xApp. It should contain basic information about the configuration of the xApp, and use JSON as the format for data exchange.

The fields that need to be filled in the xApp Descriptor will be divided into two parts:
1.	Main fields : xapp_name (Mandatory) field, version (Mandatory) field, containers (Mandatory) field, messaging (Optional) field.
2.	Secondary fields : all other fields except for the main fields are considered secondary fields.


# 2. xApp Descriptor Field
***
### 2.1 xapp_name (Mandatory)
  *	Writer : xApp designer
  *	Data type : string
  *	Description : The xApp name will serve as the unique identifier for the xApp. It should be named according to the <b>"xapp-\<type\>-\<identifier\>"</b> rule and the total length should be less than or equal to 63 characters due to the fact that this field's name is the same as the "name (container name)" field in section 2.3.1.
    The naming convention is defined as follows:</br>
    * "\<type\>" indicates the purpose of the xApp. When filling in this field, only lowercase letters and numbers should be used, and it should be named according to the suggested rules in Table 2.1-1.</br>
    
    <p align="center"> Table 2.1-1 "< type >" suggested rules to fill in the corresponding table </p>
    
    |"\<type\>"|Description|
    |---|---|
    |"5qi-\<5QI value\>"|Represents appropriate 5G QoS for traffic allocation, where "<5QI value>" corresponds to the QoS requirements of a specific vertical application, including Packet Error Rate (PER), Packet Delay Budget (PDB), Throughput, and other requirements.|  
    |"handover"|Refers to the scenario where a UE (User Equipment) performs a handover from one serving cell to another serving cell of the same base station or to a serving cell of a different base station.|
    
    * "\<identifier\>" is used to distinguish xApps of the same purpose but with different algorithm designs or controlling different UEs.  When filling in this field, only lowercase letters and numbers should be used.
  
  *	Example: if "\<type\>" is equal to "5qi-\<5QI value\>", as shown in Figure 2.1-1; if "\<type\>" is equal to "handover", as shown in Figure 2.1-2.
  
    <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.1-1.png"> </image></p>
    <p align="center"> Figure 2.1-1 xapp_name - if "< type >" equals "5QI-<5QI value>" - example </p>
    
    <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.1-2.png"></image></p>
    <p align="center"> Figure 2.1-2 xapp_name - if "< type >" equals "handover" - example </p>
    
***
### 2.2 version (Mandatory)    
 *	Writer : xApp designer
 *	Data type : string
 *	Description : The version number of xApp descriptor. </br> The naming will be filled in according to the rules of "\<major version\>.\<minor version\>.\<patch version\>", where "\<major version\>" increases the version number sequentially from 1, "\<minor version\>" and "\<patch version\>" increase the version number sequentially starting from 0, that is to say, all version number updates after this version number are based on the modified content of this xApp descriptor. </br></br> The rules for incrementing the version number are defined as follows: </br> 
     1. If the xApp descriptor changes the main field, it will increment "\<major version\>", at which time "\<minor version\>" and "\<patch version\>" must be reset to zero. Main fields e.g., <b> xapp_name (Mandatory) field, version (Mandatory) field, containers (Mandatory) field, and messaging (Optional) field</b>.</br> 
     2. If xApp descriptor maintains a backward-compatible main field and adds a secondary field, "\<minor version\>" is incremented, and "\<patch version\>" must be reset to zero when incremented. The secondary fields are shown in Table 2.2-1.</br>
     
     <p align="center"> Table 2.2-1 Secondary field correspondence table under the main field </p>

    |secondary field\main field|containers (Mandatory)|messaging (Optional)|
    |---|---|---|
    |1|name (Mandatory) (container name)|ports (Mandatory)|  
    |2|image (Mandatory)|name (Mandatory) (port name)|
    |3|registry (Mandatory)|container (Mandatory)|  
    |4|handover|port (Mandatory)|
    |5|name 	(Mandatory) (image name)|txMessages (Mandatory)|  
    |6|handover|rxMessages (Mandatory)|
    |7|tag (Mandatory)|policies (Mandatory)|  
    |8||PolicyTypeId (Conditional)|
    |9||SupportXapp (Optional)|  
    |10||description (Optional)|  

    3. If the xApp descriptor maintains a backward-compatible main field and no secondary fields are added, but the content needs to be corrected, increment "\<patch version\>". 
     
     Naming rules have the following definitions:
     * "\<major version\>" is the major version number of the xApp descriptor. Only numbers can be used when filling in the content, and there is no need to fill in zeros before the numbers.
     * "\<minor version\>" is the minor version number of xApp descriptor. Only numbers can be used when filling in the content, and there is no need to fill in zeros before the numbers.
     * "\<patch version\>" is the patch number of the xApp descriptor. Only numbers can be used when filling in the content, and there is no need to fill in zeros before the numbers.
     
 *	Example : Figure 2.2-1 </br>
     
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.2-1.png"></image></p>
 <p align="center"> Figure 2.2-1 version - example </p>
  
***


### 2.3	containers (Mandatory)
It covers the container the xApp will run in. However, this version limits xApp to only be deployed as a Pod, a Pod can only run one container, and a container can only support one image, but an image can be referenced by multiple Pods. For containers, include the following fields: <b>name (Mandatory), image (Mandatory)</b>.

***
#### 2.3.1 name (Mandatory)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : In the name of the container, fill in the same fields as <b>xapp_name (Mandatory)</b>.</br>
 *	Example : Same as Figure 2.1-2, as shown in Figure 2.3-1.
 
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.3-1.png"></image></p>
 <p align="center"> Figure 2.3-1 containers.name - example </p>
  
***

#### 2.3.2 image (Mandatory) 
Contains the image in the container that will run this xApp. For image, include the following fields: <b>registry (Mandatory), name 	(Mandatory), and tag (Mandatory)</b>.

***
##### 2.3.2.1	registry (Mandatory)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : Since the xApp designer cannot obtain the IP and port number of the local private image repository, "NULL" must be filled in, and then handed over to the xApp Manager to fill in the content.</br>
 *	Example : Figure 2.3-2
 
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.3-2.png"></image></p>
 <p align="center"> Figure 2.3-2 containers.image.registry - example </p>
 
***

##### 2.3.2.2	name (Mandatory)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : The name of the image.</br> Since an image can be referenced by multiple Pods in this RIC system, the name of the image must be different from the 2.3.1 name (Mandatory) (container name) field. The naming will be filled in according to the rules of "xapp-\<type\>-\<identifier\>", and the total length is limited to less than or equal to 255 characters.
     Naming rules have the following definitions:
     * "\<type\>" is the same as the "\<type\>" name of the xapp_name (Mandatory) field.
     * "\<identifier\>" is used to distinguish the name of the image, and when filling in the content, only lowercase English letters or numbers can be used to fill in the content.
 *	Example : Different from Figure 2.3-1, as shown in Figure 2.3-3 .
     
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.3-3.png"></image></p>
 <p align="center"> Figure 2.3-3 containers.image.name - example </p>
     
***

##### 2.3.2.3	tag (Mandatory)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : The version number of the image. </br> The naming will be filled in according to the rules of "\<major version\>.\<minor version\>.\<patch version\>", where "\<major version\>" increases the version number sequentially from 1, "\<minor version\>" and "\<patch version\>" increase the version number sequentially starting from 0, that is to say, all version number updates after this version number are based on the modified content of this xApp. </br></br> The rules for incrementing the version number are defined as follows: </br>
     1. If the xApp function has changed and cannot be backward-compatible, it will increment "\<major version\>", at which time "\<minor version\>" and "\<patch version\>" must be reset to zero.
     2. If xApp maintains backward-compatible xApp function and adds new function, "\<minor version\>" is incremented, and "\<patch version\>" must be reset to zero when incremented. The secondary fields are shown in Table 2.2-1.</br>
     3. If the xApp maintains a backward-compatible xApp function and no new functions are added, but the content needs to be corrected, increment "\<patch version\>". 
     
     Naming rules have the following definitions:
     * "\<major version\>" is the major version number of the xApp. Only numbers can be used when filling in the content, and there is no need to fill in zeros before the numbers.
     * "\<minor version\>" is the minor version number of xApp. Only numbers can be used when filling in the content, and there is no need to fill in zeros before the numbers.
     * "\<patch version\>" is the patch number of the xApp. Only numbers can be used when filling in the content, and there is no need to fill in zeros before the numbers.
     
 *	Example : Figure 2.3-4
     
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.3-4.png"></image></p>
 <p align="center"> Figure 2.3-4 containers.image.tag - example </p>
     
***

### 2.4	 messaging (Optional)
Contains the container communication port on which this xApp will run. For messaging, the ports (Mandatory) field will be included.
***
 
##### 2.4.1	ports (Mandatory)
For RMR port, include the following fields: <b>name (Mandatory), container (Mandatory), port (Mandatory), txMessages (Mandatory), rxMessages (Mandatory), policies (Mandatory), and description (Optional)</b>.

If the data port of the RMR port is the same as the route port, only one port number can be used; otherwise, if the data port of the RMR port is different from the route port, there are two port numbers that can be used, and they correspond to different purposes. The solution can only provide support in the future if xApp has updated the RMR port usage distinction and the support of the corresponding xApp Manager. 
     
For non-RMR port, include the following fields: <b>name (Mandatory), container (Mandatory), port (Mandatory), and description (Optional)</b>.

***   

###### 2.4.1.1 name (Mandatory)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : The name of the port number.</br> 
     For the RMR port, if the data port of the RMR port is the same as the route port, "rmr-data-route" must be filled in; otherwise, if the data port of the RMR port is different from the route port, the RMR data port must be filled with "rmr-data", The RMR routing port must be filled with "rmr-route". <br>
     For non-RMR ports, fill in the content according to the communication purpose of the port number. When filling in the content, only uppercase and lowercase English letters or numbers can be used to fill in the content.
     
 *	Example : For RMR port, if the data port of the RMR port is the same as the route port, as shown in Figure 2.4-1; if the data port of the RMR port is different from the route port, as shown in Figure 2.4-2 and Figure 2.4-3. For non-RMR ports, as shown in Figure 2.4-4.
 
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.4-1.png"></image></p>
 <p align="center"> Figure 2.4-1 messaging.ports.name - example </p>

 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.4-2.png"></image></p>
 <p align="center"> Figure 2.4-2 messaging.ports.name - example </p>
 
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.4-3.png"></image></p>
 <p align="center"> Figure 2.4-3 messaging.ports.name - example </p>
     
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.4-4.png"></image></p>
 <p align="center"> Figure 2.4-4 messaging.ports.name - example </p>     
     
***
     
###### 2.4.1.2 container (Mandatory)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : The name of the container, according to 2.3.1 name (Mandatory) (container name) fill in the same fields.</br>
 *	Example : The same as Figure 2.3-1 , as shown in Figure 2.4-5.
 
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.4-5.png"></image></p>
 <p align="center"> Figure 2.4-5 messaging.ports.container - example </p>
     
***

###### 2.4.1.3	port (Mandatory)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : Since the xApp designer cannot obtain the port number, "NULL" must be filled in, and then handed over to the xApp Manager to fill in the content.</br>
 *	Example : Figure 2.4-6
      
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.4-6.png"></image></p>
 <p align="center"> Figure 2.4-6 messaging.ports.port - example </p>
     
***
     
###### 2.4.1.4	txMessages (Mandatory)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : Different processes and events in xApp correspond to different RMR message type names, which will contain all RMR message type names of xApp that can deliver the request, and then xApp Manager will update the data in the xApp Manager table in influxDB.</br>
 *	Example : Figure 2.4-7
     
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.4-7.png"></image></p>
 <p align="center"> Figure 2.4-7 messaging.ports.txMessages - example </p>
     
***     
     
###### 2.4.1.5	rxMessages (Mandatory)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : Different processes and events in xApp correspond to different RMR message type names, which will contain all RMR message type names of xApp that can receive the response, and then xApp Manager will update the data in the xApp Manager table in influxDB.</br>
 *	Example : Figure 2.4-8
      
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.4-8.png"></image></p>
 <p align="center"> Figure 2.4-8 messaging.ports.rxMessages - example </p>
     
***       
     
###### 2.4.1.6	policies (Mandatory)
Contains a list of policies that xApp can support. It is recommended to fill in SupportXapp (Optional) this field will help RIC Operator know how many xApps related to A1 policies need to be deployed in the future.

The policies field will be filled in three ways:
1.	Policies field is empty, indicating that the cap does not need to receive an A1 policy.  (Please refer to Examples C for filling examples)
2.	Policies field is not empty, fill in the PolicyTypeId (Conditional) field and the SupportXapp (Optional) field, indicating that the xApp must receive the A1 policy corresponding to the policy type. (Please refer to Examples A and Examples B for filling examples)
3.	Policies field is not empty, fill in the content defined in the PolicyTypeId (Conditional) field to indicate that xApp must receive the A1 policy corresponding to the policy type. (Please refer to Examples D for filling examples)

***    

###### 2.4.1.6.1	PolicyTypeId (Conditional)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : The name of the category of A1 policy.</br> If xApp needs to receive A1 policy, you must fill in the content defined in this field, and then xApp Manager will update the data in xApp Manager Table in influxDB. When filling in this content, the xApp designer must select a PolicyTypeId according to Table 2.4 1 to fill in content;  otherwise, if the xApp does not need to receive the A1 policy, the xApp designer does not need to fill in the content defined in this field.

     <p align="center"> Table 2.2-1 Secondary field correspondence table under the main field </p>

    |Policy type objectives|Description|PolicyTypeId|
    |---|---|---|
    |QoS Target|QoS's service measurement target|ORAN_QoSTarget_2.0.0|
    |QoE Target|QoE's service measurement target|ORAN_QoETarget_2.0.0|  
    |Traffic Steering Preferences|Balance the load of the cell|ORAN_TrafficSteeringPreference_2.0.0|
    |UE Level Target|UE performance target|ORAN_UELevelTarget_1.0.0|  
    |QoS optimization with resource directive|QoS's service measurement target and balance the load of the Cell|ORAN_QoSandTSP_2.0.0|
    |QoE optimization with resource directive|QoE's service measurement target and balance the load of the Cell|ORAN_QoEandTSP_2.0.0|  
    |Load Balancing|Balance the load between congested cells and non-congested candidate cells|ORAN_LoadBalancing_1.0.0|

*	Example : Figure 2.4-9

 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.4-9.png"></image></p>
 <p align="center"> Figure 2.4-9 messaging.ports.policies.PolicyTypeId - example </p> 
 
***

###### 2.4.1.6.2	SupportXapp (Optional)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : List of xApps that support implementing A1 policy.</br> If the xApp needs to receive the A1 policy, it can choose to fill in this field, and then the xApp Manager will update the data in the xApp Manager Table in influxDB; otherwise, if the xApp does not need to receive the A1 policy, it is not necessary to fill in this field. When filling in the content, fill in this field according to the xapp_name (Mandatory) in the xApp Descriptor that the xApp designer will deploy in the future to support the implementation of the A1 policy.
 *	Example : Figure 2.4-10
 
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.4-10.png"></image></p>
 <p align="center"> Figure 2.4-10 messaging.ports.policies.SupportXapp - example </p> 
 
***

###### 2.4.1.7	description (Optional)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : The description of the port number used. Fill in the content in detail according to the purpose of the port number. When filling in the content, only "space", uppercase or lowercase English letters, and numbers can be used, and the total length is limited to less than or equal to 30 characters.</br>
 *	Example : Figure 2.4-11 
 
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.4-11.png"></image></p>
 <p align="center"> Figure 2.4-11 messaging.ports.description - example </p>
     
***   
