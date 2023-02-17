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
 *	Description : The version number of the xApp descriptor. According to the naming convention, the version number of the xApp descriptor will be filled in based on the rule <b>"\<major version\>.\<minor version\>.\<patch version\>"</b>, where "\<major version\>" starts at 1 and increments sequentially with each new major release, and "\<minor version\>" and "\<patch version\>" both start at 0 and increment sequentially with each new minor release. The version number of all subsequent releases will be based on the previous xApp descriptor version and modified accordingly. The rules for incrementing the version number are defined as follows:

     1. If there is a major change in the required fields of the xApp descriptor, such as the xapp_name (Mandatory) field, the version (Mandatory) field, the containers (Mandatory) field, or the messaging (Optional) field, then the "<major version>" must be incremented, and both "<minor version>" and "<patch version>" must be reset to 0. 
     2. If the xApp descriptor maintains backward compatibility with the major fields and adds new minor fields, such as those shown in Table 2.2-1, then the "<minor version>" must be incremented, and the "<patch version>" must be reset to 0.
     
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

    3. If the xApp Descriptor keeps backward compatibility with main fields and no secondary fields are added, but there is a need to correct content issues, then the "\<patch version\>" is incremented. Examples of content issues that need to be fixed include the "tag" (Mandatory) field in the "image" (Mandatory) field of the "containers" (Mandatory) field. 
     
     The naming convention is defined as follows:
     * "\<major version\>" is the major version number of the xApp descriptor. When filling in the content, only numbers can be used and there is no need to pad the content with zeros in front of the numbers.
     * "\<minor version\>" is the minor version number of xApp descriptor. When filling in the content, only numbers can be used and there is no need to pad the content with zeros in front of the numbers.
     * "\<patch version\>" is the patch number of the xApp descriptor. When filling in the content, only numbers can be used and there is no need to pad the content with zeros in front of the numbers.
     
 *	Example : Figure 2.2-1 </br>
     
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.2-1.png"></image></p>
 <p align="center"> Figure 2.2-1 version - example </p>
  
***


### 2.3	containers (Mandatory)
This covers the container in which the xApp will run, but this version limits the xApp to be deployed as only one Pod, and each Pod can only support running one container, which can only correspond to one image. However, one image can be referenced by multiple Pods. For a container, it will include the following fields: name (Mandatory), image (Mandatory).

***
#### 2.3.1 name (Mandatory)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : The name of the container should be filled with the same content as the xapp_name (Mandatory) field.</br>
 *	Example : Same as Figure 2.1-2, as shown in Figure 2.3-1.
 
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.3-1.png"></image></p>
 <p align="center"> Figure 2.3-1 containers.name - example </p>
  
***

#### 2.3.2 image (Mandatory) 
This refers to the image of the container in which the xApp will run. The image will include the following fields: registry (Mandatory), name (Mandatory), and tag (Mandatory).

***
##### 2.3.2.1	registry (Mandatory)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : As xApp designers cannot access the IP and port number of local private image repositories, they must fill in "NULL" and leave it to the xApp Manager to fill in this information later.</br>
 *	Example : Figure 2.3-2
 
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.3-2.png"></image></p>
 <p align="center"> Figure 2.3-2 containers.image.registry - example </p>
 
***

##### 2.3.2.2	name (Mandatory)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : The name of the image, since an image can be referenced by multiple Pods in this RIC system, must be different from the "name (container name)" field in section 2.3.1.
The naming will follow the rule of "xapp-\<type\>-\<identifier\>" and the total length is limited to less than or equal to 255 characters.
     The naming rules are defined as follows:
     * "\<type\>" will be the same as the "<type>" named in the xapp_name (Mandatory) field.
     * "\<identifier\>" is used to differentiate the name of the image, and only lowercase letters or numbers can be used when filling in the content.
     
 *	Example : Different from Figure 2.3-1, as shown in Figure 2.3-3 .
     
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.3-3.png"></image></p>
 <p align="center"> Figure 2.3-3 containers.image.name - example </p>
     
***

##### 2.3.2.3	tag (Mandatory)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : The version number of the image. </br> Naming will be based on the rule of "\<major version\>.\<minor version\>.\<patch version\>", where "\<major version\>" starts from 1 and increases version number sequentially. Both "\<minor version\>" and "\<patch version\>" start from 0 and increase version number sequentially. It represents that all version updates after this version number are based on the xApp and modify its content. The rules for increasing the version number are defined as follows: </br>
     1. If the xApp function changes and is not backward compatible, increase the "\<major version\>", and when increasing, "\<minor version\>" and "\<patch version\>" must be set to 0. For example, changes in AI/ML algorithms implemented by xApp are considered as xApp function changes.
     2. If xApp function is added and remains backward compatible, increase the "\<minor version\>", and "\<patch version\>" must be set to 0 when increasing. The secondary fields are shown in Table 2.2-1.</br>
     3. If xApp does not add any new function and remains backward compatible but needs to fix functional issues, increase the "\<patch version\>". 
     
     The naming rules are defined as follows:
     * "\<major version\>" is the major version number of the xApp. When filling in the content, only numbers can be used and there is no need to pad the content with zeros in front of the numbers.
     * "\<minor version\>" is the minor version number of the xApp. When filling in the content, only numbers can be used and there is no need to pad the content with zeros in front of the numbers.
     * "\<patch version\>" is the patch number of the xApp. When filling in the content, only numbers can be used and there is no need to pad the content with zeros in front of the numbers.
     
 *	Example : Figure 2.3-4
     
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.3-4.png"></image></p>
 <p align="center"> Figure 2.3-4 containers.image.tag - example </p>
     
***

### 2.4	 messaging (Optional)
The communication ports of the container where the xApp will run. For messaging, the "ports" (Mandatory) field will be included.
***
 
##### 2.4.1	ports (Mandatory)
For RMR port, the following fields will be included: <b>name (Mandatory), container (Mandatory), port (Mandatory), txMessages (Mandatory), rxMessages (Mandatory), policies (Mandatory), and description (Optional)</b>.
If the data port and route port of the RMR port are the same, only one port number is available. Conversely, if the data port and route port of the RMR port are different, two port numbers are available, each corresponding to another purpose. This scheme is only supported if future updates to the xApp differentiate the usage of RMR ports and if the xApp Manager supports it.
     
For non-RMR port, the following fields will be included: <b>name (Mandatory), container (Mandatory), port (Mandatory), and description (Optional)</b>.

***   

###### 2.4.1.1 name (Mandatory)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : The name of the port number.</br> 
     For RMR port, if the data port and route port of the RMR port are the same, the name "rmr-data-route" must be used; otherwise, if the data port and route port of the RMR port are different, the RMR data port must be named "rmr-data", and the RMR route port must be named "rmr-route".<br>
     For non-RMR ports, the name should be based on the communication purpose of the port number, and only uppercase or lowercase letters and numbers can be used when naming the port.
     
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
 *	Description : The name of the container, which should be filled in with the same content as the "name" (container name) field in section 2.3.1.</br>
 *	Example : The same as Figure 2.3-1 , as shown in Figure 2.4-5.
 
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.4-5.png"></image></p>
 <p align="center"> Figure 2.4-5 messaging.ports.container - example </p>
     
***

###### 2.4.1.3	port (Mandatory)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : As xApp designers cannot access the port number, "NULL" must be filled in and the content will be filled in by the xApp Manager later.</br>
 *	Example : Figure 2.4-6
      
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.4-6.png"></image></p>
 <p align="center"> Figure 2.4-6 messaging.ports.port - example </p>
     
***
     
###### 2.4.1.4	txMessages (Mandatory)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : The different procedures and events in the xApp correspond to different RMR message type names, and the list of all the RMR message type names that the xApp can send will be included. The xApp Manager will then update this information in the xApp Manager Table in the InfluxDB.</br>
 *	Example : Figure 2.4-7
     
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.4-7.png"></image></p>
 <p align="center"> Figure 2.4-7 messaging.ports.txMessages - example </p>
     
***     
     
###### 2.4.1.5	rxMessages (Mandatory)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : The different procedures and events in the xApp correspond to different RMR message type names. The list of all RMR message type names that the xApp can receive responses for will be included, and the xApp Manager will update this information in the xApp Manager Table in influxDB./br>
 *	Example : Figure 2.4-8
      
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.4-8.png"></image></p>
 <p align="center"> Figure 2.4-8 messaging.ports.rxMessages - example </p>
     
***       
     
###### 2.4.1.6	policies (Mandatory)
The list of policy types is supported by the xApp, and it is recommended to fill in the "SupportXapp" (Optional) field to facilitate the RIC Operator to know how many xApps related to the A1 policy will need to be deployed in the future.

The "policies" field will be filled in three ways:
1.	If the "policies" field is empty, it means that the xApp does not need to receive any A1 policies. (See Appendix C for an example of filling in.)
2.	If the "policies" field is not empty, and the "PolicyTypeId" (Conditional) and "SupportXapp" (Optional) fields are filled in, it means that the xApp must receive the A1 policy corresponding to that Policy type. (See Appendix A and Appendix B for examples of filling in.)
3.	If the "policies" field is not empty, and the "PolicyTypeId" (Conditional) field is filled in according to the defined content, it means that the xApp must receive the A1 policy corresponding to that Policy type. (See Appendix D for an example of filling in.)

***    

###### 2.4.1.6.1	PolicyTypeId (Conditional)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : The name of the category of A1 policy.</br> If the xApp needs to receive A1 policies, the content defined in this field must be filled in, and then the xApp Manager will update the data in the xApp Manager Table in influxDB. When filling in this content, you must select a PolicyTypeId from Table 2.4-1 to fill in the content. Conversely, if the xApp does not need to receive A1 policies, the content defined in this field does not need to be filled in.

     <p align="center"> Table 2.4-1 Secondary field correspondence table under the main field </p>

    |Policy type objectives|Description|PolicyTypeId|
    |---|---|---|
    |QoS Target|QoS metric objectives|ORAN_QoSTarget_2.0.0|
    |QoE Target|QoE metric objectives|ORAN_QoETarget_2.0.0|  
    |Traffic Steering Preferences|Balance cell load|ORAN_TrafficSteeringPreference_2.0.0|
    |UE Level Target|UE performance objectives|ORAN_UELevelTarget_1.0.0|  
    |QoS optimization with resource directive|QoS service metric objectives and balancing cell load|ORAN_QoSandTSP_2.0.0|
    |QoE optimization with resource directive|QoE service metric objectives and balancing cell load|ORAN_QoEandTSP_2.0.0|  
    |Load Balancing|Balance load between congested and non-congested candidate cells|ORAN_LoadBalancing_1.0.0|

*	Example : Figure 2.4-9

 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.4-9.png"></image></p>
 <p align="center"> Figure 2.4-9 messaging.ports.policies.PolicyTypeId - example </p> 
 
***

###### 2.4.1.6.2	SupportXapp (Optional)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : List of xApps that support executing A1 policies.</br> If an xApp needs to receive A1 policies, it can choose to fill in this field. The xApp Manager will then update this information in the xApp Manager Table in influxDB. Conversely, if the xApp does not need to receive A1 policies, this field does not need to be filled in. 
 When filling in the content, the xApp designer should enter the xapp_name (Mandatory) field according to the future deployment of xApps that support executing A1 policies in the xApp Descriptor.
 *	Example : Figure 2.4-10
 
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.4-10.png"></image></p>
 <p align="center"> Figure 2.4-10 messaging.ports.policies.SupportXapp - example </p> 
 
***

###### 2.4.1.7	description (Optional)
 *	Writer : xApp designer
 *	Data type : string
 *	Description : The description of the port number's usage should be filled in detail based on the purpose of the port number. When filling in the content, only "space", uppercase or lowercase English letters, and numbers can be used, and the total length is limited to less than or equal to 30 characters.</br>
 *	Example : Figure 2.4-11 
 
 <p align="center"><img src = "https://raw.githubusercontent.com/llab305/O-RAN/master/xApp%20Descriptor/Figure/Figure%202.4-11.png"></image></p>
 <p align="center"> Figure 2.4-11 messaging.ports.description - example </p>
     
***   
